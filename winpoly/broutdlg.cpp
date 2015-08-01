//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright © 1998-1999. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         broutdlg.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TBRouteDlg (TDialog).
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <stdlib.h>

#include "broutdlg.h"
#include "brdata.h"
#include "tfield.h"

// коэффициент увеличения картинки при копировании ее в буфер обмена
#ifndef CLIPBOARD_PICTURE_STRECH_FACTOR
   #define CLIPBOARD_PICTURE_STRECH_FACTOR 1
#endif

//
// Параметр SAMEVXY управляет построением траектории движения.
// Если он нулевой, то на траектории поводыря управление противника
// равно управлению противника основной траектории на предыдущем шаге.
// Т.е. на вымышленной траектории поводыря мы заставляем противника
// действовать так же, как и реальный противник, НО с отставанием на шаг
// Этот вариант предложен Вахрушевым.
//
// В случае положительного значения параметра SAMEVXY (мой экспериментальный
// вариант) управление противника на основной траектории и на траектории
// поводыря всегда синхронно. Это как будто бы противоречит принципу
// позиционности управления, т.к. выходит, что мы выбираем свое управление
// на очередном шаге, уже зная какое управление выберет на этом шаге противник.
//
#ifndef SAMEVXY
   #define SAMEVXY 0
#endif

// текущее рисуемое сечение для drawSection
static TField *plain = NULL;

//
// Build a response table for all messages/commands handled by the application.
//
DEFINE_RESPONSE_TABLE1(TBRouteDlg, TDialog)
//{{TBRouteDlgRSP_TBL_BEGIN}}
  EV_BN_CLICKED(IDC_B_CLOSE, BClose),
  EV_BN_CLICKED(IDC_PICTURE, BNPicture),
  EV_BN_CLICKED(IDC_CB_COLOR, EvCBColor),
  EV_BN_CLICKED(IDC_CB_GRID, EvCBGrid),
  EV_BN_CLICKED(IDC_B_TRADD, BPAdd),
  EV_BN_CLICKED(IDC_B_TRDEL, BPDel),
  EV_LBN_SELCHANGE(IDC_LBTRACE, LBNSelchange),
  EV_EN_CHANGE(IDC_EPADD, EPAddChange),
  EV_BN_CLICKED(IDC_CB_DRAWCOND, EvCBDrawCond),
//{{TBRouteDlgRSP_TBL_END}}
  EV_CHILD_NOTIFY_ALL_CODES(IDC_TB_DENSITY, EvDensityScroll),
END_RESPONSE_TABLE;


//{{TBRouteDlg Implementation}}


static TBRouteDlgXfer TBRouteDlgData;

TBRouteDlg::TBRouteDlg(TWindow* parent, TResId resId, TModule* module) :
   plainsDrawStep(3), plainsDrawn(0), routesDrawn(0), routesBuilt(0),
   crthread( NULL ), TDialog(parent, resId, module) {
//{{TBRouteDlgXFER_USE}}
  LPTrace = new TListBox(this, IDC_LBTRACE);
  CBColor = new TCheckBox(this, IDC_CB_COLOR, 0);
  CBGrid = new TCheckBox(this, IDC_CB_GRID, 0);
  EPAdd = new TEdit(this, IDC_EPADD, 255);

  Picture = new TPicture( this );
  HSDensity = new THSlider(this, IDC_TB_DENSITY, IDB_HSLIDERTHUMB);
  BDel = new TButton(this, IDC_B_TRDEL);
  BAdd = new TButton(this, IDC_B_TRADD);
  CBDrawCond = new TCheckBox(this, IDC_CB_DRAWCOND, 0);

  SetTransferBuffer(&TBRouteDlgData);
//{{TBRouteDlgXFER_USE_END}}

}

TBRouteDlg::~TBRouteDlg() {
  Destroy(IDCANCEL);
  if( crthread ) {
      if( crthread->GetStatus() == TThread::Running )
         crthread->TerminateAndWait();
      delete crthread;
  }
}

