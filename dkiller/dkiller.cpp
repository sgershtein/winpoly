//
// Для компиляции и сборки dll используется команда:
// bcc32 -I.. -tWDE -DPROBLEM_CASE=x -eresult.dll file.cpp
// где x - вариант задачи (1 или 2)
//

/*************************************************************
  Задача Шофер-убийца (Айзекс)
  / x' = -w1*y*u1/r + w2*v1
  \ y' = w1*x*u1/r + w2*v2 - w1

  P - отрезок от  (-1,-1) до (1,1)
  Q - круг радиуса 1 с центром в начале координат
      круг задается набором точек на окружности

*************************************************************/

#include "tfield.h"
#include "dllfunc.h"
#include <math.h>

// параметры задачи (по умолчанию)...
#if PROBLEM_CASE == 1
      #define M_RADIUS  1
      #define GOAL_T    2  // целевое время
      #define T_FIXED   false // к моменту времени
      #define DELTA     0.02  // шаг по времени
      #define SIEVE     0.005 // ячейка сита
#elif PROBLEM_CASE == 2
      #define M_RADIUS  1.5
      #define GOAL_T    36    // целевое время
      #define T_FIXED   false // к моменту времени
      #define DELTA     0.05  // шаг по времени
      #define SIEVE     0.02  // ячейка сита
#endif

// Количество вершин, задающих множество M
// Чем больше, тем точнее расчеты, но тем больше будет счет
// (хотя и не настолько явно, как в случае P и Q)
#ifndef M_POINTS
   #define M_POINTS  16
#else
   #if M_POINTS < 8
      #error "M_POINTS must be 8 or more"
   #endif
#endif

// Количество вершин, задающих множество P.
// Чем больше, тем точнее расчеты, но и тем дольше будет счет
#ifndef P_POINTS
   #define P_POINTS  3
#else
   #if P_POINTS < 2
      #error "P_POINTS must be 2 or more"
   #endif
#endif

// Количество вершин, задающих множество Q
// Чем больше, тем точнее расчеты, но и тем дольше будет счет
#ifndef Q_POINTS
   #define Q_POINTS  8
#else
   #if Q_POINTS < 4
      #error "P_POINTS must be 4 or more"
   #endif
#endif

#if PROBLEM_CASE == 1
   // радиус разворота автомобиля
   #ifndef r
      #define r  0.5
   #endif
   // скорость автомобиля
   #ifndef w1
      #define w1  2.5
   #endif
#elif PROBLEM_CASE == 2
   // радиус разворота автомобиля
   #ifndef r
      #define r  4.0
   #endif
   // скорость автомобиля
   #ifndef w1
      #define w1  2.0
   #endif
#else
   #pragma message PROBLEM_CASE macro must be defined to either 1 or 2.
   #pragma message Use -DPROBLEM_CASE=x compiler switch
   #error PROBLEM_CASE not defined
#endif

// скорость мальчика
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
   rx = cx + h*( w1*ux/r*cy - w2*vx );
   ry = cy - h*( w1*ux/r*cx + w2*vy - w1 );
}

// ------------------------------------------------------------
// Функция findPvalue - вычисление управления U, путем решения
// относительно него уравнения задачи при известных остальных величинах
// ВНИМАНИЕ!  Движение производится в ПРЯМОМ времени!
// Параметры (все - long double,
// ux и uy - фактические, остальные формальные):
//    Результирующая точка: (rx,ry)
//    Текущая точка (x):  (cx,cy)
//    Искомое значение управления u: (ux,uy)
//    Значение управления v: (vx,vy)
//    Текущее время t: t
//    Шаг по времени (delta): h
// Результатом работы функции должно быть присвоение значений
// параметрам ux и uy
// ------------------------------------------------------------
#pragma argsused
extern "C" FfindPvalue findpvalue {
   if( fabsl(cy) > 1e-8 )
      ux = uy = ( ( cx - rx ) / h + w2*vx ) * r / ( w1*cy ); // из 1-ого
   else if( fabsl(cx) > 1e-8 )
      ux = uy = ( ( ry - cy ) / h - w2*vy + w1 ) * r / ( w1*cx ); // из 2-ого
   else
      ux = uy = 0; // а что делать???
}

// ------------------------------------------------------------
// Функция comment возвращает краткий комментарий, описывающий
// текущую задачу
// ------------------------------------------------------------
extern "C" Fcomment comment {
   return "Шофер-убийца [см. Айзекс]. "
#if PROBLEM_CASE == 1
      "Вариант первый.";
#elif PROBLEM_CASE == 2
      "Вариант второй.";
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
extern "C" FPQset Qset {
   // равномерно распределяем Q_POINTS точек по единичной окружности
   ldb rstep = M_PI*2/Q_POINTS;
   ldb ang = 0;
   for( int i=0; i<Q_POINTS*2; i+=2 ) {
      Q[i]   = cosl( ang );
      Q[i+1] = sinl( ang );
      ang += rstep;
   }

   // возвращаем полученный результат
   pptr = (ldb*)&Q;
   pcount = Q_POINTS;
}

// ------------------------------------------------------------
// Функция MField возвращает целевое множество M для текущей
// задачи
// ------------------------------------------------------------
extern "C" FMField MField {
   static TField *M = NULL;
   if( M )
      return M;
   else
      M = new TField;
   // равномерно распределяем M_POINTS точек по окружности радиуса M_RADIUS
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
// Функция params возвращает значения параметров расчета по
// умолчанию
// ------------------------------------------------------------
extern "C" Fparams params {
   T = GOAL_T;
   sieve = SIEVE;
   fixedT = T_FIXED;
   delta = DELTA;
}

