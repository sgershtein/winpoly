//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         tpoly.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TPoly and other Polygon-related classes
//  These classes should be platform-independent
//
//----------------------------------------------------------------------------

#ifndef _TPOLY_H_
#define _TPOLY_H_

#include <stddef.h>
#include <math.h>
#include "tarray.h"
#include "tlist.h"

// ����� ���� - ��� ���������
#define EPS 1e-9
// ���������� �������� ��������� ���� ����� � ��������� ������
#define EQU(x,y) (fabsl((x)-(y))<=EPS)

// ��������������� ��������, ��� ������������� � CPoint
class TVector;
class CEdge;

// �������� ������������ ����� ������������ ������������� ������� ----
enum EPosition { P_UNKNOWN=0,	// ����������
	P_LEFT	= 1 << 0,			// � ����� �������������
   P_RIGHT	= 1 << 1, 			// � ������ �������������
   P_BEYOND	= 1 << 2,         // ������� �� ������
   P_BEHIND = 1 << 3,			// ������ �� ������
   P_BETWEEN= 1 << 4,			// �� ����� ������� (����� ������)
   P_START  = 1 << 5,         // ����� �������
   P_END    = 1 << 6,			// ����� �������
	P_ON		= 1 << 7,   		// �� ������ (�.�. ���-�� ����� P_LEFT � P_RIGHT)
   P_EDGE   = 1 << 8,         // �� ������� (������� �����)
   P_BEY_ON = P_BEYOND+P_ON,
   P_BEH_ON = P_BEHIND+P_ON,
   P_BET_ON = P_BETWEEN+P_ON+P_EDGE,
   P_STA_ON = P_START+P_ON+P_EDGE,
   P_END_ON = P_END+P_ON+P_EDGE,
};

// �������� ����������� ����� ��� ��������
enum ECrosstype {
	CT_UNKNOWN   = 0,			// ����������
	CT_COLLINEAR = 1 << 0,  // ������������ ������� ����� �� ����� ������
   CT_PARALLEL  = 1 << 1,  // ������������ ������� �����������
   CT_SKEW      = 1 << 2,  // ������ ������������
	CT_CROSS     = 1 << 3,	// ����� ������������
   CT_SKEWCROSS = CT_SKEW+CT_CROSS // �� ��������� warning-�
};

// �������� ��������� ����� ������������ ��������������
enum EPlacement {
	PL_UNKNOWN = 0,		// ��������� ����������
   PL_INSIDE  = 1 << 1, // ������
   PL_OUTSIDE = 1 << 2, // �������
   PL_BORDER  = 1 << 3 // �� �������
};

// ����� ����� ----------------------------
class CPoint {
	public:
   	long double x,y;

      // ������������
      CPoint() : x(0), y(0) {};
      CPoint(const CPoint& p) : x(p.x), y(p.y) { };
      CPoint(const long double xx, const long double yy) : x(xx), y(yy) { };

      // ������ ��� �������� ���� �����
      TVector operator-(const CPoint& p) const;

      // �������� ������ �� �������� �����
      CPoint operator+(const TVector& v) const;

      // ��������� ����� (��������������)
      bool operator==(const CPoint &p) const { return EQU(p.x,x) && EQU(p.y,y); }
      bool operator!=(const CPoint &p) const { return !(*this==p); }

      // ��������� "� ��������"
      bool is(const CPoint &p) const { return p.x==x && p.y==y; }

      // ����������� ��������� ����� ������������ ���� ������
      EPosition classify(const CPoint& p1, const CPoint& p2) const;
      EPosition classify(const CEdge &e) const;

      // ������������� ����� ��� �������
      operator TVector();
};

// ����� ������� --------------------------
class TVector { // vector definition
	public:
		long double x,y;

		// ������������
      TVector() : x(0), y(0) {};
      TVector(const TVector &v) : x(v.x), y(v.y) {};
		TVector(const CPoint &p1, const CPoint &p2) : x(p2.x-p1.x), y(p2.y-p1.y) {};
      TVector(const long double xx, const long double yy) : x(xx), y(yy) {};

      // ����� �������
      long double length() const {
      	return sqrtl(x*x+y*y);
      };

      // ��������� ������������ ��������
      long double operator*(const TVector &v) const {
      	return v.x*x+v.y*y;
      };

      // ����� � �������� ��������
      TVector operator+(const TVector &v) const {
	      return TVector(x+v.x,y+v.y);
      };
      TVector operator-(const TVector &v) const {
	      return TVector(x-v.x,y-v.y);
      };
      TVector& operator+=(const TVector &v) {
      	x+=v.x;
         y+=v.y;
         return *this;
      };
      TVector& operator-=(const TVector &v) {
      	x-=v.x;
         y-=v.y;
         return *this;
      };

