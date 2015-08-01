//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
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

// почти ноль - для сравнения
#define EPS 1e-9
// определяем операцию сравнения двух чисел с правающей точкой
#define EQU(x,y) (fabsl((x)-(y))<=EPS)

// предварительные описания, для использования в CPoint
class TVector;
class CEdge;

// варианты расположения точки относительно направленного отрезка ----
enum EPosition { P_UNKNOWN=0,	// неизвестно
	P_LEFT	= 1 << 0,			// в левой полуплоскости
   P_RIGHT	= 1 << 1, 			// в правой полуплоскости
   P_BEYOND	= 1 << 2,         // впереди на прямой
   P_BEHIND = 1 << 3,			// позади на прямой
   P_BETWEEN= 1 << 4,			// на самом отрезке (кроме концов)
   P_START  = 1 << 5,         // конец отрезка
   P_END    = 1 << 6,			// конец отрезка
	P_ON		= 1 << 7,   		// на прямой (т.е. что-то кроме P_LEFT и P_RIGHT)
   P_EDGE   = 1 << 8,         // на отрезке (включая концы)
   P_BEY_ON = P_BEYOND+P_ON,
   P_BEH_ON = P_BEHIND+P_ON,
   P_BET_ON = P_BETWEEN+P_ON+P_EDGE,
   P_STA_ON = P_START+P_ON+P_EDGE,
   P_END_ON = P_END+P_ON+P_EDGE,
};

// варианты пересечения линий или отрезков
enum ECrosstype {
	CT_UNKNOWN   = 0,			// неизвестно
	CT_COLLINEAR = 1 << 0,  // направляющие векторы лежат на одной прямой
   CT_PARALLEL  = 1 << 1,  // направляющие векторы параллельны
   CT_SKEW      = 1 << 2,  // прямые пересекаются
	CT_CROSS     = 1 << 3,	// ребра пересекаются
   CT_SKEWCROSS = CT_SKEW+CT_CROSS // во избежание warning-а
};

// Варианты положения точки относительно многоугольника
enum EPlacement {
	PL_UNKNOWN = 0,		// положение неизвестно
   PL_INSIDE  = 1 << 1, // внутри
   PL_OUTSIDE = 1 << 2, // снаружи
   PL_BORDER  = 1 << 3 // на границе
};

// класс точки ----------------------------
class CPoint {
	public:
   	long double x,y;

      // конструкторы
      CPoint() : x(0), y(0) {};
      CPoint(const CPoint& p) : x(p.x), y(p.y) { };
      CPoint(const long double xx, const long double yy) : x(xx), y(yy) { };

      // вектор как разность двух точек
      TVector operator-(const CPoint& p) const;

      // отложить вектор от заданной точки
      CPoint operator+(const TVector& v) const;

      // сравнение точек (покоординатное)
      bool operator==(const CPoint &p) const { return EQU(p.x,x) && EQU(p.y,y); }
      bool operator!=(const CPoint &p) const { return !(*this==p); }

      // сравнение "в точности"
      bool is(const CPoint &p) const { return p.x==x && p.y==y; }

      // определение положения точки относительно двух других
      EPosition classify(const CPoint& p1, const CPoint& p2) const;
      EPosition classify(const CEdge &e) const;

      // интерпретация точки как вектора
      operator TVector();
};

// класс вектора --------------------------
class TVector { // vector definition
	public:
		long double x,y;

		// конструкторы
      TVector() : x(0), y(0) {};
      TVector(const TVector &v) : x(v.x), y(v.y) {};
		TVector(const CPoint &p1, const CPoint &p2) : x(p2.x-p1.x), y(p2.y-p1.y) {};
      TVector(const long double xx, const long double yy) : x(xx), y(yy) {};

      // длина вектора
      long double length() const {
      	return sqrtl(x*x+y*y);
      };

      // скалярное произведение векторов
      long double operator*(const TVector &v) const {
      	return v.x*x+v.y*y;
      };

      // сумма и разность векторов
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

      // умножение на скаляр
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

      // обращение вектора
      TVector operator-() const {
      	return TVector(-x,-y);
      };

		// нормализация вектора
      void normalize() {
      	*this /= length();
      };

