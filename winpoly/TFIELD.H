//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         tfield.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TField class - the field where all polygons lay
//
//----------------------------------------------------------------------------

#ifndef _TFIELD_H_
#define _TFIELD_H_

#include "tpoly.h"
#include "tdraw.h"

class TField {
	private:
		// переменные
      long double xmin,ymin,xmax,ymax; // максимальные и минимальные координаты
      bool minmaxfound;				 		// min-ы и max-ы подсчитаны или еще нет
      TArray<TWinPoly*> poly;				// массив указателей на многоугольники
      TMemoryDC *mDC;  				      // DC для рисования поля
      TDC *clientDC;							// исходный клиентский DC
      TBitmap *mBitmap;						// Bitmap для рисования поля
      TDrawThread *drawThread;			// Thread, рисующий картинку

      // посчитать все минимумы и максимумы. По умолчанию, считать только
      // если !mimmaxfound.  Параметр force требует безусловного пересчета
      void calcminmax(bool force=false);

	public:
   	TField() : minmaxfound(false), mDC(NULL), mBitmap(NULL),
      	drawThread(NULL), clientDC(NULL) {};
      // сконструировать из списка многоугольников
      // список не разрушается
      TField(TList<TWinPoly*> pl);
      // copy constructor
      TField(TField &other);
/*      TField& operator=(TField other); */
      ~TField(); // все многоугольники уничтожаются

      // количество многоугольников
      unsigned nPoly() { return poly.length(); }

      // i-тый многоугольник
      TWinPoly* getPoly(unsigned i) {
      	return (i<poly.length())? poly[i]:NULL;
      };

      // добавить многоугольник на поле
      void operator+=(TWinPoly *p) { poly[poly.length()] = p; }

      // удалить i-тый многоугольник с поля (многоугольник уничтожается)
      void removePoly(unsigned i) {
      	if( i < poly.length() && poly[i] )
         	delete poly[i];
      	poly.remove(i);
      }

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

		// записать в файл / прочитать из файла
      // в случае ошибки выбрасываются исключения
      void fileSave(const char *fname);
      void fileOpen(const char *fname);

      // Пересечь два поля.  Возвращает список многоугольников,
      // образующих пересечение
      TList<TWinPoly*> intersect(TField *f,
         bool volatile *ShouldTerminate = NULL);
      // Объединить два поля.  Возвращает список многоугольников,
      // образующих объединение
      TList<TWinPoly*> unite(TField *f,
         bool volatile *ShouldTerminate = NULL);

      // найти предыдущее (t-delta) сечение моста, если *this - это текущее
      // (попятная процедура)
      // Параметры задаются классом TBrData
      // В качестве аргумента указывается текущий момент времени
      TField* Wprev(long double t, bool volatile *ShouldTerminate = NULL);

      // "просеять" текущее поле, выкинув из него "лишние" вершины
      // функция возвращает количество оставшихся на поле вершин
      unsigned long sift(bool volatile *ShouldTerminate = NULL);

      // проверить многоугольники на наличие "самопересечений"
      // и избавиться от них
      void checkselfcross(bool volatile *ShouldTerminate = NULL);

      // проверить многоугольники на наличие "самопересечений"
      // вернуть истину, если таковые найдены
      bool selfcrosscontrol(bool volatile *ShouldTerminate = NULL);

      // найти на поле точку, ближайщую к заданной
      CPoint findClosest(CPoint &p);

      // очистить поле
      void clearAll();

      // ---- функции, связанные с рисованием на некотором DC -------

      // начать рисовать картинку указанного размера для данного DC
      // (реально строится копия DC в памяти и рисуется на ней)
      // Когда картинка готова, вызывается invalidate->Invalidate() если
      // invalidate определено
      // Могут быть указаны координаты прямоугольника, который следует
      // масштабировать на картинку.  Если не указаны, то масштыб выбирается
      // автоматически так, чтобы вошла вся картинка.
      void preparePicture(TDC &DC, TRect region, TWindow *invalidate = NULL,
         TDrawThread::TDrawType drtype = TDrawThread::unspecified,
         long double xmin = 0,
         long double ymin = 0,
         long double xmax = 0,
         long double ymax = 0);
      void preparePicture(TRect region, TWindow *invalidate = NULL,
         TDrawThread::TDrawType drtype = TDrawThread::unspecified,
         long double xmin = 0,
         long double ymin = 0,
         long double xmax = 0,
         long double ymax = 0);

      // Начать рисовать картинку с указанными параметрами НЕПОСРЕДСТВЕННО
      // на данном DC. ВНИМАНИЕ! В этом случае функции getPictureDC() и
      // getPictureBitmap() не работают, т.к. внутри объекта TField
      // рисунок не сохраняется.
      // Могут быть указаны координаты прямоугольника, который следует
      // масштабировать на картинку.  Если не указаны, то масштыб выбирается
      // автоматически так, чтобы вошла вся картинка.
      void preparePictureOnDC(TDC &DC, TRect region,
         TWindow *invalidate = NULL,
         TDrawThread::TDrawType drtype = TDrawThread::unspecified,
         long double xmin = 0,
         long double ymin = 0,
         long double xmax = 0,
         long double ymax = 0);

      // прекратить рисование картинки
      void stopPreparingPicture() {
      	if( drawThread ) {
         	if( drawThread->GetStatus() == TThread::Running )
      	   	drawThread->TerminateAndWait();
            delete drawThread;
            drawThread = NULL;
         }
      }

      // вернуть уже нарисованную картинку (memory DC) или NULL
      TDC* getPictureDC();

      // вернуть нарисованную картинку в виде bitmap или NULL
      TBitmap* getPictureBitmap();

      // готова ли картинка?
      bool PictureReady();

      // ждем пока картинка будет полностью нарисована
      void PictureWaitReady();

};

#endif
