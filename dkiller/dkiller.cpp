//
// ��� ���������� � ������ dll ������������ �������:
// bcc32 -I.. -tWDE -DPROBLEM_CASE=x -eresult.dll file.cpp
// ��� x - ������� ������ (1 ��� 2)
//

/*************************************************************
  ������ �����-������ (������)
  / x' = -w1*y*u1/r + w2*v1
  \ y' = w1*x*u1/r + w2*v2 - w1

  P - ������� ��  (-1,-1) �� (1,1)
  Q - ���� ������� 1 � ������� � ������ ���������
      ���� �������� ������� ����� �� ����������

*************************************************************/

#include "tfield.h"
#include "dllfunc.h"
#include <math.h>

// ��������� ������ (�� ���������)...
#if PROBLEM_CASE == 1
      #define M_RADIUS  1
      #define GOAL_T    2  // ������� �����
      #define T_FIXED   false // � ������� �������
      #define DELTA     0.02  // ��� �� �������
      #define SIEVE     0.005 // ������ ����
#elif PROBLEM_CASE == 2
      #define M_RADIUS  1.5
      #define GOAL_T    36    // ������� �����
      #define T_FIXED   false // � ������� �������
      #define DELTA     0.05  // ��� �� �������
      #define SIEVE     0.02  // ������ ����
#endif

// ���������� ������, �������� ��������� M
// ��� ������, ��� ������ �������, �� ��� ������ ����� ����
// (���� � �� ��������� ����, ��� � ������ P � Q)
#ifndef M_POINTS
   #define M_POINTS  16
#else
   #if M_POINTS < 8
      #error "M_POINTS must be 8 or more"
   #endif
#endif

// ���������� ������, �������� ��������� P.
// ��� ������, ��� ������ �������, �� � ��� ������ ����� ����
#ifndef P_POINTS
   #define P_POINTS  3
#else
   #if P_POINTS < 2
      #error "P_POINTS must be 2 or more"
   #endif
#endif

// ���������� ������, �������� ��������� Q
// ��� ������, ��� ������ �������, �� � ��� ������ ����� ����
#ifndef Q_POINTS
   #define Q_POINTS  8
#else
   #if Q_POINTS < 4
      #error "P_POINTS must be 4 or more"
   #endif
#endif

#if PROBLEM_CASE == 1
   // ������ ��������� ����������
   #ifndef r
      #define r  0.5
   #endif
   // �������� ����������
   #ifndef w1
      #define w1  2.5
   #endif
#elif PROBLEM_CASE == 2
   // ������ ��������� ����������
   #ifndef r
      #define r  4.0
   #endif
   // �������� ����������
   #ifndef w1
      #define w1  2.0
   #endif
#else
   #pragma message PROBLEM_CASE macro must be defined to either 1 or 2.
   #pragma message Use -DPROBLEM_CASE=x compiler switch
   #error PROBLEM_CASE not defined
#endif

// �������� ��������
#ifndef w2
   #define w2  1.0
#endif

#ifndef M_POSITION
   #define M_POSITION 1
#endif

#ifndef M_COUNT
   #define M_COUNT 1
#elif M_COUNT == 3
      #undef M_RADIUS
   #if M_POSITION == 1
      #define M_RADIUS 0.8
      #define M_X2   (-1.5*M_RADIUS)
      #define M_Y2   (-1.5*M_RADIUS)
      #define M_X3   (+1.5*M_RADIUS)
      #define M_Y3   (-1.5*M_RADIUS)
   #elif M_POSITION == 2
      #define M_RADIUS 0.8
      #define M_X2   (-1.5*M_RADIUS)
      #define M_Y2   (-2*M_RADIUS)
      #define M_X3   (+1.2*M_RADIUS)
      #define M_Y3   (-3*M_RADIUS)
   #else
      #error "unknown value of M_POSITION"
   #endif
#else
   #error "M_COUNT must be 1 or 3"
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
   rx = cx + h*( w1*ux/r*cy - w2*vx );
   ry = cy - h*( w1*ux/r*cx + w2*vy - w1 );
}

// ------------------------------------------------------------
// ������� findPvalue - ���������� ���������� U, ����� �������
// ������������ ���� ��������� ������ ��� ��������� ��������� ���������
// ��������!  �������� ������������ � ������ �������!
// ��������� (��� - long double,
// ux � uy - �����������, ��������� ����������):
//    �������������� �����: (rx,ry)
//    ������� ����� (x):  (cx,cy)
//    ������� �������� ���������� u: (ux,uy)
//    �������� ���������� v: (vx,vy)
//    ������� ����� t: t
//    ��� �� ������� (delta): h
// ����������� ������ ������� ������ ���� ���������� ��������
// ���������� ux � uy
// ------------------------------------------------------------
#pragma argsused
extern "C" FfindPvalue findpvalue {
   if( fabsl(cy) > 1e-8 )
      ux = uy = ( ( cx - rx ) / h + w2*vx ) * r / ( w1*cy ); // �� 1-���
   else if( fabsl(cx) > 1e-8 )
      ux = uy = ( ( ry - cy ) / h - w2*vy + w1 ) * r / ( w1*cx ); // �� 2-���
   else
      ux = uy = 0; // � ��� ������???
}

// ------------------------------------------------------------
// ������� comment ���������� ������� �����������, �����������
// ������� ������
// ------------------------------------------------------------
extern "C" Fcomment comment {
   return "�����-������ [��. ������]. "
#if PROBLEM_CASE == 1
      "������� ������.";
#elif PROBLEM_CASE == 2
      "������� ������.";
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
extern "C" FPQset Qset {
   // ���������� ������������ Q_POINTS ����� �� ��������� ����������
   ldb rstep = M_PI*2/Q_POINTS;
   ldb ang = 0;
   for( int i=0; i<Q_POINTS*2; i+=2 ) {
      Q[i]   = cosl( ang );
      Q[i+1] = sinl( ang );
      ang += rstep;
   }

   // ���������� ���������� ���������
   pptr = (ldb*)&Q;
   pcount = Q_POINTS;
}

// ------------------------------------------------------------
// ������� MField ���������� ������� ��������� M ��� �������
// ������
// ------------------------------------------------------------
extern "C" FMField MField {
   static TField *M = NULL;
   if( M )
      return M;
   else
      M = new TField;
   // ���������� ������������ M_POINTS ����� �� ���������� ������� M_RADIUS
   ldb rstep = M_PI*2/M_POINTS;
   ldb ang = 0;
   TWinPoly *wp1 = new TWinPoly();
#if M_COUNT == 3
   TWinPoly *wp2 = new TWinPoly();
   TWinPoly *wp3 = new TWinPoly();
#endif
   for( int i=0; i<M_POINTS; i++ ) {
      ldb x = cosl( ang )*M_RADIUS;
      ldb y = sinl( ang )*M_RADIUS;
      *wp1 += CPoint( x, y );
#if M_COUNT == 3
      *wp2 += CPoint( x + M_X2, y + M_Y2 );
      *wp3 += CPoint( x + M_X3, y + M_Y3 );
#endif
      ang += rstep;
   }
   *M += wp1;
#if M_COUNT == 3
   *M += wp2;
   *M += wp3;
#endif
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