void TBRouteDlg::drawRoute() {  // нарисовать следующую траекторию
   if( routesDrawn >= routesBuilt )
      return;
   TDC *DC = Picture->GetDC();
   if( !DC ) return;
   TRect rect = Picture->GetRect();
   TWPS *wps = TBrData::wps;

   // заводим ручку для рисования траектории
   TPen rpen( (CBColor->GetCheck() == BF_CHECKED) ?
      TColor(0,0,255) : TColor().Black,
      2,PS_SOLID);
   // а также ручку для рисования траектории поводыря
   TPen rcpen( (CBColor->GetCheck() == BF_CHECKED) ?
      TColor(0,100,100) : TColor().Black,
      2,PS_SOLID);

   // определяем всяческие границы и размеры (см. tdraw.cpp)
   // высота и ширина
   long double h = wps->maxy() - wps->miny(),
      w = wps->maxx() - wps->minx();
      h*=1.1; w*=1.1; // добавляем поля
   // точка отсчета
   long double x0 = wps->minx() - 0.05*w,
      y0 = wps->miny() - 0.05*h;

   // если рисуем пропорционально.....
   double wh = (double)rect.Width() / rect.Height();
   if( h < w / wh ) {
      h = w / wh;
      y0 -= (h/1.1-( wps->maxy() - wps->miny() ))/2;
   } else {
      w = h * wh;
      x0 -= (w/1.1-( wps->maxx() - wps->minx() ))/2;
   }
   // конец если (рисуем пропорционально)

   // макросы перевода из реальных координат в экранные
   #define x2c(X) (((X)-x0)*rect.Width()/w)
   #define y2c(Y) (rect.Bottom()-(((Y)-y0)*rect.Height()/h))

   DC->SelectObject(rpen);
   CPoint p = route[routesDrawn][0].xrt;
   DC->MoveTo(x2c(p.x),y2c(p.y));
   for( unsigned i = 1; i < route[routesDrawn].length(); i++ ) {
      CPoint p = route[routesDrawn][i].xrt;
      DC->LineTo(x2c(p.x),y2c(p.y));
   }

   if( CBDrawCond->GetCheck() == BF_CHECKED ) {
      // рисуем траекторию поводыря
      DC->SelectObject(rcpen);
      CPoint p = route[routesDrawn][0].yrt;
      DC->MoveTo(x2c(p.x),y2c(p.y));
      for( unsigned i = 1; i < route[routesDrawn].length(); i++ ) {
         CPoint p = route[routesDrawn][i].yrt;
         DC->LineTo(x2c(p.x),y2c(p.y));
      }
   }

   #undef x2c(X)
   #undef y2c(X)

   // увеличиваем routesDrawn
   routesDrawn++;

   // требуем обновить картинку на экране
   Picture->Invalidate();

}

void TBRouteDlg::drawSection() { // нарисовать следующее сечение
   TDC *DC = Picture->GetDC();
   if( !DC ) return;
   TWPS *wps = TBrData::wps;
   if( (long)plainsDrawn >= wps->getSectCount() )
      return; // все уже нарисовано
   if( plain ) {
      if( !plain->PictureReady() )
         return; // еще не дорисовался предыдущий контур
      delete plain;
   }
   plain = wps->getSection( plainsDrawn );
   TRect rect = Picture->GetRect();

   // рисуем контуры *plain на нашем DC (отдельной нитью)
   // + запрашиваем вызов Picture->Invalidate() когда будет готово
   #pragma warn -eas
   TDrawThread::TDrawType drt = TDrawThread::traceField;
   if( CBColor->GetCheck() == BF_CHECKED )
      drt |= TDrawThread::useColors;
   if( plainsDrawn == 0 ) {
      drt |= TDrawThread::cleanField;
      if( CBGrid->GetCheck() == BF_CHECKED )
         drt |= TDrawThread::drawGrid;
   }
   #pragma warn .eas
   plain->preparePictureOnDC(*DC,rect,/*NULL */ Picture, drt,
      wps->minx(), wps->miny(), wps->maxx(), wps->maxy() );

   // увеличиваем plainsDrawn
   plainsDrawn += plainsDrawStep;
   if( (long)plainsDrawn >= wps->getSectCount() )
      Picture->Invalidate(); // перерисуем картинку
}

