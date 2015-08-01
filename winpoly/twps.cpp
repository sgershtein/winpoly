//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright � 1998. All Rights Reserved.
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

// ������� ����. ������� ��� ���������. ���������� nSections � secTimes
void TWPS::open( const char* fname ) {
   if( filename )
      Throw("���� ��� ������");
   file.open( fname,
      ios::in | ios::out | ios::binary | ios::nocreate | ios::noreplace );
   if( !file )
      Throw("�� ���� ������� ����");

   minmaxfound = false;

   // �������� ������ ��� ����� � �����
   filename = new char[300];
   char *junk;
   if( GetFullPathName( fname, 300, filename, &junk ) >= 300 )
      Throw("������� ������� ���� �����");

   // ������������� autoflush
   file.setf(ios::unitbuf);

   // ��������� �������
   try {
      char sigbuf[sizeof(WPS_SIGNATURE)];
      sigbuf[sizeof(WPS_SIGNATURE)-1] = '\0';
      file.read( (char*)&sigbuf, sizeof(WPS_SIGNATURE)-1 );
      if( file.gcount() < sizeof(WPS_SIGNATURE)-1 ||
         strcmp( WPS_SIGNATURE, sigbuf ) != 0 )
         Throw("�������� ������ WPS-�����");

      // ��������� ���������
      ulong hlen; // ����� ���������
      file.read( (char*)&hlen, sizeof(hlen) );
      hlen -=  sizeof(hlen);
      if( file.gcount() < sizeof(hlen) )
         Throw("������ ������ ��������� - ���� �������� ���������");
      char *hbuf = new char[hlen];
      file.read( hbuf, hlen );
      if( file.gcount() < (long)hlen )
         Throw("������ ������ ��������� - ���� �������� ���������");

      // ��������� ���������
      header = *((WPSHeader*)hbuf);
      header.comment = newPstring( (char*)&(((WPSHeader*)hbuf)->comment) );
      header.dllname = newPstring(
         (char*)&(((WPSHeader*)hbuf)->comment) + strlen(header.comment) + 1 );
      delete hbuf;

      // ������ ��������� �� �����, ��������� ������� ������� � �� ��������
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

         // ���������� �������� �������� �������
         secOffsets.push( file.tellg() );
         // ������ ����� � ����� �������
         file.read( &buf.c, sizeof(buf) );
         if( file.gcount() < sizeof(buf) )
            break; // ����� ����� ��� ����

         // ���������, ��� ������� �������� ���������
         char ch;
         file.seekg( buf.s.len-sizeof(buf)-1, ios::cur );
         file.read( &ch, 1 );
         if( file.gcount() < 1 )
            break; // ����� ����� ��� ����

         // ���������� ����� �������
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
      Throw("�� ������� ������ ��� ��������� - �������� ���� ���������");
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
      Throw("���������������� ����������!");
   }
}

// ������� ����. ���� ���� ����������, �� �� ����������������.
void TWPS::create( const char* fname ) {
   if( filename )
      Throw("���� ��� ������");

   // �������� ������ ��� ����� � �����
   filename = new char[300];
   char *junk;
   if( GetFullPathName( fname, 300, filename, &junk ) >= 300 )
      Throw("������� ������� ���� �����");

   minmaxfound = false;

   file.open( fname, ios::in | ios::out | ios::binary );
   if( !file ) {
      delete filename;
      filename = NULL;
      Throw("�� ���� ������� ����");
   }

   nSections = -1;
   secOffsets.setlength(0);
   secTimes.setlength(0);

   // ������������� autoflush
   file.setf(ios::unitbuf);

   // ���������� �������
   file.write( WPS_SIGNATURE, sizeof(WPS_SIGNATURE)-1 );
   file.flush();

}

// ������� ����. ���� ���� �� �������� ���� ���������, �� �� ���������
void TWPS::close() {
   if( !filename )
      return; // ������ close() ������ ������ �� ������
   file.close();
   if( nSections < 0 )  // ��� ���� ��������� - ������� ����
      remove( filename );
   if( header.comment )
      delete header.comment;
   if( header.dllname )
      delete header.dllname;
   header.comment = header.dllname = NULL;
   delete filename;
   filename = NULL;
}

// �������� ��������� �����. ������ �������� ���������, ��������� ��
// �������, ���������� ����� ���������. ���� �� (true), �� ��������
// ������ � ��� DLL �� ����������.  ���� ��� (false) ��� ���� �� ������
// ������� �� ��������, �� ��������� ���������� ���������, � ��� �����
// ���������� ������� ���������
void TWPS::putHeader( WPSHeader hdr, bool keepSections ) {
   if( !filename )
      Throw("���� �� ������");
   if( nSections < 0 )
      keepSections = false;

   // ������ ������� ����� ��������� (��� �����)
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
      // �������� "�����" �����, �������� ���� ���������
      chsize( file.rdbuf()->fd(), file.tellp() );
   }

   if( file.bad() )
      Throw("������ ������");

   file.flush();
}