      // ��������� �� ������
      TVector operator*(const long double d) const {
      	return TVector(x*d,y*d);
      };
      friend TVector operator*(const long double d,const TVector& v) {
      	return v*d;
      }
      TVector operator/(const long double d) const {
      	return TVector(x/d,y/d);
      };
      TVector& operator*=(const long double d) {
      	x*=d;
         y*=d;
         return *this;
      };
      TVector& operator/=(const long double d) {
      	x/=d;
         y/=d;
         return *this;
      };

      // ��������� �������
      TVector operator-() const {
      	return TVector(-x,-y);
      };

		// ������������ �������
      void normalize() {
      	*this /= length();
      };

      // �������� ���� ������� (� ��������)
      long double angle() const;

      // ��������� ��������
      bool operator==(const TVector& p) const { return EQU(p.x,x) && EQU(p.y,y); }

      // ������������� ������� ��� �����
      operator CPoint() const { return CPoint(x,y); }
};

// ����� ����������� (��� � ����������)
class CrossPoint {
	public:
   	// ����������� �� ���������
   	CrossPoint() : crosspoint(0,0), crosstype(CT_UNKNOWN) {};
      // �������� ������������ ����� �����������
      CrossPoint(const ECrosstype CT, const CPoint p=CPoint(0,0)) :
      	crosstype(CT), crosspoint(p) {};
		// ������� ����� ����������� ��� NULL ���� ����� ���
      operator const CPoint*() const {
      	return (crosstype & CT_SKEW)? &crosspoint : NULL;
      }
      // ������� ��� �����������
      const ECrosstype type() const { return crosstype; }
   private:
   	ECrosstype crosstype;
      CPoint crosspoint;
};

// ������������ ������� ��� ����� ------------------------------------
class CEdge {
	protected:
   	CPoint p1,p2;
   public:
   	// ������������� - �� ���������
      CEdge() : p1(0,0), p2(0,0) {};
   	// ������������� - �� ���� ������
      CEdge(const CPoint& s, const CPoint& e) : p1(s), p2(e) {};
   	// ������������� - �� ����� � �������
      CEdge(const CPoint& s, const TVector& v) : p1(s), p2(s+v) {};

   	// ������ � ����� ������� (� ������������ �����������)
   	CPoint& start() { return p1; }
      CPoint& end() { return p2; }

      // �������� ����� (� ��������������� �����������)
      CEdge operator-() const { return CEdge(p2,p1); }

      // �������������� � ������
      operator TVector() const { return p2-p1; }

      // �����, �� ���������� d ���� ������� ����� ����
      CPoint point(const long double d) {
      	return p1+(p2-p1)*d;
      }

      // ����� �� ����� �� �����
      bool on(const CPoint& p) const {
			return (classify(p) & P_EDGE) != 0;
      }

      // ����� �� ����� �� ������, ������������ ������
      bool onLine(const CPoint& p) const {
			return EQU((p2.x-p1.x)*(p.y-p1.y),(p2.y-p1.y)*(p.x-p1.x));
      }

      // ����������� ��������� ����� ������������ �����
      EPosition classify(const CPoint& p) const;

      // ����� ����� ����������� � ��������� ������
      CrossPoint cross(const CEdge& e) const;

      // ����� ����� �������,��������� � ������
      CPoint findClosest(const CPoint &p) const;
};

// ������������� (���� ��� �����) ------------------------------------
class TPolygon {
	protected:
      TArray<CPoint> vertex;	// �������
      bool clockwise;			// ����� ������ - �� ������� ������� ��� ��������

      long double xmin,ymin,xmax,ymax; // ������������ � ����������� ����������
      bool minmaxfound;				 // min-� � max-� ���������� ��� ��� ���

      // ��������� ��� �������� � ���������. �� ���������, ������� ������
      // ���� !mimmaxfound.  �������� force ������� ������������ ���������
      virtual void calcminmax(bool force=false);

      // �������� ����� ��� n ������. ��� ������� ����� n ���������.
      void allocate(unsigned n);

      // ��������� ���������� ���������� ������� �� i-��� ������� �� j-���
      // � ������� ��������� ������ � ������ ������������ ����������
      // ���������� ������, ���� ���������� ���������� � ����,
      // ���� ������ ������
      bool checkOrientation(bool positive, long i, long j,
         bool volatile *ShouldTerminate = NULL);

	public:
   	// ������������
		TPolygon() : 	// default constructor
      	clockwise(false), minmaxfound(false) {};
      TPolygon(unsigned n, CPoint vert[]); // construct from array of points
      TPolygon(TPolygon *P);  // ��������� ����� ��������������
/*      TPolygon(unsigned n); // just reserve space for n points */
/*      virtual ~TPolygon() {};	// do clean-up and free memory */

