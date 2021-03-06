//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         winpoly.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of Polygon classes.
//
//----------------------------------------------------------------------------

#include "tpoly.h"
#include "texcept.h"
#include <math.h>

#define EPS1 (EPS*0.001)

// ����� �� ��������� �������� ������������ ���������� �������
// ���������� ��������� � �������� ����� ������?
// ���� CLOSEST_POINT_DOUBLECHECK �������, �� ����� ���������� �����
// ����� �����������, ����� �� ��� �� �������� ������
#ifndef CLOSEST_POINT_DOUBLECHECK
   #define CLOSEST_POINT_DOUBLECHECK 0
#endif

// ����� CPoint ------------------------------------------------

TVector CPoint::operator-(const CPoint &p) const {
	return TVector(p,*this);
}

CPoint CPoint::operator+(const TVector &v) const {
	return CPoint(x+v.x,y+v.y);
}

EPosition CPoint::classify(const CPoint& p1, const CPoint& p2) const {
	return CEdge(p1,p2).classify(*this);
}

EPosition CPoint::classify(const CEdge& e) const {
	return e.classify(*this);
}

CPoint::operator TVector() {
	return TVector(x,y);
}

// ����� TVector ------------------------------------------------

long double TVector::angle() const { // �������� ����
	if( EQU(x,0) )
   	return (y>0) ? M_PI_2 : (M_PI_2*3);
	long double d = atanl(y/x);
   if( x<0 ) d = M_PI+d;
   if( d<0 ) d += 2*M_PI;
   return d;
}

// ����� CEdge --------------------------------------------------

// ����������� ��������� ����� ������������ �����
// ����� ������ "�������������� ��������� � ������������ ������� �� C++"
// �., �����, 1997, ���. 96
EPosition CEdge::classify(const CPoint &p) const {
	// ������� ������� ���������:
   //		p1 -> (0,0)
   //		p2 -> a
   //		p  -> b
	TVector a = p2 - p1;
   TVector b = p - p1;
 	long double sa = a.x * b.y - b.x * a.y;
   if( sa > EPS1 )
   	return P_LEFT;
   if( sa < -EPS1 )
   	return P_RIGHT;
   if( ( a.x*b.x < -EPS1 ) || ( a.y*b.y < -EPS1 ) )
   	return P_BEH_ON;
   if( a.length() < b.length()-EPS1 )
   	return P_BEY_ON;
   if( (p-p1).length() < EPS1 )
   	return P_STA_ON;
   if( (p-p2).length() < EPS1 )
   	return P_END_ON;
   return P_BET_ON;
}

// ����������� ����� ����������� ����� (���� ������� ����)
// ����� ������ "�������������� ��������� � ������������ ������� �� C++"
// �., �����, 1997, ���. 115
CrossPoint CEdge::cross(const CEdge& e) const {
	// ������� ������� � ������� e
	TVector n = TVector(e.p2.y-e.p1.y,e.p1.x-e.p2.x);
   // ����������� �������� ������������
   long double denom = n*(p2-p1);
   if( EQU( denom, 0 ) ) { // �� ������������
   	EPosition pos = classify(e.p1);
      if( pos == P_RIGHT || pos == P_LEFT )
      	return CrossPoint(CT_PARALLEL);
      else
      	return CrossPoint(CT_COLLINEAR);
   }
   long double t = -n*(p1-e.p1)/denom;

	// ������� ������� � ������� *this
	TVector n1 = TVector(p2.y-p1.y,p1.x-p2.x);
   long double t1 = -n1*(e.p1-p1)/(n1*(e.p2-e.p1));

	return CrossPoint( (0<=t+EPS1 && t<=1+EPS1 && 0<=t1+EPS1 && t1<=1+EPS1 )?
      CT_SKEWCROSS : CT_SKEW,	p1+(p2-p1)*t);
}

