//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
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
      static TField *M;          // ������� ��������� 1-��� ������
      static TArray< CPoint > P; // ����� ����� ����� ������� ��������� P
                                 // �������� u (���������� 1-��� ������)
      static TArray< CPoint > Q; // ����� ����� ����� ������� ��������� Q
                                 // �������� v (���������� 2-��� ������)
      static long double delta;  // ��� ��������� �� �������
      static long double T;      // ������� ����� (T)
      static long double sieve;  // "����". ������� �������������� �����
                                 // ���������, ���� ��� ���� ��� �������
                                 // ��������� �� �����, ��� �� �������� sieve
      static bool fixedT;        // � ������������� ������ ������� ���
                                 // � ������� ������� T
      static char* pComment;     // ����������� (������� ��������) ������
      static TWPS* wps;          // WPS-����
      static long double tstart; // ����� ������ �����
      static bool survive;       // ������ ���������� ���������

      // ���������� ����� wps-����.  ������ ������� ��� �������������
      void setWPS( TWPS* w ) {
         if( wps )
            delete wps;
         wps = w;
      }

      // ��������� DLL �������� ������.
      // ��������������� �������� P, Q, pComment (�� DLL)
      // ��������������� �������� M �� ���������
      // (������� � ������� (0,0) � �������� 2)
      void loadProblemDLL(const char *dllpath);

      // ������� ��������� ��������. ���������� ����� p0, �� �������
      // �������� �� ����� h ������ �� � ������� ����� cur � ������ ������� t
      // ��� ������ ��������� ���������� u � v
      CPoint prev(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h);

      // ������� ������� ��������. ���������� ����� p1, � �������
      // �������� �� ����� h ������ �� �� �������� ����� cur � ������ ������� t
      // ��� ������ ��������� ���������� u � v
      CPoint next(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h);

      // ������� ������� ��������� �������� ������������ ���������� 1-���
      // ������. ��� ��������� ��������� �����, �������, �������� ����,
      // ���������� 2-��� ������ � �������������� ����� ������� ����������
      // ������� ������. �������� �� ������������ �������� ������ ��
      // ������������!
      CPoint findPvalue(CPoint& cur, CPoint& nxt, CPoint& v,
         long double t, long double h);

      // ������� ����������� ���������. ���������� ���������� ���������
      // (������ ��� �������) � �������� ������ �������
      far TField* sect(long double t);
};

#endif