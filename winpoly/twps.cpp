//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    handling .wps files
//  FILE:         twps.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TWPS class (handling .wps files).
//
//----------------------------------------------------------------------------

#include <string.h>
#include <except.h>
#include <stdio.h>
#include <io.h>
#include "twps.h"

#define WPS_SIGNATURE "SGwps\x1a"

// открыть файл. Считать его заголовок. Установить nSections и secTimes
void TWPS::open( const char* fname ) {
   if( filename )
      Throw("Файл уже открыт");
   file.open( fname,
      ios::in | ios::out | ios::binary | ios::nocreate | ios::noreplace );
   if( !file )
      Throw("Не могу открыть файл");

   minmaxfound = false;

   // запомним полное имя файла с путем
   filename = new char[300];
   char *junk;
   if( GetFullPathName( fname, 300, filename, &junk ) >= 300 )
      Throw("Слишком длинный путь файла");

   // устанавливаем autoflush
   file.setf(ios::unitbuf);

   // проверяем подпись
   try {
      char sigbuf[sizeof(WPS_SIGNATURE)];
      sigbuf[sizeof(WPS_SIGNATURE)-1] = '\0';
      file.read( (char*)&sigbuf, sizeof(WPS_SIGNATURE)-1 );
      if( file.gcount() < sizeof(WPS_SIGNATURE)-1 ||
         strcmp( WPS_SIGNATURE, sigbuf ) != 0 )
         Throw("Неверный формат WPS-файла");

      // считываем заголовок
      ulong hlen; // длина заголовка
      file.read( (char*)&hlen, sizeof(hlen) );
      hlen -=  sizeof(hlen);
      if( file.gcount() < sizeof(hlen) )
         Throw("Ошибка чтения заголовка - файл вероятно поврежден");
      char *hbuf = new char[hlen];
      file.read( hbuf, hlen );
      if( file.gcount() < (long)hlen )
         Throw("Ошибка чтения заголовка - файл вероятно поврежден");

      // разбираем заголовок
      header = *((WPSHeader*)hbuf);
      header.comment = newPstring( (char*)&(((WPSHeader*)hbuf)->comment) );
      header.dllname = newPstring(
         (char*)&(((WPSHeader*)hbuf)->comment) + strlen(header.comment) + 1 );
      delete hbuf;

      // теперь пробегаем по файлу, определяя времена сечений и их смещения
      nSections = -1;
      secTimes.setlength(0);
      secOffsets.setlength(0);
      while( 1 ) {
         nSections++;
         union {
            struct {
               ulong len;
               ldb   t;
            } s;
            char c;
         } buf;

         // запоминаем смещение текущего сечения
         secOffsets.push( file.tellg() );
         // читаем длину и время сечения
         file.read( &buf.c, sizeof(buf) );
         if( file.gcount() < sizeof(buf) )
            break; // конец файла или сбой

         // проверяем, что сечение записано полностью
         char ch;
         file.seekg( buf.s.len-sizeof(buf)-1, ios::cur );
         file.read( &ch, 1 );
         if( file.gcount() < 1 )
            break; // конец файла или сбой

         // запоминаем время сечения
         secTimes.push( buf.s.t );
      };

   } catch( xalloc ) {
      file.close();
      filename = NULL;
      if( header.comment ) {
         delete header.comment;
         header.comment = NULL;
      }
      if( header.dllname ) {
         delete header.dllname;
         header.dllname = NULL;
      }
      Throw("Не хватает памяти для заголовка - вероятно файл поврежден");
   } catch( Exception &e ) {
      file.close();
      filename = NULL;
      if( header.comment ) {
         delete header.comment;
         header.comment = NULL;
      }
      if( header.dllname ) {
         delete header.dllname;
         header.dllname = NULL;
      }
      throw e;
   } catch( ... ) {
      file.close();
      filename = NULL;
      if( header.comment ) {
         delete header.comment;
         header.comment = NULL;
      }
      if( header.dllname ) {
         delete header.dllname;
         header.dllname = NULL;
      }
      Throw("Необрабатываемое исключение!");
   }
}

// создать файл. Если файл существует, то он перезаписывается.
void TWPS::create( const char* fname ) {
   if( filename )
      Throw("Файл уже открыт");

   // запомним полное имя файла с путем
   filename = new char[300];
   char *junk;
   if( GetFullPathName( fname, 300, filename, &junk ) >= 300 )
      Throw("Слишком длинный путь файла");

   minmaxfound = false;

   file.open( fname, ios::in | ios::out | ios::binary );
   if( !file ) {
      delete filename;
      filename = NULL;
      Throw("Не могу создать файл");
   }

   nSections = -1;
   secOffsets.setlength(0);
   secTimes.setlength(0);

   // устанавливаем autoflush
   file.setf(ios::unitbuf);

   // записываем подпись
   file.write( WPS_SIGNATURE, sizeof(WPS_SIGNATURE)-1 );
   file.flush();

}

