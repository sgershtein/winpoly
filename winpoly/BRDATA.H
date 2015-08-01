//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         brdata.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TBrData class - initial data for stable bridge
//  (goal set, Q and P sets, differential functions...)
//
//----------------------------------------------------------------------------

#ifndef _BRDATA_H_
#define _BRDATA_H_

#include "tarray.h"
#include "tfield.h"
#include "tpoly.h"
#include "twps.h"

class TBrData {
   public:
      static TField *M;          // целевое множество 1-ого игрока
      static TArray< CPoint > P; // набор точек вдоль границы множества P
                                 // векторов u (управление 1-ого игрока)
      static TArray< CPoint > Q; // набор точек вдоль границы множества Q
                                 // векторов v (управление 2-ого игрока)
      static long double delta;  // шаг разбиени€ по времени
      static long double T;      // ÷елевое врем€ (T)
      static long double sieve;  // "сито". ¬ершина многоугольника будет
                                 // выброшена, если при этом его площадь
                                 // изменитс€ не более, чем на величину sieve
      static bool fixedT;        // в фиксированный момент времени или
                                 // к моменту времени T
      static char* pComment;     // комментарий (краткое описание) задачи
      static TWPS* wps;          // WPS-файл
      static long double tstart; // врем€ начала счета
      static bool survive;       // расчет траекторий выживани€

      // ”становить новый wps-файл.  —тарый закрыть при необходимости
      void setWPS( TWPS* w ) {
         if( wps )
            delete wps;
         wps = w;
      }

      // «агрузить DLL описани€ задачи.
      // ”станавливаютс€ значени€ P, Q, pComment (из DLL)
      // ”станавливаетс€ значение M по умолчанию
      // (квадрат с центром (0,0) и стороной 2)
      void loadProblemDLL(const char *dllpath);

      // ‘ункци€ поп€тного движени€. ¬озвращ€ет точку p0, из которой
      // движение за врем€ h попало бы в текущую точку cur в момент времени t
      // при данных значени€х управлений u и v
      CPoint prev(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h);

      // ‘ункци€ пр€мого движени€. ¬озвращ€ет точку p1, в которую
      // движение за врем€ h попало бы из текущуей точку cur в момент времени t
      // при данных значени€х управлений u и v
      CPoint next(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h);

      // ‘ункци€ решени€ уравнени€ движени€ относительно управлени€ 1-ого
      // игрока. ѕри известной начальной точке, времени, величине шага,
      // управлении 2-ого игрока и результирующей точке находит управление
      // первого игрока. ѕроверок на корректность исходных данных не
      // производитс€!
      CPoint findPvalue(CPoint& cur, CPoint& nxt, CPoint& v,
         long double t, long double h);

      // функци€ отсекающего множества. ¬озвращает отсекающее множество
      // (точнее его сечение) в заданный момент времени
      far TField* sect(long double t);
};

#endif