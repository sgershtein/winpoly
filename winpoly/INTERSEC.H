//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         intersec.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Implementation of intersection and union of two polygons (Class TWinPoly)
//
//----------------------------------------------------------------------------

// Проверять на наличие возможных самопересечений контуров при
// операциях объединения и пересечения многоугольников.
// Если заранее известно, что таковых нет, то это не нужно.
// Но если самопересечения возможны (как, например,
// при построении предыдущего сечения по заданному в некоторой динамике
// при построении моста), то такая проверка необходима.
#ifndef CHECK_SELFCROSS
   #define CHECK_SELFCROSS 0
#endif

#if IU=='I'

# ifndef INTERSEC_H_INTERSECTION
#  define INTERSEC_H_INTERSECTION
#  define INTERSECT
# else
#  error "This file has already been included for INTERSECTION"
# endif

#elif IU=='U'

# ifndef INTERSEC_H_UNION
#  define INTERSEC_H_UNION
#  define UNITE
# else
#  error "This file has already been included for UNION"

# endif

#else
# error "IU must be defined as either 'I' or 'U'"
#endif

#if !defined(INTERSEC_H_PASS_1) && !defined(INTERSEC_H_PASS_2)
# define INTERSEC_H_PASS_1
#elif !defined(INTERSEC_H_PASS_2)
# define INTERSEC_H_PASS_2
# undef INTERSEC_H_PASS_1
#else
# pragma message pass 3 - this is strange!
#endif

// определяем классы только на первом проходе
#ifdef INTERSEC_H_PASS_1

class SCross;

// определяем структуру элемента списка вершин
class SVertex {
   public:
      CPoint v;         // собственно точка - вершина многоугольника
      long cross;       // номер множества вершин, совпадающих с данной
                        // (индекс в массиве scross) или -1 если такового нет
      long processed;   // номер контура, в котором лежит эта вершина или 0,
                        // если она еще не обработана
      long double angle;   // угол вектора из текущей вершины в
                           // предыдущую при движении по контуру.
                           // установлен только в точках пересечения контуров!

      // конструктор, просто присваивающий значения элементам класса
      SVertex(CPoint V, long double ang = 10)
         : v(V), processed(0), angle(ang), cross(-1) {};
      // default constructor
      SVertex() {};
      operator CPoint() const { return v; };

};

// структура, определяющая набор совпадающих вершин
class SCross {
   public:
      TArray< TListPtr<SVertex> > sv;   // совпадающие вершины
      TListPtr<SVertex> operator[](unsigned i) {
         return sv[i];
      }
      long last() {
         return sv.last();
      }
      long length() {
         return sv.length();
      }
      void push( TListPtr<SVertex> V ) {
         sv.push(V);
      }
      TListPtr<SVertex> pop() {
         return sv.pop();
      }
};

