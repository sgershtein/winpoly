//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright � 1998. All Rights Reserved.
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

// ��� ������ � ������ ������ ��������������� �������� ����������
class TWPS {
   public:
      struct WPSHeader {
         bool isComplete;   // ������ �������?
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
      // ������� ����. ������� ��� ���������. ���������� nSections � secTimes
      void open( const char* fname );
      // ������� ����. ���� ���� ����������, �� �� ����������������.
      void create( const char* fname );
      // ������� ����. ���� ���� �� �������� ���� ���������, �� �� ���������
      void close();
      // �������� ��������� �����
      WPSHeader getHeader() {
         if( !filename )
            Throw( "getHeader: ���� �� ������" );
         else if( nSections < 0 )
            Throw( "getHeader: ����� ���� - ��������� ��� �� �������" );
         return header;
      }
      // �������� ��������� �����. ������ �������� ���������, ��������� ��
      // �������, ���������� ����� ���������. ���� �� (true), �� ��������
      // ������ � ��� DLL �� ����������.  ���� ��� (false) ��� ���� �� ������
      // ������� �� ��������, �� ��������� ���������� ���������, � ��� �����
      // ���������� ������� ���������
      void putHeader( WPSHeader hdr, bool keepSections = true );
      // �������� ���������� ������� � �����
      long getSectCount() {
         if( !filename )
            Throw("getSectCount: ���� �� ������");
         return nSections;
      }
      // �������� ����� i-���� ������� (������ � ����!)
      ldb getSectionTime(unsigned i) {
         if( !filename )
            Throw("getSectionTime: ���� �� ������");

         if( (long)i >= nSections )
            Throw("getSectionTime: section index out of rande");
         return secTimes[i];
      }
      // ������� �� ����� i-��� �������
      TField* getSection( unsigned i );
      // ������� �� ����� �������, ��������� �� ������� � t
      // ��� ���� �������� t ����������, ��������� ������ ������� �������
      // �������, ������� ������������
      TField* getSection( ldb &t );
      // �������� ��������� ������� � ����
      void storeSection( ldb t, TField* fld);

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


   protected:
      char* filename;   // ��� ����� ��� NULL
      long nSections;   // ���������� ������� � �����
                        // (-1 ���� �� ������� ���������)
      WPSHeader header; // ��������� �����
      TArray<ldb> secTimes; // ������� �������
      TArray<ulong> secOffsets; // �������� ������� � �����
      fstream file;     // ����
      long double xmin,ymin,xmax,ymax; // ������������ � ����������� ����������
      bool minmaxfound;				 		// min-� � max-� ���������� ��� ��� ���

      // ���������� � ���� char* pstring, ������������ � ���������� ������
      // ������ ��������� � heap, �� ����� ����� ����� ������� delete.
      char* newPstring( char *ptr );
      // ���������� ������� �� ��������� *ptr ������ *str �
      // ������� pstring
      // � *ptr ������ ���� ���������� ����� ( strlen(str)+1 )
      void setPstring( char *ptr, char *str );
      // ��������� ��� �������� � ���������. �� ���������, ������� ������
      // ���� !mimmaxfound.  �������� force ������� ������������ ���������
      void calcminmax(bool force=false);
};

#endif