// построение одного отрезка траетории - следующего по счету
void TBRouteDlg::buildRouteStep() {
   if( routesBuilt > route.last() )
      return; // уже все построили

   // каждая траетория содержит столько же вершин, сколько сечений
   // в насчитанно мосте. Только нумерация у точек траектории прямая,
   // а у сечений моста - обратная.  Нулевая точка траетории соответсвует
   // последнему по построению (и номеру), т.е. первому по времени сечению
   // Т.о. i-тая точка траетории лежит на (getSectCount()-i)-том сечении

   TBrData brd;

   // номер текущей точки траектории
   unsigned rno = route[routesBuilt].last();

   // номер текущей траектории = routesBuilt
   if( (long)route[routesBuilt].length() >= brd.wps->getSectCount() ||
      route[routesBuilt][rno].done ) {
      // все точки этой траетории уже построены или цель уже достигнута
      // значит, здесь больше делать нечего. Переходим к следующей траектории.
      routesBuilt++;
      return;
   }

   // Номер текущего сечения в масиве сечений
   unsigned sectno = brd.wps->getSectCount()-route[routesBuilt].length();
   // время текущего сечения
   long double t = brd.wps->getSectionTime(sectno);
   // шаг от текущего сечения до следующего по времени
   long double delta = brd.wps->getSectionTime(sectno-1)-t;

   // данные следующей точки траектории
   CRPoint nrp = route[routesBuilt][rno+1];
   // данные текущей точки траетории
   CRPoint crp = route[routesBuilt][rno];
   // ВНИМАНИЕ! Вообще-то ссылки на элемент массива TArray - вещь чрезвычайно
   // опасная, т.к массив при автоматическом расширении/сужении может перенести
   // свои элементы в другое место в памяти.  Необходимо либо работать с
   // элементами массива непосредственно, либо гарантировать, что на время
   // жизни ссылки никакого изменения длиных массива не произойдет!

   int rnd; // случайное число...

   if( rno == 0 ) {
      // мы находимся в начальной точке траектории
      // сначала выберем поводыря - ближайшую к нам точку моста
      TField *scur = brd.wps->getSection(sectno);
      crp.yrt = scur->findClosest( crp.xrt );
      delete scur;
      randomize();
#if !SAMEVXY
      // на первом шаге выбираем случайное управление противника для поводыря
      rnd = random( brd.Q.length() );
      crp.yv = brd.Q[ rnd ];
   } else {
      // очередной шаг траектории
      // берем управление противника для поводыря равным управлению противника
      // на предыдущем шаге основной траектории
      crp.yv = route[routesBuilt][rno-1].xv;
#endif
   }

#if SAMEVXY
   // выбираем случайное управление противника (одно и то же для основной
   // траектории и траектории поводыря
   rnd = random( brd.Q.length() );
   crp.yv = crp.xv = brd.Q[ rnd ];
#endif

   // если шаг равен нулю, значит по ошибке в мост затесалось
   // два одинаковых сечения подряд.  В этом случае просто копируем
   // все параметры текущей точки траектории на следующую
   if( EQU( delta, 0 ) ) {
      route[routesBuilt][rno] = route[routesBuilt][rno+1] = crp;
      return;
   }

   // построим набор вершин множества достижимости поводыря
   TArray<CPoint> ypt;
   for( unsigned p=0; p<brd.P.length(); p++ )
      ypt.push( brd.next( crp.yrt, brd.P[p], crp.yv, t, delta ) );

   // набор найденных точек
   TArray<CPoint> yrtp;

   // ищем любую точку из пересечения множества достижимости поводыря
   // и следующего (по времени) сечения
   TField *snext = brd.wps->getSection(sectno-1); // следующее сечение
   for( unsigned p=0; p<ypt.length(); p++ ) {
      // цикл по всем многоугольникам следующего сечения
      for( unsigned k=0; k<snext->nPoly(); k++ ) {
         TWinPoly *wp = snext->getPoly(k);
         EPlacement pl = wp->placement(ypt[p]);
         if( pl == PL_INSIDE || pl == PL_BORDER ) {
            // точка уже внутри. Берем ее в качестве целевой
            yrtp.push( ypt[p] );
         }
         // пробуем искать пересечение отрезка из двух соседних точек
         // целевого множества с каждой из граней текущего многоугольника
         CEdge e( p?ypt[p-1]:ypt[ypt.last()] , ypt[p] );

         // цикл по всем внешим ребрам многоугольника
         for( unsigned i=0; i<wp->nVertex(); i++ ) {
            CEdge we( i?*(wp->getVertex(i-1)):*(wp->getVertex(wp->nVertex()-1)),
               *(wp->getVertex(i)) );
            CrossPoint cp = we.cross(e);
            if( cp.type() == CT_CROSS ) {
               // нашли точку пересечения
               yrtp.push( *((CPoint*)cp) );
            }
         }

         // цикл по всем дыркам многоугольника
         for( unsigned j=0; j<wp->nHoles(); j++ ) {
            TPolygon *hole = wp->getHole(j);
            // цикл по всем ребрам дырки
            for( unsigned i=0; i<hole->nVertex(); i++ ) {
               CEdge we( *(i?hole->getVertex(i-1):
                  hole->getVertex(hole->nVertex()-1)),
                  *(hole->getVertex(i)) );
               CrossPoint cp = we.cross(e);
               if( cp.type() == CT_CROSS ) {
                  // нашли точку пересечения
                  yrtp.push( *((CPoint*)cp) );
               }
            }
         }

      }
   }

   if( yrtp.length() > 0 ) {
      // выбираем случайным образом одну из найденных точек
      nrp.yrt = yrtp[ random( yrtp.length() ) ];
   } else {
      // а здесь у нас мы не нашли пересечения!  Нужно взять за следующую точку
      // траектории поводыря ближайшую к множества достижимости поводыря точку
      // из snext

      // возможно, это не самый точный способ, но пока что просто найдем ближайшие
      // точки из snext к каждой из вершин множества достижимости и возьмем
      // ближайшую из них
      nrp.yrt = snext->findClosest(ypt[0]);
      long double mdist = ( nrp.yrt - ypt[0] ).length();
      for( unsigned p=1; p<ypt.length(); p++ ) {
         CPoint pp = snext->findClosest(ypt[p]);
         if( ( pp - ypt[p] ).length() < mdist ) {
            nrp.yrt = pp;
            mdist = ( nrp.yrt - ypt[p] ).length();
         }
      }
   }

   // следующее сечение нам больше не нужно
   delete snext;

   // теперь вычисляем управление первого игрока для поводыря, путем решения
   // уравнения движения относительно U
   crp.yu   = brd.findPvalue( crp.yrt, nrp.yrt, crp.yv, t, delta );

   // на основной траектории управление первого игрока берем то же,
   crp.xu = crp.yu;

#if !SAMEVXY
   // а управление противника может быть любым
   rnd = random( brd.Q.length() );
   crp.xv = brd.Q[rnd];
#endif

   // строим движение по основной траектории при заданных управлениях
   nrp.xrt = brd.next( crp.xrt, crp.xu, crp.xv, t, delta );

   // если наша задача - к моменту времени, то нужно проверить,
   // не достигли ли мы уже цели
   if( !brd.wps->getHeader().fixedT ) {
      TField *M = brd.wps->getSection(0); // целевое множество
      for( unsigned i=0; i<M->nPoly(); i++ ) {
         TWinPoly *wp = M->getPoly(i);
         EPlacement pl = wp->placement( nrp.xrt );
         if( pl == PL_INSIDE || pl == PL_BORDER ) {
            nrp.done = true;
            break;
         }
      }
   }

   // вносим изменения в основной массив
   route[routesBuilt][rno+1] = nrp; // следующая точка траектории
   route[routesBuilt][rno] = crp;   // текущая точка траектории

   // вот и все. Очередная точка траектории построена.
}

