
/*************************************************************
  Задача Вайндлеха (?)
  / x' = x*( a*u - x ) [ + v1 ]
  \ y' = y*( b*u - y ) [ + v2 ]

  a,b - небольшие положительные константы
  u   - из отрезка [ -1 ; 1 ]
  v [ если есть ] - тоже из этого же отрезка

  Целевое множество - две окружности
  Отсекающее множество - "штаны", в момент T "штанины" совпадают
  с целевым множеством.

  Время на отрезке ???

*************************************************************/

#include "tfield.h"
#include "dllfunc.h"
#include <math.h>

#define USESECFIELD 0
#define USEPLAYER2 1

// используем второго игрока (v1, v2)?
#ifndef USEPLAYER2
   #if USESECFIELD == 1
      #define USEPLAYER2 0
   #else
      #define USEPLAYER2 1
   #endif
#endif

// выживающие траектории?
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

// параметры задачи...
#define GOAL_T    0.8  // целевое время
#define T_FIXED   true // в момент времени
#define DELTA     0.005  // шаг по времени
#define SIEVE     0.005  // ячейка сита

// параметры уравнения
#define a   0.2
#define b   1.2

// Количество вершин, задающих множество P.
// Чем больше, тем точнее расчеты, но и тем дольше будет счет
#ifndef P_POINTS
   #define P_POINTS  3
#else
   #if P_POINTS < 2
      #error "P_POINTS must be 2 or more"
   #endif
#endif

// Количество вершин, задающих множество Q.
// Чем больше, тем точнее расчеты, но и тем дольше будет счет
#if USEPLAYER2 == 1
   #ifndef Q_POINTS
      #define Q_POINTS  3
   #else
      #if Q_POINTS < 2
         #error "Q_POINTS must be 2 or more"
      #endif
   #endif
#endif

// Количество вершин, задающих каждую из окружностей множества M
// Чем больше, тем точнее расчеты, но тем больше будет счет
// (хотя и не настолько явно, как в случае P и Q)
#ifndef M_POINTS
   #define M_POINTS  16
#else
   #if M_POINTS < 8
      #error "M_POINTS must be 8 or more"
   #endif
#endif

// Радиус целевого множества M
#ifndef M_RADIUS
   #define M_RADIUS  (1)
#else
   #if M_RADIUS <= 0
      #error "M_RADIUS must be positive"
   #endif
#endif

// вертикальная координата центра мн-ва M
#if USESECFIELD == 1
   #ifndef M_Y
      #define M_Y  (3)
   #else
      #if M_Y <= 0
         #error "M_Y must be positive"
      #endif
   #endif
#endif

// два параметра времени (от 0 до 1)
// T1 - точка раздвоения "штанов"
// T2 - точка выхода "штанин" на горизонталь
#if USESECFIELD == 1
   #define T1  0.3
   #define T2  1
#endif

// множество P. Задается координатами вершин (последовательно x и y)
ldb P[P_POINTS*2];

// множество Q. Задается координатами вершин (последовательно x и y)
ldb Q[Q_POINTS*2];

// ------------------------------------------------------------
// Функция calcprev - задает попятное движение для точки x при
// заданных векторах управления u,v в момент времени t, шаг по
// времени h
// Параметры (все - long double,
// rx и ry - фактические, остальные формальные):
//    Результирующая точка: (rx,ry)
//    Текущая точка (x):  (cx,cy)
//    Значение управления u: (ux,uy)
//    Значение управления v: (vx,vy)
//    Текущее время t: t
//    Шаг по времени (delta): h
// Результатом работы функции должно быть присвоение значений
// параметрам rx и ry
// ------------------------------------------------------------
#pragma argsused
extern "C" Fcalcprev calcprev {
   // метод Эйлера (попятное движение)
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
// Функция comment возвращает краткий комментарий, описывающий
// текущую задачу
// ------------------------------------------------------------
extern "C" Fcomment comment {
   #if USESECFIELD == 1
      return "Вайндлех (выживающие траектории) - "
          "ограничение в виде \"штанов\"";
   #else
      return "Вайндлех - задача для двух игроков";
   #endif
}

// ------------------------------------------------------------
// Функции Pset и Qset задают множества P и Q в виде набора
// координат вершин. Обе этих функции устанавливают значение
// своего первого аргумента в количество вершин. Значение второго
// аргумента устанавливается в указатель на массив координат
// вершин (для каждой вершины в массиве последовательно указывается
// координата x и y)
// ------------------------------------------------------------
extern "C" FPQset Pset {
   // равномерно распределяем P_POINTS точек по отрезку
   // множества P [ от (-1,-1) до (1,1) ]
   ldb step = 2.0/(P_POINTS-1);
   P[0] = P[1] = -1;
   for( int i=2; i<P_POINTS*2; i+=2 )
      P[i] = P[i+1] = P[i-2]+step;

   // возвращаем полученный результат
   pptr = (ldb*)&P;
   pcount = P_POINTS;
}

#if USEPLAYER2 == 1
extern "C" FPQset Qset {
   // равномерно распределяем Q_POINTS точек по отрезку
   // множества Q [ от (-1,-1) до (1,1) ]
   ldb step = 2.0/(Q_POINTS-1);
   Q[0] = Q[1] = -1;
   for( int i=2; i<Q_POINTS*2; i+=2 )
      Q[i] = Q[i+1] = Q[i-2]+step;

   // возвращаем полученный результат
   pptr = (ldb*)&Q;
   pcount = Q_POINTS;
}
#else
extern "C" FPQset Qset {
   // фиктивное множество из одной точки - в задаче не используется.
   Q[0] = Q[1] = 0;
   pptr = (ldb*)&Q;
   pcount = 1;
}
#endif

// ------------------------------------------------------------
// Функция MField возвращает целевое множество M для текущей
// задачи
// ------------------------------------------------------------
#if USESECFIELD == 1
extern "C" FMField MField {
   static TField* M = NULL;
   if( M )
      delete M;
   static bool init = true;
   if( init ) {
      // равномерно распределяем M_POINTS точек по окружности радиуса M_RADIUS
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
   // равномерно распределяем M_POINTS точек по окружности радиуса M_RADIUS
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
// Функция SecField возвращает отсекающее множество для
// указанного момента времени (сечение отсекающего множества
// в момент времени t)
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
      // одна штанина
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
      // наклонный участок
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
      // прямой участок
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
// Функция params возвращает значения параметров расчета по
// умолчанию
// ------------------------------------------------------------
extern "C" Fparams params {
   T = GOAL_T;
   sieve = SIEVE;
   fixedT = T_FIXED;
   delta = DELTA;
}

