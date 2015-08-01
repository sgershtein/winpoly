//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Window drawing
//  FILE:         tdraw.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class implementation for thread class that draws the field
//
//----------------------------------------------------------------------------

#include "tdraw.h"
#include "tfield.h"
#include <stdio.h>

// ������ �� ������ ��� DC ���� �� ���������������
int TDrawThread::Run() { // TThread's run

   // ������������� ��������
   TBrush polyBrush( (drawtype & useColors) ?
      TColor(255,0,0) : TColor().Black );
   TBrush holeBrush( (drawtype & useColors) ?
      TColor(200,200,255) : TColor().White );
   TBrush noneBrush(TColor().White);
   TPen penGrid(TColor(75,75,75),1,PS_DOT);
   TPen penPoly( (drawtype & useColors) ?
      TColor(255,0,0) : TColor().Black,
      1,PS_SOLID);
   TPen penHole( (drawtype & useColors) ?
      TColor(255,0,0) : TColor().Black,
      1,PS_SOLID);
   TFont fontText("Arial",14);

   DC->SelectObject(fontText);

   // ������� ����
   if( drawtype & cleanField )
      DC->FillRect(Client,noneBrush);

   if( Field->nPoly() < 1 ) { // ������ ��������
      if( drawtype & cleanField )
         DC->FillRect(Client,TBrush(TColor().SysAppWorkspace));
	   if( Invalidate )
   		Invalidate->Invalidate();
      return 0;
   }
   if( !Client.Width() || !Client.Height() )
      return 0; // ��������-�� �����...

   if( !xmin && !ymin && !ymax && !xmax ) {
      // ���� ������ ��� ������ �� �����, �� ������� ��� ��������
      xmin = Field->minx(), xmax = Field->maxx(),
      ymin = Field->miny(), ymax = Field->maxy();
   }

   /* ------ ����� ���������� ����� - ��������� �� ���� ��������������
     ������� - ���������� ��������� ... 

      #define T_WIDEN 1
      #define GOAL_T    2.25  // ������� �����
      char *bbb = getenv("T");
      long double t = bbb? _atold(bbb) : 0;
      long double mcoef = 1 + GOAL_T*T_WIDEN;
      ymax = xmax = mcoef;
      ymin = xmin = -mcoef;
      long double coef = 1 + (GOAL_T - t)*(T_WIDEN);

   /* ------ ����� ���� ����� */

   long double h = ymax-ymin, w = xmax-xmin; // ������ � ������
   h*=1.1; w*=1.1; // ��������� ����
   long double x0 = xmin-0.05*w, y0 = ymin-0.05*h; // ����� �������

   // ������� �������� �� �������� ��������� � ��������
   #define x2c(X) (((X)-x0)*Client.Width()/w)
   #define y2c(Y) (Client.Bottom()-(((Y)-y0)*Client.Height()/h))

   // ���� ������ ���������������.....
   double wh = (double)Client.Width() / Client.Height();
   if( h < w / wh ) {
      h = w / wh;
      y0 -= (h/1.1-(ymax-ymin))/2;
   } else {
      w = h * wh;
      x0 -= (w/1.1-(xmax-xmin))/2;
   }
   // ����� ���� (������ ���������������)

   /* ------------ � ��� ������� ����� - ������ ���������� ������������� --
      TPoint *vvv = new TPoint[5];
      vvv[0] = TPoint( x2c(1*coef),y2c(1*coef));
      vvv[1] = TPoint( x2c(-1*coef),y2c(1*coef));
      vvv[2] = TPoint( x2c(-1*coef),y2c(-1*coef));
      vvv[3] = TPoint( x2c(1*coef),y2c(-1*coef));
      vvv[4] = vvv[0];
      DC->SelectObject(TPen( TColor().Black,3,PS_SOLID));
      DC->Polyline(vvv,5);
   /* ------------ ����� --*/

   // ������ ��������������
   for( unsigned n = 0; n < Field->nPoly(); n++ ) {
      DC->SelectObject(polyBrush);
      DC->SelectObject(penPoly);
      TWinPoly *poly = Field->getPoly(n);
      TPoint *vert = new TPoint[poly->nVertex()+1];
      for( unsigned k = 0; k < poly->nVertex(); k++ ) {
         CPoint *P = poly->getVertex(k);
         vert[k] = TPoint(x2c(P->x),y2c(P->y));
      }
      if( drawtype & doFills )
         // ������ � ��������
         DC->Polygon(vert,poly->nVertex());
      else {
         // ������ ������ �������
         vert[poly->nVertex()] = vert[0];
         DC->Polyline(vert,poly->nVertex()+1);
      }
      delete[] vert;
   	if( ShouldTerminate() )	// ��� ������ ��� ������� � �����������
  	   	return 1;
      // ������ �����
      DC->SelectObject(holeBrush);
      DC->SelectObject(penHole);
      for( unsigned l = 0; l < poly->nHoles(); l++ ) {
         TPolygon *hole = poly->getHole(l);
         vert = new TPoint[hole->nVertex()+1];
         for( unsigned k = 0; k < hole->nVertex(); k++ ) {
            CPoint *P = hole->getVertex(k);
            vert[k] = TPoint(x2c(P->x),y2c(P->y));
         }
         if( drawtype & doFills )
            // ������ � ��������
            DC->Polygon(vert,hole->nVertex());
         else {
            // ������ ������ �������
            vert[hole->nVertex()] = vert[0];
            DC->Polyline(vert,hole->nVertex()+1);
         }
         delete[] vert;
	   	if( ShouldTerminate() )	// ��� ������ ��� ������� � �����������
  		   	return 1;
      }
      DC->SelectObject(polyBrush);
   }

   if( drawtype & drawGrid ) {
      // ���������� ��� �����
      long double hgrid = 1, wgrid = 1;
      while( h / hgrid < 10 ) hgrid /= 10;
      while( h / hgrid > 20 ) hgrid *= 10;
      while( h / hgrid < 5 ) hgrid /= 2;
      while( w / wgrid < 10 ) wgrid /= 10;
      while( w / wgrid > 20 ) wgrid *= 10;
      while( w / wgrid < 5 ) wgrid /= 2;
      double gh = hgrid*Client.Height()/h,
          gw = wgrid*Client.Width()/w;
      // ������� ��������� ����� �����
      int gx0 = x2c(int(x0/wgrid)*wgrid),
         gy0 = y2c(int(y0/hgrid)*hgrid);

      // ������ �����-�������
      DC->SelectObject(penGrid);
      DC->SetROP2(R2_MASKPEN);
      int cb = Client.Bottom();
      int cr = Client.Right();
      DC->SetTextAlign(TA_TOP & TA_LEFT);

      for( double x = gx0, xp = int(x0/wgrid)*wgrid; x <= cr;
            x += gw, xp+=wgrid ) {
         DC->MoveTo(x,0);
         DC->LineTo(x,cb);
         char str[50];
         if( EQU( xp, 0 ) )
            xp = 0;
         sprintf(str,"%g",xp);
         DC->ExtTextOut(x,1,0,NULL,str,-1);
         DC->ExtTextOut(x,cb-14,0,NULL,str,-1);
      }
      for( double y = gy0, yp = int(y0/hgrid)*hgrid; y >= 0;
            y -= gh, yp+=hgrid ) {
         DC->MoveTo(0,y);
         DC->LineTo(cr,y);
         char str[50];
         if( EQU( yp, 0 ) )
            yp = 0;
         sprintf(str,"%g",yp);
         DC->ExtTextOut(2,y,0,NULL,str,-1);
         DC->ExtTextOut(cr-20,y,0,NULL,str,-1);
      }
   }  // ��������� �����

   #undef x2c(X)
   #undef y2c(X)

   if( Invalidate )
   	Invalidate->Invalidate();

	return 0;
}