// ����� ����� �������,��������� � ������
// ������� �������� �������������� ��������� � ����,
// � ��� �������� ������. ��� �� ��� ���������?
CPoint CEdge::findClosest(const CPoint &p) const {
   // ������� ������ ��������� � p ����� ���� ������,
   // �� ������� ����� ��� �������
   long double
      x21 = p2.x - p1.x,
      y21 = p2.y - p1.y;
   long double denom = x21*x21+y21*y21;
   CPoint clp = CPoint(
      ( p1.x*y21*y21 + p.x*x21*x21 - (p1.y-p.y)*x21*y21 ) / denom,
      ( p1.y*x21*x21 + p.y*y21*y21 - (p1.x-p.x)*x21*y21 ) / denom );
#if CLOSEST_POINT_DOUBLECHECK
   if( !onLine(clp) )
      Throw("CEdge::findClosest(): ���������� ������ � ������� "
         "���������� ��������� �����");
#endif
   // clp �������� ������ ���� ��� ����� �� �������
   if( on(clp) )
      return clp;
   // �� ����� - ������ ��������� ����� ���� �� ������ �������
   if( (p1-p).length() > (p2-p).length() )
      return p2;
   else
      return p1;
}


// ����� TPolygon -----------------------------------------------

// construct from array of points
TPolygon::TPolygon(unsigned n, CPoint vert[]) :
      	clockwise(false), minmaxfound(false) {
	allocate(n);
   for( unsigned i=0; i<n; i++ )
   	vertex[i] = vert[i];
}

// ������� ����� ��������������
TPolygon::TPolygon(TPolygon *P) {
   allocate(P->vertex.length());
   for( unsigned i=0; i<P->vertex.length(); i++ )
   	vertex[i] = P->vertex[i];
   clockwise = P->clockwise;
   minmaxfound = P->minmaxfound;
   xmin = P->xmin;
   xmax = P->xmax;
   ymin = P->ymin;
   ymax = P->ymax;
}

// �������� ����� ��� n ������. ��� ������� ����� n ���������.
// ������ ��������������� � 0
void TPolygon::allocate(unsigned n) {
	vertex.setrspace(0);
	vertex.setlength(n);
   minmaxfound = false;
}

void TPolygon::calcminmax(bool force) {
	if( !force && minmaxfound )
   	return;
   minmaxfound = true;
   if( !vertex.length() ) {
		xmin = xmax = ymin = ymax = 0;
      return;
   }
   xmin = xmax = vertex[0].x;
   ymin = ymax = vertex[0].y;
   for( unsigned i = 1; i < vertex.length(); i++ ) {
   	if( vertex[i].x > xmax ) xmax = vertex[i].x;
   	if( vertex[i].y > ymax ) ymax = vertex[i].y;
   	if( vertex[i].x < xmin ) xmin = vertex[i].x;
   	if( vertex[i].y < ymin ) ymin = vertex[i].y;
   }
}

// �������������� ����� ������������ ��������������
// ������ �������� - ������� ��������� ����
EPlacement TPolygon::placement(CPoint& p) {
   if( vertex.length() < 2 )  // ������ ������� ��������������
      return PL_OUTSIDE;

   // ������� ��������� ����
   long double prev = TVector(p,vertex[vertex.last()]).angle(), angle = 0;
   for( unsigned i = 0; i < vertex.length(); i++ ) {
      // ���������, � �� ����� �� p �� ����� (i-1)-(i)
      if( CEdge(vertex[ i==0? vertex.last() : i-1 ],vertex[i]).on(p) )
         return PL_BORDER;
      // ����������� ���������� ����
      long double ang = TVector(p,vertex[i]).angle();
      if( fabsl(ang-prev) < M_PI )
         angle += ang-prev;
      else if( fabsl(ang + 2*M_PI - prev) < M_PI )
         angle += ang + 2*M_PI - prev;
      else if( fabsl(ang - 2*M_PI - prev) < M_PI )
         angle += ang - 2*M_PI - prev;
      else
         Throw("������ ��� ������������ - ���������� �������� ����");
      prev = ang;
   }

   // ������ �������, ��� � �����.  �� ������ ���� ���� 0, ���� 2 ��
   if( EQU( angle, 0 ) )
      return PL_OUTSIDE;
   else if( EQU( fabsl(angle), 2*M_PI ) )
      return PL_INSIDE;
   else Throw("TPolygon::Placement: strange angle!");

}

// ���������� ��������� ����� �������������� � ������
CPoint TPolygon::findClosest(CPoint& p) {
   if( ! ( placement(p) & PL_OUTSIDE ) )
      return p; // ����� p ����� � ��������������
   // ���������� ������� ��� ����� �������������� � ������� ��
   // ��� ��������� ����� � p. ����� ���� ����� �������� �����
   // �������
   CPoint closest = CPoint(1e4000,1e4000); // �� ����� ������� �����
   for( unsigned i = 0; i < vertex.length(); i++ ) {
      CEdge side = CEdge(i? vertex[i-1]:vertex[vertex.last()], vertex[i]);
      CPoint scl = side.findClosest(p);
      if( (scl-p).length() < (closest-p).length() )
         closest = scl;
   }
   return closest;
}


