
/*************************************************************
  ��������� ���-���-����
    x'' + k * ( x*x -1 ) * x' + x = u

  ������� ��� � ���� �������, �����

  / x' = y
  \ y' = -x - k*y*( x*x - 1 ) + u

  u   - �� ������� [ p ; q ]  ( p,q > 0 )

  ������� ��������� - ��� ����������
  ���������� ��������� - "�����", � ������ T "�������" ���������
  � ������� ����������.

  ����� �� ������� ???

*************************************************************/

#include "tfield.h"
#include "dllfunc.h"
#include <math.h>

// ��������� ������...
#define GOAL_T    1  // ������� �����
#define T_FIXED   true // � ������ �������
#define DELTA     0.01  // ��� �� �������
#define SIEVE     0.01  // ������ ����

// ��������� ���������
#define p   1
#define q   10
#define K   2

// ���������� ������, �������� ��������� P.
// ��� ������, ��� ������ �������, �� � ��� ������ ����� ����
#ifndef P_POINTS
   #define P_POINTS  3
#else
   #if P_POINTS < 2
      #error "P_POINTS must be 8 or more"
   #endif
#endif

// ���������� ������, �������� ������ �� ����������� ��������� M
// ��� ������, ��� ������ �������, �� ��� ������ ����� ����
// (���� � �� ��������� ����, ��� � ������ P � Q)
#ifndef M_POINTS
   #define M_POINTS  16
#else
   #if M_POINTS < 8
      #error "M_POINTS must be 8 or more"
   #endif
#endif

// ������ �������� ��������� M
#ifndef M_RADIUS
   #define M_RADIUS  (1)
#else
   #if M_RADIUS <= 0
      #error "M_RADIUS must be positive"
   #endif
#endif

// ������������ ���������� ������ ��-�� M
#ifndef M_Y
   #define M_Y  (2)
#else
   #if M_Y <= 0
      #error "M_Y must be positive"
   #endif
#endif

// ��� ��������� ������� (�� 0 �� 1)
// T1 - ����� ���������� "������"
// T2 - ����� ������ "������" �� �����������
#define T1  0.3
#define T2  1

// ��������� P. �������� ������������ ������ (��������������� x � y)
ldb P[P_POINTS*2];

// ��������� Q. �������� ������������ ������ (��������������� x � y)
ldb Q[2];

// ------------------------------------------------------------
// ������� calcprev - ������ �������� �������� ��� ����� x ���
// �������� �������� ���������� u,v � ������ ������� t, ��� ��
// ������� h
// ��������� (��� - long double,
// rx � ry - �����������, ��������� ����������):
//    �������������� �����: (rx,ry)
//    ������� ����� (x):  (cx,cy)
//    �������� ���������� u: (ux,uy)
//    �������� ���������� v: (vx,vy)
//    ������� ����� t: t
//    ��� �� ������� (delta): h
// ����������� ������ ������� ������ ���� ���������� ��������
// ���������� rx � ry
// ------------------------------------------------------------
#pragma argsused
extern "C" Fcalcprev calcprev {
   // ����� ������ (�������� ��������)
   rx = cx - h*cy;
   ry = cy + h*( cx + K*cy*( cx*cx - 1 ) - uy );
}

// ------------------------------------------------------------
// ������� comment ���������� ������� �����������, �����������
// ������� ������
// ------------------------------------------------------------
extern "C" Fcomment comment {
   return "��������� ���-���-���� (���������� ����������) - "
       "����������� � ���� \"������\"";
}

// ------------------------------------------------------------
// ������� Pset � Qset ������ ��������� P � Q � ���� ������
// ��������� ������. ��� ���� ������� ������������� ��������
// ������ ������� ��������� � ���������� ������. �������� �������
// ��������� ��������������� � ��������� �� ������ ���������
// ������ (��� ������ ������� � ������� ��������������� �����������
// ���������� x � y)
// ------------------------------------------------------------
extern "C" FPQset Pset {
   // ���������� ������������ P_POINTS ����� �� �������
   // ��������� P [ �� (p,p) �� (q,q) ]
   ldb step = (q-p)/(P_POINTS-1);
   P[0] = P[1] = p;
   for( int i=2; i<P_POINTS*2; i+=2 )
      P[i] = P[i+1] = P[i-2]+step;

   // ���������� ���������� ���������
   pptr = (ldb*)&P;
   pcount = P_POINTS;
}