// закрыть файл. Если файл не содержит даже заголовка, то он удаляется
void TWPS::close() {
   if( !filename )
      return; // лишний close() просто ничего не делает
   file.close();
   if( nSections < 0 )  // нет даже заголовка - удаляем файл
      remove( filename );
   if( header.comment )
      delete header.comment;
   if( header.dllname )
      delete header.dllname;
   header.comment = header.dllname = NULL;
   delete filename;
   filename = NULL;
}

// записать заголовок файла. Второй параметр указывает, сохранять ли
// сечения, записанные после заголовка. Если да (true), то описание
// задачи и имя DLL не изменяются.  Если нет (false) или если ни одного
// сечения не записано, то заголовок изменяется полностью, а все ранее
// записанные сечения пропадают
void TWPS::putHeader( WPSHeader hdr, bool keepSections ) {
   if( !filename )
      Throw("Файл не открыт");
   if( nSections < 0 )
      keepSections = false;

   // размер базовой части заголовка (без строк)
   unsigned hbsize = (char*)&header.comment - (char*)&header;

   file.seekp( sizeof(WPS_SIGNATURE)-1 );
   file.clear();
   if( keepSections ) {
      WPSHeader htmp = hdr;
      htmp.comment = header.comment;
      htmp.dllname = header.dllname;
      header = htmp;
      file.seekp( sizeof(ulong), ios::cur );
      file.write( (char*)&header, hbsize );
   } else {
      if( header.comment )
         delete header.comment;
      if( header.dllname )
         delete header.dllname;
      header = hdr;
      nSections = 0;
      secTimes.setlength(0);
      secOffsets.setlength(0);
      ulong hlen =
         sizeof(ulong) + hbsize +
         1 + strlen(header.comment) +
         1 + strlen(header.dllname);
      char *hbuf = new char[hlen];
      char *hptr;
      memcpy( hptr = hbuf, &hlen, sizeof(hlen) );
      memcpy( hptr = hptr + sizeof(hlen), &header, hbsize );
      setPstring( hptr = hptr + hbsize, header.comment );
      setPstring( hptr = hptr + strlen(header.comment) + 1, header.dllname );
      file.write( (char*)hbuf, hlen );
      secOffsets[0] = file.tellp();
      // отрезаем "хвост" файла, оставляя лишь заголовок
      chsize( file.rdbuf()->fd(), file.tellp() );
   }

   if( file.bad() )
      Throw("Ошибка записи");

   file.flush();
}

// считать из файла i-тое сечение
TField* TWPS::getSection( unsigned i ) {
   if( !filename || nSections <= 0 )
      Throw("Нет сечений");
   if( (long)i >= nSections )
      Throw("getSection: index out of range" );

   file.seekg( secOffsets[i] );
   file.clear();
   TField *fld = new TField;
   ulong secLen;
   file.read( (char*)&secLen, sizeof(ulong) );
   if( secOffsets[i]+secLen != secOffsets[i+1] )
      Throw("Ошибка в структуре файла");
   ldb t;
   file.read( (char*)&t, sizeof(ldb) );
   ulong nPoly;
   file.read( (char*)&nPoly, sizeof(ulong) );

   for( ulong i=0; i<nPoly; i++ ) {
      TWinPoly *wp = new TWinPoly;
      ulong nHoles;
      file.read( (char*)&nHoles, sizeof(ulong) );
      ulong nVertex;
      file.read( (char*)&nVertex, sizeof(ulong) );
      for( unsigned i=0; i<nVertex; i++ ) {
         CPoint v;
         file.read( (char*)&v.x, sizeof(v.x) );
         file.read( (char*)&v.y, sizeof(v.y) );
         *wp += v;
      }
      for( unsigned k=0; k<nHoles; k++ ) {
         TPolygon *hole = new TPolygon;
         ulong nVertex;
         file.read( (char*)&nVertex, sizeof(ulong) );
         for( unsigned i=0; i<nVertex; i++ ) {
            CPoint v;
            file.read( (char*)&v.x, sizeof(v.x) );
            file.read( (char*)&v.y, sizeof(v.y) );
            *hole += v;
         }
         wp->addHole( hole );
      }
      *fld += wp;
   }

   return fld;
}

