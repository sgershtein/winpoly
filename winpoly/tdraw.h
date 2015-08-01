//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Window drawing
//  FILE:         tdraw.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for thread class that draws the field
//
//----------------------------------------------------------------------------

#ifndef _TDRAW_H_
#define _TDRAW_H_

#include <classlib/thread.h>
#include <owl/dc.h>
#include <owl/window.h>

class TField;

class TDrawThread : public TThread {
	public:
      enum TDrawType {
         unspecified = 0,
         cleanField  = 1 << 0,   // очистить поле перед рисованием
         doFills     = 1 << 1,   // заливать многоугольники
         drawGrid    = 1 << 2,   // рисовать сетку
         useColors   = 1 << 3,   // рисовать в цвете
         drawBorder  = 1 << 4,   // рисовать границы многоугольников

         usualField  = cleanField + doFills + drawGrid,
         colorField  = usualField + useColors,
         traceField  = drawBorder
      };
		TDrawThread( TDC *dc, TRect region, TField *field,
      	TWindow *invalidate = NULL, TDrawType drType = unspecified,
         long double xmi=0, long double ymi=0, long double xma=0,
         long double yma=0) : xmin(xmi), ymin(ymi), xmax(xma), ymax(yma),
      	Field(field), DC(dc), Client(region), Invalidate(invalidate),
         drawtype( drType == unspecified? colorField : drType) {};
   private:
   	TField *Field;    // рабочее поле
      TDC *DC;				// DC, на котором рисуем
      TRect Client;		// рабочий регион в DC
      TWindow *Invalidate;	// окошко, для которого вызвать Invalidate()
      						// после завершения рисования
      TDrawType drawtype;  // "тип" картинки
      long double xmin,ymin,xmax,ymax; // область изображения для рисования

      int Run();
};

#endif