// ������� �� ����� i-��� �������
TField* TWPS::getSection( unsigned i ) {
   if( !filename || nSections <= 0 )
      Throw("��� �������");
   if( (long)i >= nSections )
      Throw("getSection: index out of range" );

   file.seekg( secOffsets[i] );
   file.clear();
   TField *fld = new TField;
   ulong secLen;
   file.read( (char*)&secLen, sizeof(ulong) );
   if( secOffsets[i]+secLen != secOffsets[i+1] )
      Throw("������ � ��������� �����");
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

// ������� �� ����� �������, ��������� �� ������� � t
// ��� ���� �������� t ����������, ��������� ������ ������� �������
// �������, ������� ������������
TField* TWPS::getSection( ldb &t ) {
   if( !filename || nSections <= 0 )
      Throw("��� �������");
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

// �������� ��������� ������� � ����
void TWPS::storeSection( ldb t, TField* fld) {
   if( !filename )
      Throw("���� �� ������");
   if( nSections < 0 )
      Throw("�� ������� ��������� �����");
   if( nSections > 0 && t >= secTimes[ nSections-1 ] )
      Throw("����� ���������� ������� ������ �����������");
   file.seekp( secOffsets[nSections] );

   minmaxfound = false;

   ulong secLen = (ulong)-1;  // ����� ������ ����� �������
   file.write( (char*)&secLen, sizeof(ulong)); // ���� ����� �� ����������
   file.write( (char*)&t, sizeof(ldb) ); // ����� �������
   ulong nPoly = fld->nPoly();
   file.write( (char*)&nPoly, sizeof(ulong) ); // ���������� ��-���

   for( ulong i=0; i<nPoly; i++ ) {
      // ���������� i-��� �������������
      TWinPoly *wp = fld->getPoly(i);
      ulong nHoles = wp->nHoles();
      file.write( (char*)&nHoles, sizeof(ulong) ); // ���-�� �����
      // ���������� ������� ��������������
      ulong nVertex = wp->nVertex();
      file.write( (char*)&nVertex, sizeof(ulong) ); // ���-�� ������
      for( unsigned j=0; j<nVertex; j++ ) {
         CPoint *v = wp->getVertex(j);
         file.write( (char*)&v->x, sizeof(v->x) );
         file.write( (char*)&v->y, sizeof(v->y) );
      }
      // ���������� ��������������� ��� �����
      for( unsigned k=0; k<nHoles; k++ ) {
         TPolygon *hole = wp->getHole(k);
         ulong nVertex = hole->nVertex();
         file.write( (char*)&nVertex, sizeof(ulong) ); // ���-�� ������
         for( unsigned j=0; j<nVertex; j++ ) {
            CPoint *v = hole->getVertex(j);
            file.write( (char*)&v->x, sizeof(v->x) );
            file.write( (char*)&v->y, sizeof(v->y) );
         }
      }
   }
   secTimes[nSections] = t;
   secOffsets[nSections+1] = file.tellp();

   // ������������ � ���������� ����� ��������������
   secLen = secOffsets[nSections+1] - secOffsets[nSections];
   file.seekp( secOffsets[nSections] );
   file.write( (char*)&secLen, sizeof(ulong)); // ���������� ����� �������

   file.flush();
   nSections++;
}

// ���������� ������� �� ��������� *ptr ������ *str �
// ������� pstring
// � *ptr ������ ���� ���������� ����� ( strlen(str)+1 )
void TWPS::setPstring( char *ptr, char *str ) {
   if( strlen(str) > 255 )
      Throw("setPstring(): ������� ������� ������ (>255)!");
   *ptr++ = (char)strlen(str);
   memcpy( ptr, str, strlen(str) );
}

// ���������� � ���� char* pstring, ������������ � ���������� ������
// ������ ��������� � heap, �� ����� ����� ����� ������� delete.
char* TWPS::newPstring( char* ptr ) {
   char *str = new char[*ptr+1];
   memcpy( str, ptr+1, *ptr );
   str[*ptr] = '\0';
   return str;
}

// ��������� ���������� ������������ ��������������, ����������� ��� ����
void TWPS::calcminmax(bool force) {
	if( !force && minmaxfound )
   	return;
   if( !filename || nSections <= 0 )
      Throw("��� �������");

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