      // ���������� ������
      unsigned nVertex() const { return vertex.length(); }

      // i-��� �������
      CPoint* getVertex(unsigned i) {
      	return (i<vertex.length())? &vertex[i]:NULL;
      };

      // �������� ������� (� �����)
      void operator+=(CPoint p) {
			vertex[vertex.length()] = p;
      }

      // ������� i-��� �������
      void remove(unsigned i) { vertex.remove(i); }

      // ���������� ����������� � ������������ ���������� ������
		long double minx() {
      	if( !minmaxfound ) calcminmax();
         return xmin;
      }
      long double maxx() {
      	if( !minmaxfound ) calcminmax();
         return xmax;
      }
      long double miny() {
      	if( !minmaxfound ) calcminmax();
         return ymin;
      }
      long double maxy() {
      	if( !minmaxfound ) calcminmax();
         return ymax;
      }

      // �������������� ����� ������������ ��������������
      EPlacement placement(CPoint& p);

      // ���������� ��������� ����� �������������� � ������
      CPoint findClosest(CPoint& p);

      // ����������� ��������������.  ������������� ��� �����
      // �������, ������������ ������� �������� ������� ��-�� �� �����,
      // ��� �� �������� ��������� sieve
      // ������� ���������� ���������� ���������� ������ 
      unsigned long sift(long double siege,
         bool volatile *ShouldTerminate = NULL);

      // ��������� ������� �� ������� "���������������"
      // � ���������� �� ���
      // positive - ������������� ����������� ������ (true) ��� ����� (false)
      // ���������� true ���� �������� ���������� �������������
      // ���������� ���������� � false � ��������� ������
      bool checkselfcross(bool positive,bool volatile *ShouldTerminate = NULL);

      // ��������� ������� �� ������� "���������������"
      // ������� ������ ���� "�� ����"
      bool selfcrosscontrol(bool volatile *ShouldTerminate = NULL);
};

// ������������� � ������� ------------------------------------------
class TWinPoly : public TPolygon {
	protected:
   	TArray<TPolygon*> hole;	// �����

      // ��������������� ����� ��� �����
      void allocateHoles( unsigned n );

	public:
   	// ������������
      TWinPoly() : TPolygon() {};
      // ��������� �� ������ ������ � ������ �����
      // ��������! ����� �� ����������, � ������������ �����
      // ��, ��� ��������, ��� ����������� ��������������
      // ����� ����� ���� �������
      TWinPoly(unsigned nV, CPoint vv[], unsigned nH, TPolygon *hh[]);
      TWinPoly(TWinPoly *wp); // ������� ����� ������������� ��-��
      virtual ~TWinPoly();	// ����������. ���� ������ �������� delete!

      // ������ i-��� �����
   	TPolygon *getHole(unsigned n) {
      	return ( n<hole.length())? hole[n] : NULL;
      }

      // ������ ��� ������������� (��� �����)
      TPolygon *getSelf() {
      	return this;
      }

      // ������ ���������� �����
      unsigned nHoles() const {
      	return hole.length();
      }

      // �������� �����
      void addHole(TPolygon *h) {
			hole[hole.length()] = h;
      }

      // �������� i-��� �����
      void removeHole(unsigned i) {
      	if( i<hole.length() && hole[i] )
         	delete hole[i];
         hole.remove(i);
      }

      // �������������� ����� ������������ ��������������
      EPlacement placement(CPoint& p);

      // ���������� ��������� ����� �������������� � ������
      CPoint findClosest(CPoint& p);

      // ����������� ���� ��������������� - ���������� ��������������
      // ������ ���������������.  ������� ������������� �� ��������
      TList<TWinPoly*> intersect(TWinPoly *,
         bool volatile *ShouldTerminate = NULL);

      // ����������� ���� ��������������� - ���������� ��������������
      // ������ ���������������.  ������� ������������� �� ��������
      TList<TWinPoly*> unite(TWinPoly *,
         bool volatile *ShouldTerminate = NULL);

      // ����������� �������������� � �����.  ������������� ��� �����
      // �������, ������������ ������� �������� ������� ��-�� �� �����,
      // ��� �� �������� ��������� sieve
      // ������� ���������� ���������� ���������� ������ (������� �����)
      unsigned long sift(long double siege,
         bool volatile *ShouldTerminate = NULL);

      // ��������� ������� �� ������� "���������������"
      // � ���������� �� ���
      void checkselfcross(bool volatile *ShouldTerminate = NULL);

      // ��������� ������� �� ������� ���������������.
      // �������� ������, ���� "�� ����"
      bool selfcrosscontrol(bool volatile *ShouldTerminate = NULL);

};

#endif