bool TBRouteDlg::IdleAction(long idleCount) {
   bool result;
   result = TDialog::IdleAction(idleCount);

   try {
      // как только готово очередное сечение, начинаем рисовать следующее
      if( (long)plainsDrawn < TBrData::wps->getSectCount() &&
         ( !plain || plain->PictureReady() ) )
         drawSection();
      else if( routesDrawn < routesBuilt )
         drawRoute();

      // вычисление траекторий
      if( routesBuilt <= route.last() &&
         ( !crthread || crthread->GetStatus() != TThread::Running ) ) {
         // запускаем crthread для вычисления траекторий
         if( crthread )
            delete crthread;
         crthread = new TCalcRouteThread(this);
         crthread->Start();
      }

   } catch( Exception &e ) {
      char buf[100];
      sprintf(buf,"Программное исключение (%s)",e.where());
      MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      CmCancel();
   }

   return result;
}

// функция вычисления траекторий
int TCalcRouteThread::Run() {
   try {
      while( parent->routesBuilt <= parent->route.last() ) {
         parent->buildRouteStep();
         if( ShouldTerminate() )
            return 1;
      }
   } catch( Exception &e ) {
      char buf[100];
      sprintf(buf,"Программное исключение (%s)",e.where());
      parent->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      return 1;
   }

   return 0;
}