// ����������� �������������� � �����.  ������������� ��� �����
// �������, ������������ ������� �������� ������� ��-�� �� �����,
// ��� �� �������� ��������� sieve
// ������� ���������� ���������� ���������� ������ (������� �����)
unsigned long TPolygon::sift(long double siege,
   bool volatile *ShouldTerminate) {
   // ��������� �� ���� �������� � ������� �� �� ���, ��� ������� �������
   // ������� �� ������� ��������� sieve
   for( long n=0; n<=vertex.last(); n++ ) {
      CPoint p0 = n? vertex[n-1] : vertex[vertex.last()];
      CPoint p1 = n<vertex.last()? vertex[n+1] : vertex[0];

      // ������ �����
      if( (vertex[n]-p0).length() < siege ||
          (vertex[n]-p1).length() < siege ) {
         vertex.remove(n--); // ����������� ������� �������
      } else {
         // ����� ����� - ����������� �������, ���� ������, ���������
         // �� ��� � ��������� ������������, ������������� �� � �����
         // ��������� ���������, ������ siege*0.1
         TVector p0n = vertex[n]-p0;
         TVector p10 = p1 - p0;
         // alpha - ���� ����������� ��� ������� p0
         long double alpha = fabsl( p0n.angle() - p10.angle() );
         if( alpha > M_PI )
            alpha = 2*M_PI - alpha;
         // h - ������ ������������ �� ����� vertex[n]
         long double h = p0n.length() * sinl(alpha);
         if( h < siege*0.1             )
            vertex.remove(n--);
      }

      if( vertex.length() < 3 ) {
         vertex.setlength(0);
         return 0;
      }
      if( ShouldTerminate && *ShouldTerminate )
         return 0;
   }
   return nVertex();
}

// ��������� ���������� ���������� ������� �� i-��� ������� �� j-���
// � ������� ��������� ������ � ������ ������������ ����������
// ���������� ������, ���� ���������� ���������� � ����, ���� ������ ������
bool TPolygon::checkOrientation(bool positive, long i, long j,
      bool volatile *ShouldTerminate) {

   long double angle = 0; // ��������� ���� ��������
   long double aprev =
      TVector( vertex[j>0?j-1:vertex.last()],
         vertex[i] ).angle();
   for( long k=i; k != j; k == vertex.last()? k=0:k++ ) {
      long double a = TVector(vertex[k],
         vertex[(k<vertex.last())?k+1:0]).angle();
      if( fabsl(a-aprev) <= M_PI )
         angle += a-aprev;
      else if( fabsl(a + 2*M_PI - aprev) <= M_PI )
         angle += a + 2*M_PI - aprev;
      else if( fabsl(a - 2*M_PI - aprev) <= M_PI )
         angle += a - 2*M_PI - aprev;
      else
         Throw("������ ����������� ���������� �������"
            " - ���������� �������� ����");
      aprev = a;
      if( ShouldTerminate && *ShouldTerminate )
         return true;
   }
   if( EQU( angle, 2*M_PI ) ) {
      if( positive )
         return true;
   } else if( EQU( angle, -2*M_PI ) ) {
      if( !positive )
         return true;
   } else {
      // �����, ������ ����� i � j ���� �����-�� �����...
      // �� ��� �� ��� ���������, � ���� �� ������
      // ������
      return true;
   }

   // ������� ��������� ������ ������������ ���������� �� i �� j
   if( i<j )
      vertex.remove(i,j-1);
   else {
      vertex.setlength(i);
      if( j > 0 )
         vertex.remove(0,j-1);
   }
   return false;
}

// ��������� ������� �� ������� "���������������"
// ������� ������ ���� "�� ����"
bool TPolygon::selfcrosscontrol(bool volatile *ShouldTerminate) {
   for( long i=0; i<=vertex.last(); i++ ) {
      CPoint vni = vertex[ i==vertex.last()? 0:i+1 ];
      CEdge ei(vertex[i],vni);
      for( long j= ( (i+2)%vertex.length() ); j!=(i?i-1:vertex.last());
         j<vertex.last()? j++:j=0 ) {
         if( ShouldTerminate && *ShouldTerminate )
            return false;
         CPoint vnj = vertex[j==vertex.last()? 0:j+1];
         CrossPoint cxp = ei.cross(CEdge(vertex[j],vnj));
         if( cxp.type() & CT_CROSS )
            return true;
      }
   }
   return false;
}

