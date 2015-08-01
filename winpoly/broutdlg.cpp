//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright � 1998-1999. All Rights Reserved.
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

// ����������� ���������� �������� ��� ����������� �� � ����� ������
#ifndef CLIPBOARD_PICTURE_STRECH_FACTOR
   #define CLIPBOARD_PICTURE_STRECH_FACTOR 1
#endif

//
// �������� SAMEVXY ��������� ����������� ���������� ��������.
// ���� �� �������, �� �� ���������� �������� ���������� ����������
// ����� ���������� ���������� �������� ���������� �� ���������� ����.
// �.�. �� ����������� ���������� �������� �� ���������� ����������
// ����������� ��� ��, ��� � �������� ���������, �� � ����������� �� ���
// ���� ������� ��������� ����������.
//
// � ������ �������������� �������� ��������� SAMEVXY (��� �����������������
// �������) ���������� ���������� �� �������� ���������� � �� ����������
// �������� ������ ���������. ��� ��� ����� �� ������������ ��������
// ������������� ����������, �.�. �������, ��� �� �������� ���� ����������
// �� ��������� ����, ��� ���� ����� ���������� ������� �� ���� ���� ���������.
//
#ifndef SAMEVXY
   #define SAMEVXY 0
#endif

// ������� �������� ������� ��� drawSection
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

void TBRouteDlg::drawRoute() {  // ���������� ��������� ����������
   if( routesDrawn >= routesBuilt )
      return;
   TDC *DC = Picture->GetDC();
   if( !DC ) return;
   TRect rect = Picture->GetRect();
   TWPS *wps = TBrData::wps;

   // ������� ����� ��� ��������� ����������
   TPen rpen( (CBColor->GetCheck() == BF_CHECKED) ?
      TColor(0,0,255) : TColor().Black,
      2,PS_SOLID);
   // � ����� ����� ��� ��������� ���������� ��������
   TPen rcpen( (CBColor->GetCheck() == BF_CHECKED) ?
      TColor(0,100,100) : TColor().Black,
      2,PS_SOLID);

   // ���������� ��������� ������� � ������� (��. tdraw.cpp)
   // ������ � ������
   long double h = wps->maxy() - wps->miny(),
      w = wps->maxx() - wps->minx();
      h*=1.1; w*=1.1; // ��������� ����
   // ����� �������
   long double x0 = wps->minx() - 0.05*w,
      y0 = wps->miny() - 0.05*h;

   // ���� ������ ���������������.....
   double wh = (double)rect.Width() / rect.Height();
   if( h < w / wh ) {
      h = w / wh;
      y0 -= (h/1.1-( wps->maxy() - wps->miny() ))/2;
   } else {
      w = h * wh;
      x0 -= (w/1.1-( wps->maxx() - wps->minx() ))/2;
   }
   // ����� ���� (������ ���������������)

   // ������� �������� �� �������� ��������� � ��������
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
      // ������ ���������� ��������
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

   // ����������� routesDrawn
   routesDrawn++;

   // ������� �������� �������� �� ������
   Picture->Invalidate();

}

void TBRouteDlg::drawSection() { // ���������� ��������� �������
   TDC *DC = Picture->GetDC();
   if( !DC ) return;
   TWPS *wps = TBrData::wps;
   if( (long)plainsDrawn >= wps->getSectCount() )
      return; // ��� ��� ����������
   if( plain ) {
      if( !plain->PictureReady() )
         return; // ��� �� ����������� ���������� ������
      delete plain;
   }
   plain = wps->getSection( plainsDrawn );
   TRect rect = Picture->GetRect();

   // ������ ������� *plain �� ����� DC (��������� �����)
   // + ����������� ����� Picture->Invalidate() ����� ����� ������
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

   // ����������� plainsDrawn
   plainsDrawn += plainsDrawStep;
   if( (long)plainsDrawn >= wps->getSectCount() )
      Picture->Invalidate(); // ���������� ��������
}