// считать из файла сечение, ближайшее во времени к t
// при этом значение t изменяется, становясь равным точному времени
// сечения, которое возвращается
TField* TWPS::getSection( ldb &t ) {
   if( !filename || nSections <= 0 )
      Throw("Нет сечений");
   for( unsigned i=0; i<(ulong)nSections; i++ )
      if( secTimes[i] < t ) {
         if( i>0 && fabsl( secTimes[i]-t ) > fabsl( secTimes[i-1]-t ) )
            return ( t = secTimes[i-1], getSection( i-1 ) );
         else
            return ( t = secTimes[i], getSection( i ) );
      }
   t = secTimes[ nSections-1 ];
   return getSection( (unsigned)(nSections-1) );
}

// записать следующее сечение в файл
void TWPS::storeSection( ldb t, TField* fld) {
   if( !filename )
      Throw("Файл не открыт");
   if( nSections < 0 )
      Throw("Не записан заголовок файла");
   if( nSections > 0 && t >= secTimes[ nSections-1 ] )
      Throw("Время очередного сечения больше предыдущего");
   file.seekp( secOffsets[nSections] );

   minmaxfound = false;

   ulong secLen = (ulong)-1;  // длина данных этого сечения
   file.write( (char*)&secLen, sizeof(ulong)); // пока длину не записываем
   file.write( (char*)&t, sizeof(ldb) ); // время сечения
   ulong nPoly = fld->nPoly();
   file.write( (char*)&nPoly, sizeof(ulong) ); // количество мн-ков

   for( ulong i=0; i<nPoly; i++ ) {
      // записываем i-тый многоугольник
      TWinPoly *wp = fld->getPoly(i);
      ulong nHoles = wp->nHoles();
      file.write( (char*)&nHoles, sizeof(ulong) ); // кол-во дырок
      // записываем вершины многоугольника
      ulong nVertex = wp->nVertex();
      file.write( (char*)&nVertex, sizeof(ulong) ); // кол-во вершин
      for( unsigned j=0; j<nVertex; j++ ) {
         CPoint *v = wp->getVertex(j);
         file.write( (char*)&v->x, sizeof(v->x) );
         file.write( (char*)&v->y, sizeof(v->y) );
      }
      // записываем последовательно все дырки
      for( unsigned k=0; k<nHoles; k++ ) {
         TPolygon *hole = wp->getHole(k);
         ulong nVertex = hole->nVertex();
         file.write( (char*)&nVertex, sizeof(ulong) ); // кол-во вершин
         for( unsigned j=0; j<nVertex; j++ ) {
            CPoint *v = hole->getVertex(j);
            file.write( (char*)&v->x, sizeof(v->x) );
            file.write( (char*)&v->y, sizeof(v->y) );
         }
      }
   }
   secTimes[nSections] = t;
   secOffsets[nSections+1] = file.tellp();

   // возвращаемся и записываем длину многоугольника
   secLen = secOffsets[nSections+1] - secOffsets[nSections];
   file.seekp( secOffsets[nSections] );
   file.write( (char*)&secLen, sizeof(ulong)); // записываем длину сечения

   file.flush();
   nSections++;
}

// записываем начиная от указателя *ptr строку *str в
// формате pstring
// в *ptr Должно быть достаточно места ( strlen(str)+1 )
void TWPS::setPstring( char *ptr, char *str ) {
   if( strlen(str) > 255 )
      Throw("setPstring(): Слишком длинная строка (>255)!");
   *ptr++ = (char)strlen(str);
   memcpy( ptr, str, strlen(str) );
}

// возвращает в виде char* pstring, начинающуюся с указанного адреса
// строка создается в heap, ей потом нужно будет сделать delete.
char* TWPS::newPstring( char* ptr ) {
   char *str = new char[*ptr+1];
   memcpy( str, ptr+1, *ptr );
   str[*ptr] = '\0';
   return str;
}

// посчитать координаты минимального прямоугольника, содержащего все поля
void TWPS::calcminmax(bool force) {
	if( !force && minmaxfound )
   	return;
   if( !filename || nSections <= 0 )
      Throw("Нет сечений");

   TField *fld = getSection(0);
   xmin = fld->minx();
   xmax = fld->maxx();
   ymin = fld->miny();
   ymax = fld->maxy();
   for( unsigned i = 1; (long)i < nSections; i++ ) {
      delete fld;
      fld = getSection(i);
   	if( fld->maxx() > xmax ) xmax = fld->maxx();
   	if( fld->maxy() > ymax ) ymax = fld->maxy();
   	if( fld->minx() < xmin ) xmin = fld->minx();
   	if( fld->miny() < ymin ) ymin = fld->miny();
   }
   delete fld;
   minmaxfound = true;
}