// ��������� ������� �� ������� "���������������"
// � ���������� �� ���
// positive - ������������� ����������� ������ (true) ��� ����� (false)
bool TPolygon::checkselfcross(bool positive,bool volatile *ShouldTerminate) {
   for( long i=0; i<=vertex.last(); i++ ) {
      CPoint vni = vertex[ i==vertex.last()? 0:i+1 ];
      CEdge ei(vertex[i],vni);
      for( long j= ( (i+2)%vertex.length() ); j!=(i?i-1:vertex.last());
         j<vertex.last()? j++:j=0 ) {
         if( ShouldTerminate && *ShouldTerminate )
            return true;
         CPoint vnj = vertex[j==vertex.last()? 0:j+1];
         CrossPoint cxp = ei.cross(CEdge(vertex[j],vnj));
         if( cxp.type() & CT_CROSS ) { // ���� ������������

            CPoint *xp = cxp;

            // ������� ����� ����������� �� ��� �����
            if( *xp == vni ) {
               // ����������� � �������� ����� �������, �� ���������� ���
               // �� ��������� ������� (����� ��� ����� � ��������� ����� :)
               continue;
            } else if( *xp != vertex[i] ) {
               // ����������� � �������� ������� - ��������� � ��� �����
               // �������������� �������
               if( j > i ) j++; // �.�. ��� ��������� ����������
               vertex.insert(*xp,++i);
            }
            if( *xp == vnj ) {
               // ����������� � �������� ����� �������, �� ���������� ���
               // �� ��������� ������� (����� ��� ����� � ��������� ����� :)
               continue;
            } else if( *xp != vertex[j] ) {
               // ����������� � �������� ������� - ��������� � ��� �����
               // �������������� �������
               if( i > j ) i++; // �.�. ��� ��������� ����������
               vertex.insert(*xp,++j);
            }

            // ������ vertex[i] == vertex[j] - ��� ��������� ����� �����������
            // ��������� ����������� ������� �� i �� j �
            // ������� ���, ���� ���������� �� �����
            if( !checkOrientation(positive,i,j) ) {
               // ���������� �� �����
               // �������� ��� � ������
               i=0; break;
            } else {
               break;
               // ��� ������ ������ ������ ���, �.�. ����� ������
               // 2*M_PI (���� ������������������ ������ ����������� ����)
               // ���� ��-��������, �� ���� ������ ���� �������� � ����
               // ���������� ��������������.  �� � ������ ���������
               // ���������� ������ ��������������� ��������� ���������
               // ����������, � ��������� ���� ���������������� ��� ����� �����
               // ����, � �.�.
            }

         } // if ������� �����������
      } // for j
   } // for i

   // ������ ��������� ��������� ���� �� ������������� �������
   long double angle = 0; // ��������� ���� ��������
   long double aprev =
      TVector( vertex[vertex.last()], vertex[0] ).angle();
   for( long k=0; k <= vertex.last(); k++ ) {
      long double a = TVector(vertex[k],
         vertex[(k<vertex.last())?k+1:0]).angle();
      if( fabsl(a-aprev) <= M_PI )
         angle += a-aprev;
      else if( fabsl(a + 2*M_PI - aprev) <= M_PI )
         angle += a + 2*M_PI - aprev;
      else if( fabsl(a - 2*M_PI - aprev) <= M_PI )
         angle += a - 2*M_PI - aprev;
      else
         Throw("������ ����������� ���������� �������"
            " - ���������� �������� ����");
      aprev = a;
      if( ShouldTerminate && *ShouldTerminate )
         return true;
   }

   if( EQU( angle, 2*M_PI ) ) {
      return positive;
   } else if( EQU( angle, -2*M_PI ) ) {
      return !positive;
   } else {
      Throw("���������� ���������� ��������� ������� - �� ���� � �� �����!");
   }

}

// ����� TWinPoly -----------------------------------------------