      // полярный угол вектора (в радианах)
      long double angle() const;

      // сравнение векторов
      bool operator==(const TVector& p) const { return EQU(p.x,x) && EQU(p.y,y); }

      // интерпретация вектора как точки
      operator CPoint() const { return CPoint(x,y); }
};

// точка пересечения (или её отсутствие)
class CrossPoint {
	public:
   	// конструктор по умолчанию
   	CrossPoint() : crosspoint(0,0), crosstype(CT_UNKNOWN) {};
      // создание определенной точки пересечения
      CrossPoint(const ECrosstype CT, const CPoint p=CPoint(0,0)) :
      	crosstype(CT), crosspoint(p) {};
		// вернуть точку пересечения или NULL если такой нет
      operator const CPoint*() const {
      	return (crosstype & CT_SKEW)? &crosspoint : NULL;
      }
      // вернуть тип пересечения
      const ECrosstype type() const { return crosstype; }
   private:
   	ECrosstype crosstype;
      CPoint crosspoint;
};

// направленный отрезок или ребро ------------------------------------
class CEdge {
	protected:
   	CPoint p1,p2;
   public:
   	// инициализация - по умолчанию
      CEdge() : p1(0,0), p2(0,0) {};
   	// инициализация - по двум точкам
      CEdge(const CPoint& s, const CPoint& e) : p1(s), p2(e) {};
   	// инициализация - по точке и вектору
      CEdge(const CPoint& s, const TVector& v) : p1(s), p2(s+v) {};

   	// начало и конец отрезка (с возможностью модификации)
   	CPoint& start() { return p1; }
      CPoint& end() { return p2; }

      // обратное ребро (в противоположном направлении)
      CEdge operator-() const { return CEdge(p2,p1); }

      // преобразование в вектор
      operator TVector() const { return p2-p1; }

      // точка, на расстоянии d длин отрезка вдоль него
      CPoint point(const long double d) {
      	return p1+(p2-p1)*d;
      }

      // лежит ли точка на ребре
      bool on(const CPoint& p) const {
			return (classify(p) & P_EDGE) != 0;
      }

      // лежит ли точка на прямой, определяемой ребром
      bool onLine(const CPoint& p) const {
			return EQU((p2.x-p1.x)*(p.y-p1.y),(p2.y-p1.y)*(p.x-p1.x));
      }

      // определение положение точки относительно ребра
      EPosition classify(const CPoint& p) const;

      // найти точку пересечения с указанным ребром
      CrossPoint cross(const CEdge& e) const;

      // найти точку отрезка,ближайшую к данной
      CPoint findClosest(const CPoint &p) const;
};

// многоугольник (пока без дырок) ------------------------------------
class TPolygon {
	protected:
      TArray<CPoint> vertex;	// вершины
      bool clockwise;			// обход вершин - по часовой стрелке или наоборот

      long double xmin,ymin,xmax,ymax; // максимальные и минимальные координаты
      bool minmaxfound;				 // min-ы и max-ы подсчитаны или еще нет

      // посчитать все минимумы и максимумы. По умолчанию, считать только
      // если !mimmaxfound.  Параметр force требует безусловного пересчета
      virtual void calcminmax(bool force=false);

      // выделить место под n вершин. Все вершины после n пропадают.
      void allocate(unsigned n);

      // проверить ориентацию замкнутого контура от i-той вершины до j-той
      // и удалить замкнутый контур в случае неправильной ориентации
      // Возвращает истину, ести ориентация правильная и ложь,
      // если контур удален
      bool checkOrientation(bool positive, long i, long j,
         bool volatile *ShouldTerminate = NULL);

	public:
   	// конструкторы
		TPolygon() : 	// default constructor
      	clockwise(false), minmaxfound(false) {};
      TPolygon(unsigned n, CPoint vert[]); // construct from array of points
      TPolygon(TPolygon *P);  // построить копию многоугольника
/*      TPolygon(unsigned n); // just reserve space for n points */
/*      virtual ~TPolygon() {};	// do clean-up and free memory */

      // количество вершин
      unsigned nVertex() const { return vertex.length(); }

