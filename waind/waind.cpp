
/*************************************************************
  ������ ��������� (?)
  / x' = x*( a*u - x ) [ + v1 ]
  \ y' = y*( b*u - y ) [ + v2 ]

  a,b - ��������� ������������� ���������
  u   - �� ������� [ -1 ; 1 ]
  v [ ���� ���� ] - ���� �� ����� �� �������

  ������� ��������� - ��� ����������
  ���������� ��������� - "�����", � ������ T "�������" ���������
  � ������� ����������.

  ����� �� ������� ???

*************************************************************/

#include "tfield.h"
#include "dllfunc.h"
#include <math.h>

#define USESECFIELD 0
#define USEPLAYER2 1

// ���������� ������� ������ (v1, v2)?
#ifndef USEPLAYER2
   #if USESECFIELD == 1
      #define USEPLAYER2 0
   #else
      #define USEPLAYER2 1
   #endif
#endif

// ���������� ����������?
#ifndef USESECFIELD
   #if USEPLAYER2 == 1
      #define USESECFIELD 0
   #else
      #define USESECFIELD 1
   #endif
#endif

#if USEPLAYER2 == 1 && USESECFIELD == 1
   #error USEPLAYER2 and USESECFIELD cannot be both activated
#endif

// ��������� ������...
#define GOAL_T    0.8  // ������� �����
#define T_FIXED   true // � ������ �������
#define DELTA     0.005  // ��� �� �������
#define SIEVE     0.005  // ������ ����

// ��������� ���������
#define a   0.2
#define b   1.2

// ���������� ������, �������� ��������� P.
// ��� ������, ��� ������ �������, �� � ��� ������ ����� ����
#ifndef P_POINTS
   #define P_POINTS  3
#else
   #if P_POINTS < 2
      #error "P_POINTS must be 2 or more"
   #endif
#endif

// ���������� ������, �������� ��������� Q.
// ��� ������, ��� ������ �������, �� � ��� ������ ����� ����
#if USEPLAYER2 == 1
   #ifndef Q_POINTS
      #define Q_POINTS  3
   #else
      #if Q_POINTS < 2
         #error "Q_POINTS must be 2 or more"
      #endif
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
#if USESECFIELD == 1
   #ifndef M_Y
      #define M_Y  (3)
   #else
      #if M_Y <= 0
         #error "M_Y must be positive"
      #endif
   #endif
#endif

// ��� ��������� ������� (�� 0 �� 1)
// T1 - ����� ���������� "������"
// T2 - ����� ������ "������" �� �����������
#if USESECFIELD == 1
   #define T1  0.3
   #define T2  1
#endif

// ��������� P. �������� ������������ ������ (��������������� x � y)
ldb P[P_POINTS*2];

// ��������� Q. �������� ������������ ������ (��������������� x � y)
ldb Q[Q_POINTS*2];

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
   #if USEPLAYER2 == 1
      rx = cx*( 1 - h*( a*ux - cx ) ) - h*vx;
      ry = cy*( 1 - h*( b*uy - cy ) ) - h*vy;
   #else
      rx = cx*( 1 - h*( a*ux - cx ) );
      ry = cy*( 1 - h*( b*uy - cy ) );
/*
      rx = cx*( 1 - h*( a + 1 ) ) + h*a*ux;
      ry = cy*( 1 - h*( b + 1 ) ) + h*b*uy;
*/
   #endif
}

// ------------------------------------------------------------
// ������� comment ���������� ������� �����������, �����������
// ������� ������
// ------------------------------------------------------------
extern "C" Fcomment comment {
   #if USESECFIELD == 1
      return "�������� (���������� ����������) - "
          "����������� � ���� \"������\"";
   #else
      return "�������� - ������ ��� ���� �������";
   #endif
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
   // ��������� P [ �� (-1,-1) �� (1,1) ]
   ldb step = 2.0/(P_POINTS-1);
   P[0] = P[1] = -1;
   for( int i=2; i<P_POINTS*2; i+=2 )
      P[i] = P[i+1] = P[i-2]+step;

   // ���������� ���������� ���������
   pptr = (ldb*)&P;
   pcount = P_POINTS;
}

#if USEPLAYER2 == 1
extern "C" FPQset Qset {
   // ���������� ������������ Q_POINTS ����� �� �������
   // ��������� Q [ �� (-1,-1) �� (1,1) ]
   ldb step = 2.0/(Q_POINTS-1);
   Q[0] = Q[1] = -1;
   for( int i=2; i<Q_POINTS*2; i+=2 )
      Q[i] = Q[i+1] = Q[i-2]+step;

   // ���������� ���������� ���������
   pptr = (ldb*)&Q;
   pcount = Q_POINTS;
}
#else
extern "C" FPQset Qset {
   // ��������� ��������� �� ����� ����� - � ������ �� ������������.
   Q[0] = Q[1] = 0;
   pptr = (ldb*)&Q;
   pcount = 1;
}
#endif

// ------------------------------------------------------------
// ������� MField ���������� ������� ��������� M ��� �������
// ������
// ------------------------------------------------------------
#if USESECFIELD == 1
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
#else
extern "C" FMField MField {
   static TField* M = NULL;
   if( M )
      delete M;
   // ���������� ������������ M_POINTS ����� �� ���������� ������� M_RADIUS
   ldb rstep = M_PI*2/M_POINTS;
   ldb ang = 0;
   TWinPoly *wp = new TWinPoly();
   for( int i=0; i<M_POINTS; i++ ) {
      ldb x = cosl( ang )*M_RADIUS;
      ldb y = sinl( ang )*M_RADIUS;
      *wp += CPoint( x, y );
      ang += rstep;
   }
   M = new TField;
   *M += wp;
   return M;
}
#endif

// ------------------------------------------------------------
// ������� SecField ���������� ���������� ��������� ���
// ���������� ������� ������� (������� ����������� ���������
// � ������ ������� t)
// ------------------------------------------------------------
#if USESECFIELD == 1
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
#endif

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