extern "C" FPQset Qset {
   // ��������� ��������� �� ����� ����� - � ������ �� ������������.
   Q[0] = Q[1] = 0;
   pptr = (ldb*)&Q;
   pcount = 1;
}

// ------------------------------------------------------------
// ������� MField ���������� ������� ��������� M ��� �������
// ������
// ------------------------------------------------------------
extern "C" FMField MField {
   static TField* M = NULL;
   if( M )
      delete M;
   static bool init = true;
   if( init ) {
      // ���������� ������������ M_POINTS ����� �� ���������� ������� M_RADIUS
      ldb rstep = M_PI*2/M_POINTS;
      ldb ang = 0;
      TWinPoly *wp1 = new TWinPoly();
      TWinPoly *wp2 = new TWinPoly();
      for( int i=0; i<M_POINTS; i++ ) {
         ldb x = cosl( ang )*M_RADIUS;
         ldb y = sinl( ang )*M_RADIUS;
         *wp1 += CPoint( x, y + M_Y );
         *wp2 += CPoint( x, y - M_Y );
         ang += rstep;
      }
      TField M1, M2;
      M1 += wp1;
      M2 += wp2;
      TList<TWinPoly*> uns = M1.unite(&M2);
      M = new TField( uns );
      uns.clear();
      init = false;
   }
   return M;
}

// ------------------------------------------------------------
// ������� SecField ���������� ���������� ��������� ���
// ���������� ������� ������� (������� ����������� ���������
// � ������ ������� t)
// ------------------------------------------------------------
#pragma argsused
extern "C" FSecField SecField {
   static TField* M = NULL;
   if( M )
      delete M;
   ldb tc = t/GOAL_T;
   ldb rstep = M_PI*2/M_POINTS;
   ldb ang = 0;
   if( tc < T1 ) {
      // ���� �������
      M = new TField();
      TWinPoly *wp = new TWinPoly();
      for( int i=0; i<M_POINTS; i++ ) {
         ldb x = cosl( ang )*M_RADIUS;
         ldb y = sinl( ang )*M_RADIUS;
         *wp += CPoint( x, y );
         ang += rstep;
      }
      *M += wp;
   } else if( tc <= T2 ) {
      // ��������� �������
      ldb off = (tc-T1)/(T2-T1)*M_Y;
      TWinPoly *wp1 = new TWinPoly();
      TWinPoly *wp2 = new TWinPoly();
      for( int i=0; i<M_POINTS; i++ ) {
         ldb x = cosl( ang )*M_RADIUS;
         ldb y = sinl( ang )*M_RADIUS;
         *wp1 += CPoint( x, y + off );
         *wp2 += CPoint( x, y - off );
         ang += rstep;
      }
      TField M1, M2;
      M1 += wp1;
      M2 += wp2;
      TList<TWinPoly*> uns = M1.unite(&M2);
      M = new TField( uns );
      uns.clear();
   } else {
      // ������ �������
      TWinPoly *wp1 = new TWinPoly();
      TWinPoly *wp2 = new TWinPoly();
      for( int i=0; i<M_POINTS; i++ ) {
         ldb x = cosl( ang )*M_RADIUS;
         ldb y = sinl( ang )*M_RADIUS;
         *wp1 += CPoint( x, y + M_Y );
         *wp2 += CPoint( x, y - M_Y );
         ang += rstep;
      }
      TField M1, M2;
      M1 += wp1;
      M2 += wp2;
      TList<TWinPoly*> uns = M1.unite(&M2);
      M = new TField( uns );
      uns.clear();
   }

   return M;
}

// ------------------------------------------------------------
// ������� params ���������� �������� ���������� ������� ��
// ���������
// ------------------------------------------------------------
extern "C" Fparams params {
   T = GOAL_T;
   sieve = SIEVE;
   fixedT = T_FIXED;
   delta = DELTA;
}