void TBRouteDlg::SetupWindow() {
   TDialog::SetupWindow();
   routesDrawn = plainsDrawn = 0;
   HSDensity->SetRuler(1,true);
   HSDensity->SetRange(1,
      TBrData::wps->getSectCount()<90?
      int(TBrData::wps->getSectCount()/3):30);
   HSDensity->SetPosition(3);
   plainsDrawStep = HSDensity->GetPosition();
   CBGrid->SetCheck(true);
   CBColor->SetCheck(true);
   CBDrawCond->SetCheck(false);
   BDel->EnableWindow(false);
   BAdd->EnableWindow(false);
   LPTrace->ClearList();
}

// изменили плотность
void TBRouteDlg::EvDensityScroll(uint /*code*/) {
   if( HSDensity->GetPosition() != plainsDrawStep ) {
      plainsDrawStep = HSDensity->GetPosition();
      routesDrawn = plainsDrawn = 0;
   }
}

// щелчок по кнопке закрытия
void TBRouteDlg::BClose() {
   CmOk();
}

// щелчок по картинке - занесем ее в буфер обмена
void TBRouteDlg::BNPicture() {
   TClipboard clb = OpenClipboard();
   clb.EmptyClipboard();
   TBitmap *fbm = Picture->GetBitmap();
   if( fbm )
      fbm->ToClipboard( clb );
   clb.CloseClipboard();
   EPAdd->SetFocus();
}

// сменили цветное изображение на черно-белое или наоборот.
// придется все перерисовать
void TBRouteDlg::EvCBColor() {
   routesDrawn = plainsDrawn = 0;
}

// включили/отменили вывод сетки
// придется все перерисовать
void TBRouteDlg::EvCBGrid() {
   routesDrawn = plainsDrawn = 0;
}

// включили/отменили вывод траектории поводыря
// придется все перерисовать
void TBRouteDlg::EvCBDrawCond() {
   routesDrawn = plainsDrawn = 0;
}