      // i-тая вершина
      CPoint* getVertex(unsigned i) {
      	return (i<vertex.length())? &vertex[i]:NULL;
      };

      // добавить вершину (в конец)
      void operator+=(CPoint p) {
			vertex[vertex.length()] = p;
      }

      // удалить i-тую вершину
      void remove(unsigned i) { vertex.remove(i); }

      // подсчитать минимальные и максимальные координаты вершин
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

      // местоположение точки относительно многоугольника
      EPlacement placement(CPoint& p);

      // нахождение ближайшей точки многоугольника к данной
      CPoint findClosest(CPoint& p);

      // просеивание многоугольника.  Выбрасываются все такие
      // вершины, выбрасывание которых изменяет площадь мн-ка не более,
      // чем на величину параметра sieve
      // Функция возвращает количество оставшихся вершин 
      unsigned long sift(long double siege,
         bool volatile *ShouldTerminate = NULL);

      // проверить контуры на наличие "самопересечений"
      // и избавиться от них
      // positive - положительное направление обхода (true) или дырка (false)
      // Возвращает true если итоговый полученный многоугольник
      // правильной ориентации и false в противном случае
      bool checkselfcross(bool positive,bool volatile *ShouldTerminate = NULL);

      // проверить контуры на наличие "самопересечений"
      // вернуть истину если "их есть"
      bool selfcrosscontrol(bool volatile *ShouldTerminate = NULL);
};

// многоугольник с дырками ------------------------------------------
class TWinPoly : public TPolygon {
	protected:
   	TArray<TPolygon*> hole;	// дырки

      // зарезервировать место для дырок
      void allocateHoles( unsigned n );

	public:
   	// конструкторы
      TWinPoly() : TPolygon() {};
      // построить по списку вершин и списку дырок
      // ВНИМАНИЕ! Дырки не копируются, а используются прямо
      // те, что переданы, при уничтожении многоугольника
      // дырки будут тоже удалены
      TWinPoly(unsigned nV, CPoint vv[], unsigned nH, TPolygon *hh[]);
      TWinPoly(TWinPoly *wp); // сделать копию существующего мн-ка
      virtual ~TWinPoly();	// деструктор. Всем дыркам делается delete!

      // выдать i-тую дырку
   	TPolygon *getHole(unsigned n) {
      	return ( n<hole.length())? hole[n] : NULL;
      }

      // выдать сам многоугольник (без дырок)
      TPolygon *getSelf() {
      	return this;
      }

      // выдать количество дырок
      unsigned nHoles() const {
      	return hole.length();
      }

      // добавить дырку
      void addHole(TPolygon *h) {
			hole[hole.length()] = h;
      }

      // выкинуть i-тую дырку
      void removeHole(unsigned i) {
      	if( i<hole.length() && hole[i] )
         	delete hole[i];
         hole.remove(i);
      }

      // местоположение точки относительно многоугольника
      EPlacement placement(CPoint& p);

      // нахождение ближайшей точки многоугольника к данной
      CPoint findClosest(CPoint& p);

      // пересечение двух многоугольников - возвращает результирующий
      // список многоугольников.  Текущий многоугольник не меняется
      TList<TWinPoly*> intersect(TWinPoly *,
         bool volatile *ShouldTerminate = NULL);

      // объединение двух многоугольников - возвращает результирующий
      // список многоугольников.  Текущий многоугольник не меняется
      TList<TWinPoly*> unite(TWinPoly *,
         bool volatile *ShouldTerminate = NULL);

      // просеивание многоугольника и дырок.  Выбрасываются все такие
      // вершины, выбрасывание которых изменяет площадь мн-ка не более,
      // чем на величину параметра sieve
      // Функция возвращает количество оставшихся вершин (включая дырки)
      unsigned long sift(long double siege,
         bool volatile *ShouldTerminate = NULL);

      // проверить контуры на наличие "самопересечений"
      // и избавиться от них
      void checkselfcross(bool volatile *ShouldTerminate = NULL);

      // проверить контуры на наличие самопересечений.
      // Вернгуть истину, если "их есть"
      bool selfcrosscontrol(bool volatile *ShouldTerminate = NULL);

};

#endif