// проверить на пересечение и добавить его в список при необходимости
void cpCross( TArray<SCross> &scross,
   TListPtr<SVertex> &pv0, TListPtr<SVertex> &pv1 ) {

   // определяем следующую вершину по контуру 0
   TListPtr<SVertex> pv0n = pv0.next();
   // определяем следующую вершину по контуру 1
   TListPtr<SVertex> pv1n = pv1.next();

   // находим точку пересечения отрезков
   // (*pv0,*pv0n) и (*pv1,*pv1n), если таковая существует
   CrossPoint cxp = CEdge(*pv0,*pv0n).cross(CEdge(*pv1,*pv1n));

   // Точка пересечения рёбер
   CPoint *xp;

   if( cxp.type() & CT_CROSS ) { // явно пересекаются
      xp = cxp;
      // если пересечение совпадает с точкой *pv0 или *pv1, то
      // мы его уже обработали (или сделаем это в конце).
      // Т.е. пересечения с начальными точками граней игнорируем
      if( *xp == *pv0 || *xp == *pv1 )
         return;
   } else if( cxp.type() & CT_COLLINEAR ) // лежат на одной прямой
      // в этом случае точкой пересечения будем считать только
      // ту из точек pv1n, pv0n, которая лежит на грани
      // (собственно на отрезке, не снаружи!) противоположного
      // контура.
      // Точки pv0 и pv1 нас не интересуют, по той же причине,
      // что и в случае нормального пересечения.
      if( CEdge(*pv0,*pv0n).on(*pv1n) &&
         (*pv1n).v != (*pv0).v )
         // *pv1n лежит на грани (*pv0,*pv0n) но не совпадает с *pv0
         xp = &(*pv1n).v;
      else if( CEdge(*pv1,*pv1n).on(*pv0n) &&
         (*pv0n).v != (*pv1).v )
         // *pv0n лежит на грани (*pv1,*pv1n) но не совпадает с *pv1
         xp = &(*pv0n).v;
      else
         return;
   else
      return;   // рёбра не пересекаются

   // если точка пересечения лежит на середине грани, то
   // добавляем её на грань в качестве новой вершины
   if( *xp != *pv0n ) {
      pv0.insertnext(SVertex(*xp));
      pv0n = pv0.next();
   }
   if( *xp != *pv1n ) {
      pv1.insertnext(SVertex(*xp));
      pv1n = pv1.next();
   }

   // устанавливаем значения углов подхода к точкам пересечения
   // на самом деле это углы от точки пересечения в сторону
   // предыдущих вдоль движения по контуру точек
   (*pv0n).angle = TVector(*pv0n,*pv0).angle();
   (*pv1n).angle = TVector(*pv1n,*pv1).angle();

   // Итак, вершины *pv0n и *pv1n у нас совпадают.
   // Занесём данные о них в набор совпадающих вершин
   if( (*pv0n).cross < 0 && (*pv1n).cross < 0 ) {
      long cno = scross.last()+1;
      scross[cno].push(pv0n);
      scross[cno].push(pv1n);
      (*pv0n).cross = cno;
      (*pv1n).cross = cno;
   } else if( (*pv0n).cross >= 0 && (*pv1n).cross >= 0 ) {
      if( (*pv0n).cross != (*pv1n).cross ) {
         // необходимо объединить два набора
         long pv1no = (*pv1n).cross;
         while( scross[pv1no].length() > 0 ) {
            TListPtr<SVertex> sp = scross[pv1no].pop();
            (*sp).cross = (*pv0n).cross;
            scross[(*pv0n).cross].push(sp);
         }
      }
   } else if( (*pv0n).cross >= 0 ) {
      scross[(*pv0n).cross].push(pv1n);
      (*pv1n).cross = (*pv0n).cross;
   } else if( (*pv1n).cross >= 0 ) {
      scross[(*pv1n).cross].push(pv0n);
      (*pv0n).cross = (*pv1n).cross;
   } else
      Throw("Impossible situation!");

} // конец функции cpCross

#endif   // INTERSEC_H_PASS_1

#ifdef INTERSECT