// ���������� ������ ������� ��������� - ���������� �� �����
void TBRouteDlg::buildRouteStep() {
   if( routesBuilt > route.last() )
      return; // ��� ��� ���������

   // ������ ��������� �������� ������� �� ������, ������� �������
   // � ���������� �����. ������ ��������� � ����� ���������� ������,
   // � � ������� ����� - ��������.  ������� ����� ��������� ������������
   // ���������� �� ���������� (� ������), �.�. ������� �� ������� �������
   // �.�. i-��� ����� ��������� ����� �� (getSectCount()-i)-��� �������

   TBrData brd;

   // ����� ������� ����� ����������
   unsigned rno = route[routesBuilt].last();

   // ����� ������� ���������� = routesBuilt
   if( (long)route[routesBuilt].length() >= brd.wps->getSectCount() ||
      route[routesBuilt][rno].done ) {
      // ��� ����� ���� ��������� ��� ��������� ��� ���� ��� ����������
      // ������, ����� ������ ������ ������. ��������� � ��������� ����������.
      routesBuilt++;
      return;
   }

   // ����� �������� ������� � ������ �������
   unsigned sectno = brd.wps->getSectCount()-route[routesBuilt].length();
   // ����� �������� �������
   long double t = brd.wps->getSectionTime(sectno);
   // ��� �� �������� ������� �� ���������� �� �������
   long double delta = brd.wps->getSectionTime(sectno-1)-t;

   // ������ ��������� ����� ����������
   CRPoint nrp = route[routesBuilt][rno+1];
   // ������ ������� ����� ���������
   CRPoint crp = route[routesBuilt][rno];
   // ��������! ������-�� ������ �� ������� ������� TArray - ���� �����������
   // �������, �.� ������ ��� �������������� ����������/������� ����� ���������
   // ���� �������� � ������ ����� � ������.  ���������� ���� �������� �
   // ���������� ������� ���������������, ���� �������������, ��� �� �����
   // ����� ������ �������� ��������� ������ ������� �� ����������!

   int rnd; // ��������� �����...

   if( rno == 0 ) {
      // �� ��������� � ��������� ����� ����������
      // ������� ������� �������� - ��������� � ��� ����� �����
      TField *scur = brd.wps->getSection(sectno);
      crp.yrt = scur->findClosest( crp.xrt );
      delete scur;
      randomize();
#if !SAMEVXY
      // �� ������ ���� �������� ��������� ���������� ���������� ��� ��������
      rnd = random( brd.Q.length() );
      crp.yv = brd.Q[ rnd ];
   } else {
      // ��������� ��� ����������
      // ����� ���������� ���������� ��� �������� ������ ���������� ����������
      // �� ���������� ���� �������� ����������
      crp.yv = route[routesBuilt][rno-1].xv;
#endif
   }

#if SAMEVXY
   // �������� ��������� ���������� ���������� (���� � �� �� ��� ��������
   // ���������� � ���������� ��������
   rnd = random( brd.Q.length() );
   crp.yv = crp.xv = brd.Q[ rnd ];
#endif

   // ���� ��� ����� ����, ������ �� ������ � ���� ����������
   // ��� ���������� ������� ������.  � ���� ������ ������ ��������
   // ��� ��������� ������� ����� ���������� �� ���������
   if( EQU( delta, 0 ) ) {
      route[routesBuilt][rno] = route[routesBuilt][rno+1] = crp;
      return;
   }

   // �������� ����� ������ ��������� ������������ ��������
   TArray<CPoint> ypt;
   for( unsigned p=0; p<brd.P.length(); p++ )
      ypt.push( brd.next( crp.yrt, brd.P[p], crp.yv, t, delta ) );

   // ����� ��������� �����
   TArray<CPoint> yrtp;

   // ���� ����� ����� �� ����������� ��������� ������������ ��������
   // � ���������� (�� �������) �������
   TField *snext = brd.wps->getSection(sectno-1); // ��������� �������
   for( unsigned p=0; p<ypt.length(); p++ ) {
      // ���� �� ���� ��������������� ���������� �������
      for( unsigned k=0; k<snext->nPoly(); k++ ) {
         TWinPoly *wp = snext->getPoly(k);
         EPlacement pl = wp->placement(ypt[p]);
         if( pl == PL_INSIDE || pl == PL_BORDER ) {
            // ����� ��� ������. ����� �� � �������� �������
            yrtp.push( ypt[p] );
         }
         // ������� ������ ����������� ������� �� ���� �������� �����
         // �������� ��������� � ������ �� ������ �������� ��������������
         CEdge e( p?ypt[p-1]:ypt[ypt.last()] , ypt[p] );

         // ���� �� ���� ������ ������ ��������������
         for( unsigned i=0; i<wp->nVertex(); i++ ) {
            CEdge we( i?*(wp->getVertex(i-1)):*(wp->getVertex(wp->nVertex()-1)),
               *(wp->getVertex(i)) );
            CrossPoint cp = we.cross(e);
            if( cp.type() == CT_CROSS ) {
               // ����� ����� �����������
               yrtp.push( *((CPoint*)cp) );
            }
         }

         // ���� �� ���� ������ ��������������
         for( unsigned j=0; j<wp->nHoles(); j++ ) {
            TPolygon *hole = wp->getHole(j);
            // ���� �� ���� ������ �����
            for( unsigned i=0; i<hole->nVertex(); i++ ) {
               CEdge we( *(i?hole->getVertex(i-1):
                  hole->getVertex(hole->nVertex()-1)),
                  *(hole->getVertex(i)) );
               CrossPoint cp = we.cross(e);
               if( cp.type() == CT_CROSS ) {
                  // ����� ����� �����������
                  yrtp.push( *((CPoint*)cp) );
               }
            }
         }

      }
   }

   if( yrtp.length() > 0 ) {
      // �������� ��������� ������� ���� �� ��������� �����
      nrp.yrt = yrtp[ random( yrtp.length() ) ];
   } else {
      // � ����� � ��� �� �� ����� �����������!  ����� ����� �� ��������� �����
      // ���������� �������� ��������� � ��������� ������������ �������� �����
      // �� snext

      // ��������, ��� �� ����� ������ ������, �� ���� ��� ������ ������ ���������
      // ����� �� snext � ������ �� ������ ��������� ������������ � �������
      // ��������� �� ���
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

   // ��������� ������� ��� ������ �� �����
   delete snext;

   // ������ ��������� ���������� ������� ������ ��� ��������, ����� �������
   // ��������� �������� ������������ U
   crp.yu   = brd.findPvalue( crp.yrt, nrp.yrt, crp.yv, t, delta );

   // �� �������� ���������� ���������� ������� ������ ����� �� ��,
   crp.xu = crp.yu;

#if !SAMEVXY
   // � ���������� ���������� ����� ���� �����
   rnd = random( brd.Q.length() );
   crp.xv = brd.Q[rnd];
#endif

   // ������ �������� �� �������� ���������� ��� �������� �����������
   nrp.xrt = brd.next( crp.xrt, crp.xu, crp.xv, t, delta );

   // ���� ���� ������ - � ������� �������, �� ����� ���������,
   // �� �������� �� �� ��� ����
   if( !brd.wps->getHeader().fixedT ) {
      TField *M = brd.wps->getSection(0); // ������� ���������
      for( unsigned i=0; i<M->nPoly(); i++ ) {
         TWinPoly *wp = M->getPoly(i);
         EPlacement pl = wp->placement( nrp.xrt );
         if( pl == PL_INSIDE || pl == PL_BORDER ) {
            nrp.done = true;
            break;
         }
      }
   }

   // ������ ��������� � �������� ������
   route[routesBuilt][rno+1] = nrp; // ��������� ����� ����������
   route[routesBuilt][rno] = crp;   // ������� ����� ����������

   // ��� � ���. ��������� ����� ���������� ���������.
}

bool TBRouteDlg::IdleAction(long idleCount) {
   bool result;
   result = TDialog::IdleAction(idleCount);

   try {
      // ��� ������ ������ ��������� �������, �������� �������� ���������
      if( (long)plainsDrawn < TBrData::wps->getSectCount() &&
         ( !plain || plain->PictureReady() ) )
         drawSection();
      else if( routesDrawn < routesBuilt )
         drawRoute();

      // ���������� ����������
      if( routesBuilt <= route.last() &&
         ( !crthread || crthread->GetStatus() != TThread::Running ) ) {
         // ��������� crthread ��� ���������� ����������
         if( crthread )
            delete crthread;
         crthread = new TCalcRouteThread(this);
         crthread->Start();
      }

   } catch( Exception &e ) {
      char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
      MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      CmCancel();
   }

   return result;
}

// ������� ���������� ����������
int TCalcRouteThread::Run() {
   try {
      while( parent->routesBuilt <= parent->route.last() ) {
         parent->buildRouteStep();
         if( ShouldTerminate() )
            return 1;
      }
   } catch( Exception &e ) {
      char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
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

// �������� ���������
void TBRouteDlg::EvDensityScroll(uint /*code*/) {
   if( HSDensity->GetPosition() != plainsDrawStep ) {
      plainsDrawStep = HSDensity->GetPosition();
      routesDrawn = plainsDrawn = 0;
   }
}

// ������ �� ������ ��������
void TBRouteDlg::BClose() {
   CmOk();
}

// ������ �� �������� - ������� �� � ����� ������
void TBRouteDlg::BNPicture() {
   TClipboard clb = OpenClipboard();
   clb.EmptyClipboard();
   TBitmap *fbm = Picture->GetBitmap();
   if( fbm )
      fbm->ToClipboard( clb );
   clb.CloseClipboard();
   EPAdd->SetFocus();
}

// ������� ������� ����������� �� �����-����� ��� ��������.
// �������� ��� ������������
void TBRouteDlg::EvCBColor() {
   routesDrawn = plainsDrawn = 0;
}

// ��������/�������� ����� �����
// �������� ��� ������������
void TBRouteDlg::EvCBGrid() {
   routesDrawn = plainsDrawn = 0;
}

// ��������/�������� ����� ���������� ��������
// �������� ��� ������������
void TBRouteDlg::EvCBDrawCond() {
   routesDrawn = plainsDrawn = 0;
}

// ������ ������ "�������� ��������� �����"
// ����� ���������
void TBRouteDlg::BPAdd() {
   char buf[100];
   EPAdd->GetText(buf,100);
   EPAdd->SetFocus();

   // ��������� ������������ ������ � ���� �����
   char *ptr;
   long double x,y;
   x = _strtold(buf,&ptr);
   while( *ptr == ' ' ) ptr++;
   if( *ptr != '\0' ) {
      y = _strtold(ptr,&ptr);
      while( *ptr == ' ' ) ptr++;
      if( *ptr == '\0' ) {
         // ������ ���������. ��������� �� � ������
         sprintf(buf,"%Lg\t%Lg",x,y);
         EPAdd->Clear();   // ������� ������
         TPArray spt;
         spt[0].xrt = CPoint(x,y);
         int idx = LPTrace->AddString(buf);
         route.insert(spt,idx);
         if( idx < routesBuilt )
            routesBuilt = idx;
         return;
      }
   }

   // ������ �����
   MessageBox("���������� ������ ���������� ����� - ��� ����� ����� ������",
      "������ ����� ���������", MB_ICONSTOP | MB_OK);
   EPAdd->SetSelection(0,strlen(buf));
   return;

}

// ������������ ������ ������� ���� �� ��������� �����
void TBRouteDlg::BPDel() {
   int idx = LPTrace->GetSelIndex();
   if( idx >= 0 ) {
      // ���������� ���� ���������� ����������...
      if( crthread && crthread->GetStatus() == TThread::Running ) {
         crthread->TerminateAndWait();
         delete crthread;
         crthread = NULL;
      }
      // ������� ��������� ����������
      LPTrace->DeleteString(idx);
      route.remove(idx);
      if( idx < routesBuilt )
         routesBuilt--;
      LBNSelchange();
      // ���� ��� ������������
      routesDrawn = plainsDrawn = 0;
   }
   EPAdd->SetFocus();
}

// ��������� ���������� �������� ������
void TBRouteDlg::LBNSelchange() {
   BDel->EnableWindow( LPTrace->GetCount() > 0 && LPTrace->GetSelIndex() >=0 );
}

// ��������� ������ � ���� �����
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
      // �������������� ������, ���� ��� �� ������� �����
      mDC = new TMemoryDC(drawDC);
      mBitmap = new TBitmap( drawDC, mregion.Width(), mregion.Height() );
      mDC->SelectObject(*mBitmap);
      mDC->FillRect(mregion,TBrush(TColor().SysAppWorkspace));
      rect = mregion;
   }
   // �������� ������ �� ����� (��������, ���������� ��� ������)
#if CLIPBOARD_PICTURE_STRECH_FACTOR == 1
   drawDC.BitBlt( region, *mDC, TPoint(0,0) );
#else
   drawDC.StretchBlt( region, *mDC, mregion );
#endif
}