TWinPoly::TWinPoly(unsigned nV, CPoint vv[], unsigned nH, TPolygon *hh[]) :
	TPolygon(nV, vv) {
	allocateHoles(nH);
   for( unsigned i=0; i<nH; i++ )
   	hole[i] = hh[i];
}

// ������� ����� ������������� ��-��
TWinPoly::TWinPoly(TWinPoly *wp) : TPolygon( wp ) {
   allocateHoles(wp->nHoles());
   for( unsigned i=0; i<wp->nHoles(); i++ )
   	hole[i] = new TPolygon(wp->hole[i]);
/*   minmaxfound = wp->minmaxfound;
   xmin = wp->xmin;
   xmax = wp->xmax;
   ymin = wp->ymin;
   ymax = wp->ymax; - ��� ��� ������� ��� ����������� TPolygon */
}

void TWinPoly::allocateHoles( unsigned n ) {
	hole.setlength(n);
   minmaxfound = false;
}

TWinPoly::~TWinPoly() { // ����������
	for( unsigned i=0; i<hole.length(); i++ )
   	if( hole[i] )
      	delete hole[i];
}

// �������������� ����� ������������ ��������������
EPlacement TWinPoly::placement(CPoint& p) {
	EPlacement pl0 = TPolygon::placement(p);
	if( pl0 != PL_INSIDE )
   	return pl0;				 	// �� ������ ������ => �� ������� ��� �������
	for( unsigned i=0; i<nHoles(); i++ ) {
   	EPlacement pl = hole[i]->placement(p);
      if( pl & PL_BORDER )
      	return PL_BORDER;	 	// �� ���������� �������
      if( pl & PL_INSIDE )
      	return PL_OUTSIDE; 	// � ����� => �������
   }
   return PL_INSIDE;				// �� � ����� �� ����� => ������
}

// ���������� ��������� ����� �������������� � ������
CPoint TWinPoly::findClosest(CPoint& p) {
   if( ! ( placement(p) & PL_OUTSIDE ) )
      return p; // ����� p ����� � ��������������
   CPoint closest = getSelf()->findClosest(p);
   for( unsigned i = 0; i < hole.length(); i++ ) {
      CPoint scl = hole[i]->findClosest(p);
      if( (scl-p).length() < (closest-p).length() )
         closest = scl;
   }
   return closest;
}

// ����������� �������������� � �����.  ������������� ��� �����
// �������, ������������ ������� �������� ������� ��-�� �� �����,
// ��� �� �������� ��������� sieve
// ������� ���������� ���������� ���������� ������ (������� �����)
unsigned long TWinPoly::sift(long double siege,
   bool volatile *ShouldTerminate) {
   unsigned long vc = 0;
   for( unsigned i=0; i<nHoles(); i++ ) {
      unsigned long vcc = hole[i]->sift(siege, ShouldTerminate);
      if( vcc >=3 )
         vc += vcc;
      else
         hole.remove(i--);
      if( ShouldTerminate && *ShouldTerminate )
         return 0;
   }
   return vc + getSelf()->sift(siege,ShouldTerminate);
}

// ��������� ������� �� ������� "���������������"
// � ���������� �� ���
void TWinPoly::checkselfcross(bool volatile *ShouldTerminate) {
   if( ! getSelf()->checkselfcross(true,ShouldTerminate) )
      Throw("�� ������� ������������� ������!");
   for( unsigned i=0; i<nHoles(); i++ ) {
      if( ShouldTerminate && *ShouldTerminate )
         return;
      // ����������� ����� ��� ������ ��, ���� ��� ����������.
      if( ! hole[i]->checkselfcross(false,ShouldTerminate) ||
         hole[i]->nVertex() < 3 )
         hole.remove(i);
   }
}

// ��������� ������� �� ������� "���������������"
// ������� ������, ���� "�� ����"
bool TWinPoly::selfcrosscontrol(bool volatile *ShouldTerminate) {
   if( getSelf()->selfcrosscontrol(ShouldTerminate) )
      return true;
   for( unsigned i=0; i<nHoles(); i++ ) {
      if( ShouldTerminate && *ShouldTerminate )
         break;
      if( hole[i]->selfcrosscontrol(ShouldTerminate) )
         return true;
   }
   return false;
}


// ���������� ����, ����������� ������� ����������� � �����������
#define IU 'I'
#include "intersec.h"   // ������� �����������
#undef IU
#define IU 'U'
#include "intersec.h"   // ������� �����������
#undef IU