// пересечение двух многоугольников - возвращает результирующий
// список многоугольников.  Текущий многоугольник не меняется
TList<TWinPoly*> TWinPoly::intersect(TWinPoly *P,
   bool volatile *ShouldTerminate) {

#elif defined(UNITE)

// объединение двух многоугольников - возвращает результирующий
// список многоугольников.  Текущий многоугольник не меняется
TList<TWinPoly*> TWinPoly::unite(TWinPoly *P,
   bool volatile *ShouldTerminate) {

#endif

   // определяем основные массивы списков вершин двух многоугольников и
   // дырок в них.  Нулевые элементы масивов - списки вершин внешних
   // контуров многоугольников, остальные элементы - списки вершин контуров
   // дырок.
   // lst[0] - список для многоугольника *this
   // lst[1] - список для многоугольника *P

   TArray< TListPtr< SVertex > > lst[2];

   // Мы не будем специально различать *this и *P, а заведем просто
   // двухэлементный массив со ссылками на эти два многоугольника.

   TWinPoly* pp[2] = { this, P };

   // Массив множеств совпадающих вершин

   TArray<SCross> scross;

   // просто пустой список.  Возвращаем его в случае чего

   TList<TWinPoly*> EmptyList;

   // заносим в список реальные вершины многоугольников
   // Для этого поочередно добавляем в голову списка вершины контура,
   // в обратном порядке.  Т.о. в списке они окажутся в правильном
   // порядке

   for( int n = 0; n < 2; n++ ) { // цикл по номеру многоугольника

      // сначала - внешний контур
      for( long i = pp[n]->vertex.last(); i >= 0; i-- )
         lst[n][0].insertfirst(SVertex(pp[n]->vertex[i]));

      // теперь контуры дырок
      for( unsigned j = 0; j < pp[n]->hole.length(); j++ ) {
         TPolygon *hole = pp[n]->hole[j];
         for( long i = hole->nVertex()-1; i >= 0; i-- ) {
            lst[n][j+1].insertfirst(SVertex(*(hole->getVertex(i))));
            if( ShouldTerminate && *ShouldTerminate )
               break;
         }
         if( ShouldTerminate && *ShouldTerminate )
            break;
      }
      if( ShouldTerminate && *ShouldTerminate )
         break;
   }

   if( ShouldTerminate && *ShouldTerminate ) { // преждевременное завершение
      // освобождаем память из-под lst
      for( int n=0; n<2; n++ )
         while( lst[n].length() > 0 )
            lst[n].pop().deletelist();
      return EmptyList;
   }

   // теперь ищем все точки пересечения между контурами _разных_
   // многоугольников и заносим их в виде дополнительных вершин
   // в оба соответсвующих контура.
   // Отдельно ведём список множеств совпадающих вершин.

   // цикл по всем контурам мн-ка 0
   for( unsigned k0 = 0; k0 < lst[0].length(); k0++ ) {
      // цикл по всем вершинам контура мн-ка 0
      bool dopv0 = true; // флаг продолжения цикла
      for( TListPtr<SVertex> pv0 = lst[0][k0]; dopv0;
         dopv0 = ( ++pv0 != lst[0][k0] ) )
         // цикл по всем контурам мн-ка 1
         for( unsigned k1 = 0; k1 < lst[1].length(); k1++ ) {
            // цикл по всем вершинам контура мн-ка 1
            bool dopv1 = true; // флаг продолжения цикла
            for( TListPtr<SVertex> pv1 = lst[1][k1]; dopv1;
               dopv1 = ( ++pv1 != lst[1][k1] ) ) {
               // зацикливаем списки
               if( pv0.last() )
                  pv0.setnext(lst[0][k0]);
               if( pv1.last() )
                  pv1.setnext(lst[1][k1]);

               // проверим, не нужно ли преждевременно завершиться
               if( ShouldTerminate && *ShouldTerminate ) {
                  // освобождаем память из-под lst
                  for( int n=0; n<2; n++ )
                     while( lst[n].length() > 0 )
                        lst[n].pop().deletelist();
                  return EmptyList;
               }

               // делаем все необходимое:
               // находим точку пересечения ребер, и если таковая есть,
               // то заносим ее на каждое ребро, а также в массив точек
               // пересечения
               cpCross( scross, pv0, pv1 );

            }
         }
   } // система четырех вложенных циклов по всем парам вершин

   // теперь идет кусок проверки каждого из двух многоугольников на наличие
   // самопересечений. Если заранее известно, что таковых нет, то этот
   // кусок кода не нужен.  Но если самопересечения возможны (как, например,
   // при построении предыдущего сечения по заданному в некоторой динамике
   // при построении моста), то этот кусок необходим.
   #if CHECK_SELFCROSS == 1

   for( int n=0; n<2; n++ ) // цикл по обоим многоугольникам
      // цикл по всем контурам мн-ка n
      for( unsigned k0 = 0; k0 < lst[n].length(); k0++ ) {
         // цикл по всем вершинам контура мн-ка n
         bool dopv0 = true; // флаг продолжения цикла
         for( TListPtr<SVertex> pv0 = lst[n][k0]; dopv0;
            dopv0 = ( ++pv0 != lst[n][k0] ) )
            // цикл по всем контурам того же мн-ка
            for( unsigned k1 = k0; k1 < lst[n].length(); k1++ ) {
               // цикл по всем вершинам контура того же мн-ка
               bool dopv1 = true; // флаг продолжения цикла
               for( TListPtr<SVertex> pv1 = lst[n][k1]; dopv1;
                  dopv1 = ( ++pv1 != lst[n][k1] ) ) {

                  if( pv1 == pv0 ||
                     pv1.next() == pv0 )
                     break;

                  // проверим, не нужно ли преждевременно завершиться
                  if( ShouldTerminate && *ShouldTerminate ) {
                     // освобождаем память из-под lst
                     for( int n=0; n<2; n++ )
                        while( lst[n].length() > 0 )
                           lst[n].pop().deletelist();
                     return EmptyList;
                  }

                  // делаем все необходимое:
                  // находим точку пересечения ребер, и если таковая есть,
                  // то заносим ее на каждое ребро, а также в массив точек
                  // пересечения
                  cpCross( scross, pv0, pv1 );

               }
            }
      }

   #endif   // проверка самопересчений контуров

   // В этом месте мы имеем все точки пересечения.  Можно начинать строить
   // результирующие контуры.

   // Списки получающихся положительных и отрицательных контуров
   TList<TWinPoly*> rplus, rminus;

   // Номер текущего контура
   long bno = 0;

   // Цикл по всем вершинам. Как только находим ещё необработанную,
   // начинаем вести от неё линюю контура...
   for( int n=0; n<2; n++ )
      for( unsigned j = 0; j < lst[n].length(); j++ ) {
         bool dopv = true; // флаг продолжения цикла. Этот флаг сбрасывается
                           // при возврате в исходную вершину
         for( TListPtr<SVertex> pv = lst[n][j]; dopv;
            dopv = ( ++pv != lst[n][j] ) )
            if( (*pv).processed == 0 ) {
               // Нашли нетронутую вершину. Начинаем линию контура
               bno++;
               TWinPoly *wp = new TWinPoly;  // текущий контур
               TListPtr<SVertex> cv = pv;    // текущая вершина

               // ходим по контуру, пока не попадём в точку, в которой уже
               // были
               while( 1 ) {

                  // проверим, не нужно ли преждевременно завершиться
                  if( ShouldTerminate && *ShouldTerminate ) {
                     // освобождаем память из-под lst
                     for( int n=0; n<2; n++ )
                        while( lst[n].length() > 0 )
                           lst[n].pop().deletelist();
                     if( wp ) delete wp;
                     rplus.deletelist();
                     rminus.deletelist();
                     return EmptyList;
                  }

                  // проверим, не попали ли мы в вершину, которая уже
                  // лежит в нашем контуре?  Если да, то следует
                  // выделить цикл в виде отдельного контура.
                  bool loopchk = false;   // нужна детальная проверка?
                  if( (*cv).v == (*pv).v )
                     loopchk = true;   // получили контур
                  else if( (*cv).cross >= 0 )
                     for( long sj=0; sj < scross[(*cv).cross].length(); sj++ )
                        if( (*(scross[(*cv).cross][sj])).processed == bno ) {
                           loopchk = true; // необходимо проверить детально
                           break;
                        }

                  if( loopchk )
                     // проводим детальную проверку, т.е. сравниваем
                     // координаты всех вершин нашего контура с текущей
                     for( unsigned i=0; i<wp->vertex.length(); i++ )
                        if( wp->vertex[i] == (*cv).v ) {
                           // вершины совпали!
                           // копируем замкнутый контур в отдельный мн-ник

                           TWinPoly *wp1 = new TWinPoly;
                           long double angle = 0; // суммарный угол поворота
                           long double aprev =
                              TVector(wp->vertex[wp->vertex.last()],
                              wp->vertex[i]).angle();
                           for( long j=i; j<=wp->vertex.last(); j++ ) {
                              *wp1 += wp->vertex[j];
                              long double a = TVector(wp->vertex[j],
                                 wp->vertex[(j<wp->vertex.last())?j+1:i]).angle();
                              if( fabsl(a-aprev) <= M_PI )
                                 angle += a-aprev;
                              else if( fabsl(a + 2*M_PI - aprev) <= M_PI )
                                 angle += a + 2*M_PI - aprev;
                              else if( fabsl(a - 2*M_PI - aprev) <= M_PI )
                                 angle += a - 2*M_PI - aprev;
                              else
                                 Throw("Ошибка определения ориентации контура"
                                 " - непонятные полярные углы");
                              aprev = a;
                           }

                           if( wp1->nVertex() < 3 ) {
                              delete wp1; // вершины слишком близко -
                                          // ошибки округления
                           } else {
                              // добавляем этот мн-ник к результирующий список
                              // или в список дырок, в зависимости от ориентации
                              if( EQU( angle, 2*M_PI ) ) {
                                 rplus.add(wp1); // положительная ориентация
                              } else if( EQU( angle, -2*M_PI ) ) {
                                 rminus.add(wp1); // отрицательная ориентация
                              } else {
                                 Throw("Ошибка определения ориентации контура:"
                                    " странный угол");
                              }
                           }

                           // выкидываем этот контур из нашего многоугольника
                           // (выкидываем начиная с i-той вершины)
                           wp->vertex.setlength(i);
                           break;
                        }

                  // заканчиваем работу, если эта точка - уже
                  // пройденный этап
                  if( (*cv).processed )
                     break;

                  // добавляем текущую вершину в список вершин нового мн-ка
                  *wp += *cv;
                  // помечаем вершину как обработанную
                  (*cv).processed = bno;

                  // теперь определяем дальнейшее направление движения
                  // из этой вершины.
                  // Если вершина не совпадает с другой, то просто движемся
                  // дальше по контуру
                  if( (*cv).cross >= 0 ) {
                     // совпадает... Придется определять, нужно ли менять
                     // контур
                     unsigned bj = -1;
#ifdef INTERSECT
                     long double ba = -10;
#elif defined( UNITE )
                     long double ba = 10;
#endif
                     for( long sj=0; sj < scross[(*cv).cross].length(); sj++ ) {
                        long double a = TVector(*cv,
                           *(scross[(*cv).cross][sj].next())).angle();
#ifdef INTERSECT
                        // находим максимальный (самый левый) угол
                        if( a < (*cv).angle )
                           a += 2*M_PI;
                        if( a > ba ) {
                           bj = sj;
                           ba = a;
                        }
#elif defined( UNITE )
                        // находим минимальный (самый правый) угол
                        if( a > (*cv).angle )
                           a -= 2*M_PI;
                        if( a < ba ) {
                           bj = sj;
                           ba = a;
                        }
#endif
                     }
                     // итак, лучше всего перейти на совпадающую вершину
                     // с номером bj. Переходим.
                     cv =  scross[(*cv).cross][bj];
                     // вершину, в которую мы прыгнули, не помечаем
                     // как обработанную.  Возможно, мы придём в нее
                     // другим путём и уйдём тоже другим.
                  }

                  // переходим к следующей вершине в списке
                  cv++;
               }

               // мы пришли в точку, уже принадлежащую контуру,
               // построенному ранее. Значит, наш хвост (если он есть)
               // оказался лишним, его следует просто выкинуть
               delete wp;

            } // if( (*pv).processed )
      } // два вложенных цикла for по всем вершинам

   // Наконец мы имеем два списка кунтуров, rplus и rminus, соответственно
   // положительной и отрицательной ориентации.

   // вводим временные списки
   TList<TWinPoly*> rplus1;
   TList<TWinPoly*> rminus1;

   // Как ни странно, при объединении практически вся логика меняется
   // на строгшо противоволожную...

#ifdef INTERSECT

# define RPLUS rplus
# define RPLUS1 rplus1
# define RMINUS rminus
# define RMINUS1 rminus1

#elif defined( UNITE )

# define RPLUS rminus
# define RPLUS1 rminus1
# define RMINUS rplus
# define RMINUS1 rplus1

#endif

   // Просматриваем список положительных (INTERSECT) контуров на предмет
   // их содержания в обоих исходных многоугольниках.
   // [отрицательных (UNION)]

   while( !RPLUS.empty() ) {
      TWinPoly* wp = RPLUS.pop();
      // проверяем, что wp целиком лежит внутри как pp[0], так и pp[1]
      // Для этого проверяем, что СЕРЕДИНЫ всех его граней лежат внутри
      unsigned outcnt = 0;
      for( int n = 0; n < 2; n++ ) {
         for( long i = wp->nVertex()-1; i >= 0; i-- ) {
            // Проверочная сторона
            TVector v = TVector( wp->vertex[i],
               i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
            // проверочная точка
            CPoint cp = wp->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)pp[n])->placement(cp);
            if( pl == PL_OUTSIDE ) {
               outcnt++;
               break;
            } else if( pl == PL_INSIDE ) {
#ifdef UNITE
               // проверяем, не внутри ли какой-нибудь дырки pp[n]
               // мы находимся? Если да, то => outside!
               // т.о. мы сохраним совпадающие (возможно, частично)
               // дырки различных многоугольников

               for( unsigned j = 0; j < pp[n]->hole.length(); j++ ) {
                  TPolygon *hole = pp[n]->hole[j];
                  for( long i = wp->nVertex()-1; i >= 0; i-- ) {
                     // Проверочная сторона
                     TVector v = TVector( wp->vertex[i],
                        i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
                     // проверочная точка
                     CPoint cp = wp->vertex[i] + v*0.5;
                     EPlacement pl = hole->placement(cp);
                     if( pl == PL_OUTSIDE ) {
                        break;
                     } else if( pl == PL_INSIDE ) {
                        outcnt++;
                        break;
                     }
                     if( i <= 0 )
                        outcnt++; // если наша дырка совпадает с
                                  // исходной дыркой hole
                  }
               }
#endif
               break;
            }
         }
      }
#ifdef INTERSECT
      if( outcnt == 0 )
#elif defined( UNITE )
      if( outcnt > 0 )
#endif
         RPLUS1.push(wp);
      else
         delete wp;
   }
   RPLUS = RPLUS1;
   RPLUS1.clear();

   // нужно попарно сравнить положительные контуры и выкинуть те из них,
   // которые содержат внутри другие (ПЕРЕСЕЧЕНИЕ) [которые содержатся
   // внутри других (ОБЪЕДИНЕНИЕ)]
   // *** Новая мысль [21.11.99]: в случае пересечения зачем выкидывать
   // многоугольники, содержащие внутри себя другие? Ведь мы уже проверили,
   // что все они лежат внутри обоих исходных, следовательно все они
   // лежат в пересечении... Ведь внутренний многоугольник возможно
   // просто лежит внутри дырки внешнего - и это вполне корректно!

   // нужно попарно сравнить отрицательные контуры и выкинуть те из них,
   // которые лежат внутри других (intersection)
   // [содержат внутри себя другие (union). Но эта проверка - лишняя, т.к.
   // мы уже выкинули их ранее, как дырки, попадающие во оба исходных
   // внешних контура]
   // Итак, эту проверку проводим ТОЛЬКО для случая INTERSECT

//#ifdef INTERSECT  -- см. новую мысль [21.11.99] выше.
#if 0
   while( !RPLUS.empty() ) {
      TWinPoly* wp = RPLUS.pop();
      bool isinside = false; // есть ли кто-то внутри нас?
      for( TListPtr<TWinPoly*> ppl = RPLUS.empty()?
         RPLUS1.head() : RPLUS.head();
         !ppl.empty() && !isinside;
         ppl++ ) {
         for( long i = (*ppl)->nVertex()-1; i >= 0; i-- ) {
            // Проверочная сторона
            TVector v = TVector( (*ppl)->vertex[i],
               i? (*ppl)->vertex[i-1]: (*ppl)->vertex[(*ppl)->nVertex()-1] );
            // проверочная точка
            CPoint cp = (*ppl)->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)wp)->placement(cp);
            if( pl == PL_OUTSIDE )
               break;
            else if( pl == PL_INSIDE ) {
               isinside = true;
               break;
            }
         }
         // необходимо проверять, не лежит ли внутри текущего кто-то из уже
         // проверенных... Переходим с конца списка rplus на начало списка
         // rplus1
         if( ppl == RPLUS.tail() )
            ppl = RPLUS1.head();
      }
      if( !isinside )
         RPLUS1.push(wp);  // никто в нем не лежит => берём
   }
   RPLUS = RPLUS1;
   RPLUS1.clear();
