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
//  Initial data for stable bridge
// (goal set M, Q and P sets, differential functions...)
//
//----------------------------------------------------------------------------

#include "brdata.h"
#include "texcept.h"
#include "dllfunc.h"

TField *TBrData::M = NULL;          // ������� ��������� ������� ������
TArray< CPoint > TBrData::P;        // ����� ����� ����� ������� ��������� P
                                    // �������� u (���������� 1-��� ������)
TArray< CPoint > TBrData::Q;        // ����� ����� ����� ������� ��������� Q
                                    // �������� v (���������� 2-��� ������)
char* TBrData::pComment;            // ������� �������� ������ (�����������)
TWPS* TBrData::wps = NULL;          // ��������� � ������� wps-����

long double TBrData::delta = 0.05;  // ��� ��������� �� �������
long double TBrData::T     = 2;     // ������� ����� (T)
long double TBrData::sieve = 0.015; // "����" ��� ������
bool TBrData::fixedT       = true;  // ����. ������ / � ������� �������
long double TBrData::tstart = TBrData::T; // ����� ������ �����
bool TBrData::survive      = true;  // ������ ���������� ���������

// ���������� �� ����� � DLL
HINSTANCE hLib = 0;  // ������������� ����������� DLL
Fcalcprev *calcprev = NULL; // ������ �� ������� ���������� ��������� ��������
FSecField *secfield = NULL; // ������ �� ������� ����������� ���������
FfindPvalue *findpvalue = NULL;  // ������ �� ������� ������� ���������
                                 // �������� ������������ ���������� 1 ������

// ���� ����� ��������� ��� ���������� ������ FreeLibrary ��� ����������
// ���������. ����� ������������ �� ����������� ������� ������. ��� �����
// �� �������� ������ ����� �����������.
class ThLibCleaner {
   public:
      ~ThLibCleaner() {
         if( hLib )
            FreeLibrary( hLib );
      }
} hLibCleaner;

// ��������� DLL � �������, ��������� ������.
// � ������ ������ ����������� ����������� ���������� � ������� ������
void TBrData::loadProblemDLL(const char *dllpath) {
   // ����������� ������� DLL, ���� ��� ��� ���� ���������
   if( hLib )
      FreeLibrary( hLib );

   // ���������� DLL
   hLib = LoadLibrary(dllpath);
   if( (unsigned)hLib <= HINSTANCE_ERROR ) {
      calcprev = NULL;
      pComment = NULL;
      findpvalue = NULL;
      hLib = 0;
      Throw("������ �������� DLL �������� ������");
   }

   // ������� ��������������� �������
   calcprev = (Fcalcprev*)GetProcAddress(hLib,calcprevName);
   if( !calcprev )
      Throw("� DLL �������� ������ �� ������� ������� ��������� ��������");

   findpvalue = (FfindPvalue*)GetProcAddress(hLib,findPvalueName);

   secfield = (FSecField*)GetProcAddress(hLib,SecFieldName);
   survive = secfield != NULL;

   Fcomment *comment = (Fcomment*)GetProcAddress(hLib,commentName);
   if( comment )
      pComment = (*comment)();
   else
      pComment = "[�������� ������ �����������]";

   FPQset *Pset = (FPQset*)GetProcAddress(hLib,PsetName);
   FPQset *Qset = (FPQset*)GetProcAddress(hLib,QsetName);
   if( !Pset || !Qset )
      Throw("� DLL �������� ������ �� ������� �������� Q �/��� P");

   // ����������� � DLL �������� Q � P � ������������� ��
   long double *pld; unsigned cnt;

   P.setlength(0);
   (*Pset)(cnt,pld);
   for( unsigned i=0; i<cnt*2; i+=2 )
      P.push( CPoint(pld[i], pld[i+1]) );

   Q.setlength(0);
   (*Qset)(cnt,pld);
   for( unsigned i=0; i<cnt*2; i+=2 )
      Q.push( CPoint(pld[i], pld[i+1]) );

   // �������� M �� ��������� - ������� �� �������� 2
   // � ������� � ������ ���������
   if( M ) delete M;
   FMField *MField = (FMField*)GetProcAddress(hLib,MFieldName);
   if( MField ) {
      TField *far MF = (*MField)();
      M = new TField(*MF);
   } else
      M = NULL;

   // �������� ���������� ������ �� ���������
   // ��������� �� ������, ���� ��� �� ����������� ���������� ��������
   if( !wps || wps->getSectCount() <= 0 ) {
      Fparams *params = (Fparams*)GetProcAddress(hLib,paramsName);
      if( params )
         (*params)( T, fixedT, delta, sieve );
   }

}

CPoint TBrData::findPvalue(CPoint& cur, CPoint& nxt, CPoint& v,
   long double t, long double h) {
   CPoint u;
   if( findpvalue )
      (*findpvalue)(nxt.x,nxt.y,cur.x,cur.y,v.x,v.y,u.x,u.y,t,h);
   else
      Throw("� DLL ������ �� ���������� ������� ���������� "
         "���������� �� ��������");

   return u;
}

// ������� ��������� ��������
CPoint TBrData::prev(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h) {
   CPoint res;

   if( calcprev )
      (*calcprev)(res.x,res.y,cur.x,cur.y,v.x,v.y,u.x,u.y,t,h);
   else
      Throw("�� ��������� DLL �������� ������");

   return res;
}

// ������� ������� ��������
CPoint TBrData::next(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h) {
   return prev( cur, u, v, t, -h );
}

// ������� ����������� ���������. ���������� ���������� ���������
// (������ ��� �������) � �������� ������ �������
TField* TBrData::sect(long double t) {
   if( !secfield )
      Throw("�� ���������� ������� ����������� ���������");

   return (*secfield)(t);
}


