//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    handling .wps files
//  FILE:         twps.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Header file for TWPS class (handling .wps files).
//
//----------------------------------------------------------------------------

#ifndef _TWPS_H_
#define _TWPS_H_

#include <fstream.h>
#include "tarray.h"
#include "tfield.h"
#include "texcept.h"

typedef long double ldb;
typedef unsigned long ulong;

// Все ошибки в данном классе сигнализируются выбросом исключений
class TWPS {
   public:
      struct WPSHeader {
         bool isComplete;   // полное решение?
         ldb T;
         ldb delta;
         ldb sieve;
         bool fixedT;
         char* comment;
         char* dllname;
      };

      TWPS() : filename(NULL), minmaxfound(false) {
         header.comment = header.dllname = NULL;
      };
      ~TWPS() {
         close();
         if( header.comment )
            delete header.comment;
         if( header.dllname )
            delete header.dllname;
      }
      // открыть файл. Считать его заголовок. Установить nSections и secTimes
      void open( const char* fname );
      // создать файл. Если файл существует, то он перезаписывается.
      void create( const char* fname );
      // закрыть файл. Если файл не содержит даже заголовка, то он удаляется
      void close();
      // получить заголовок файла
      WPSHeader getHeader() {
         if( !filename )
            Throw( "getHeader: Файл не открыт" );
         else if( nSections < 0 )
            Throw( "getHeader: Новый файл - заголовок еще не записан" );
         return header;
      }
      // записать заголовок файла. Второй параметр указывает, сохранять ли
      // сечения, записанные после заголовка. Если да (true), то описание
      // задачи и имя DLL не изменяются.  Если нет (false) или если ни одного
      // сечения не записано, то заголовок изменяется полностью, а все ранее
      // записанные сечения пропадают
      void putHeader( WPSHeader hdr, bool keepSections = true );
      // Получить количество сечений в файле
      long getSectCount() {
         if( !filename )
            Throw("getSectCount: Файл не открыт");
         return nSections;
      }
      // получить время i-того сечения (отсчет с нуля!)
      ldb getSectionTime(unsigned i) {
         if( !filename )
            Throw("getSectionTime: файл не открыт");

         if( (long)i >= nSections )
            Throw("getSectionTime: section index out of rande");
         return secTimes[i];
      }
      // считать из файла i-тое сечение
      TField* getSection( unsigned i );
      // считать из файла сечение, ближайшее во времени к t
      // при этом значение t изменяется, становясь равным точному времени
      // сечения, которое возвращается
      TField* getSection( ldb &t );
      // записать следующее сечение в файл
      void storeSection( ldb t, TField* fld);

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


   protected:
      char* filename;   // имя файла или NULL
      long nSections;   // количество сечений в файле
                        // (-1 если не записан заголовок)
      WPSHeader header; // заголовок файла
      TArray<ldb> secTimes; // времена сечений
      TArray<ulong> secOffsets; // смещения сечений в файле
      fstream file;     // файл
      long double xmin,ymin,xmax,ymax; // максимальные и минимальные координаты
      bool minmaxfound;				 		// min-ы и max-ы подсчитаны или еще нет

      // возвращает в виде char* pstring, начинающуюся с указанного адреса
      // строка создается в heap, ей потом нужно будет сделать delete.
      char* newPstring( char *ptr );
      // записываем начиная от указателя *ptr строку *str в
      // формате pstring
      // в *ptr Должно быть достаточно места ( strlen(str)+1 )
      void setPstring( char *ptr, char *str );
      // посчитать все минимумы и максимумы. По умолчанию, считать только
      // если !mimmaxfound.  Параметр force требует безусловного пересчета
      void calcminmax(bool force=false);
};

#endif