#endif

   while( !RMINUS.empty() ) {
      TWinPoly* wp = RMINUS.pop();
      bool isinside = false;
      for( TListPtr<TWinPoly*> ppl = RMINUS.empty()?
         RMINUS1.head(): RMINUS.head();
         !ppl.empty() && !isinside;
         ppl++ ) {
         for( long i = wp->nVertex()-1; i >= 0; i-- ) {
            // Проверочная сторона
            TVector v = TVector( wp->vertex[i],
               i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
            // проверочная точка
            CPoint cp = wp->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)(*ppl))->placement(cp);
            if( pl == PL_OUTSIDE )
               break;
            else if( pl == PL_INSIDE ) {
               isinside = true;
               break;
            }
         }
         // необходимо проверять, не лежит ли текущий внутри уже
         // проверенных... Переходим с конца списка rminus на начало списка
         // rminus1
         if( ppl == RMINUS.tail() )
            ppl = RMINUS1.head();
      }
      if( !isinside )
         RMINUS1.push(wp);  // не входит ни в один => берём
   }
   RMINUS = RMINUS1;
   RMINUS1.clear();

   // наконец, осталось распределить отрицательные контуры втнутри
   // положительных (общее действие как для объединения, так и для пересечения)
   while( !rminus.empty() ) {
      TWinPoly* wp = rminus.pop();
      bool isinside = false;
      for( TListPtr<TWinPoly*> ppl = rplus.head(); !ppl.empty() && !isinside;
         ppl++ ) {
         for( long i = wp->nVertex()-1; i >= 0; i-- ) {
            // Проверочная сторона
            TVector v = TVector( wp->vertex[i],
               i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
            // проверочная точка
            CPoint cp = wp->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)(*ppl))->placement(cp);
            if( pl == PL_OUTSIDE )
               break;
            else if( pl == PL_INSIDE ) {
               isinside = true;
               // добавляем дырку в многоугольник
               (*ppl)->addHole((TPolygon*)wp);
               break;
            }
         }
      }
   }

   // освобождаем память из-под lst
   for( int n=0; n<2; n++ )
      while( lst[n].length() > 0 )
         lst[n].pop().deletelist();

   return rplus;
}

#undef INTERSECT
#undef UNITE
#undef RPLUS
#undef RPLUS1
#undef RMINUS
#undef RMINUS1