// нажали кнопку "добавить начальную точку"
// будем добавлять
void TBRouteDlg::BPAdd() {
   char buf[100];
   EPAdd->GetText(buf,100);
   EPAdd->SetFocus();

   // проверяем корректность данных в окне ввода
   char *ptr;
   long double x,y;
   x = _strtold(buf,&ptr);
   while( *ptr == ' ' ) ptr++;
   if( *ptr != '\0' ) {
      y = _strtold(ptr,&ptr);
      while( *ptr == ' ' ) ptr++;
      if( *ptr == '\0' ) {
         // данные корректны. Добавляем их в список
         sprintf(buf,"%Lg\t%Lg",x,y);
         EPAdd->Clear();   // очищаем окошко
         TPArray spt;
         spt[0].xrt = CPoint(x,y);
         int idx = LPTrace->AddString(buf);
         route.insert(spt,idx);
         if( idx < routesBuilt )
            routesBuilt = idx;
         return;
      }
   }

   // ошибка ввода
   MessageBox("Необходимо ввести координаты точки - два числа через пробел",
      "Ошибка ввода координат", MB_ICONSTOP | MB_OK);
   EPAdd->SetSelection(0,strlen(buf));
   return;

}

// пользователь желает удалить одну из начальных точек
void TBRouteDlg::BPDel() {
   int idx = LPTrace->GetSelIndex();
   if( idx >= 0 ) {
      // необходимо пока прекратить вычисления...
      if( crthread && crthread->GetStatus() == TThread::Running ) {
         crthread->TerminateAndWait();
         delete crthread;
         crthread = NULL;
      }
      // удаляем выбранную траекторию
      LPTrace->DeleteString(idx);
      route.remove(idx);
      if( idx < routesBuilt )
         routesBuilt--;
      LBNSelchange();
      // надо все перерисовать
      routesDrawn = plainsDrawn = 0;
   }
   EPAdd->SetFocus();
}

// Изменение выбранного элемента списка
void TBRouteDlg::LBNSelchange() {
   BDel->EnableWindow( LPTrace->GetCount() > 0 && LPTrace->GetSelIndex() >=0 );
}

// Изменение текста в окне ввода
void TBRouteDlg::EPAddChange() {
   char buf[3];
   EPAdd->GetText(buf,3);
   BAdd->EnableWindow( buf[0]!=0 );
}

void TPicture::ODADrawEntire(DRAWITEMSTRUCT far& drawInfo) {
   TDC drawDC(drawInfo.hDC);
   TRect region(drawInfo.rcItem.left, drawInfo.rcItem.top,
                 drawInfo.rcItem.right, drawInfo.rcItem.bottom);
   TRect mregion( drawInfo.rcItem.left, drawInfo.rcItem.top,
      drawInfo.rcItem.left +
         (drawInfo.rcItem.right-drawInfo.rcItem.left)*
         CLIPBOARD_PICTURE_STRECH_FACTOR,
      drawInfo.rcItem.top +
         (drawInfo.rcItem.bottom-drawInfo.rcItem.top)*
         CLIPBOARD_PICTURE_STRECH_FACTOR );
   if( !mBitmap ) {
      // инициализируем битмап, если еще не сделали этого
      mDC = new TMemoryDC(drawDC);
      mBitmap = new TBitmap( drawDC, mregion.Width(), mregion.Height() );
      mDC->SelectObject(*mBitmap);
      mDC->FillRect(mregion,TBrush(TColor().SysAppWorkspace));
      rect = mregion;
   }
   // копируем битмап на экран (возможно, растягивая или сжимая)
#if CLIPBOARD_PICTURE_STRECH_FACTOR == 1
   drawDC.BitBlt( region, *mDC, TPoint(0,0) );
#else
   drawDC.StretchBlt( region, *mDC, mregion );
#endif
}


