//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         brview.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TBrView (TDialog).
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include "brview.h"
#include "brdata.h"
#include "polywin.h"

//{{TBrView Implementation}}

static TBrViewXfer TBrViewData;

TBrView::TBrView(TField* &fld,
   TWindow* parent, TResId resId, TModule* module) :
   field( fld ), TDialog(parent, resId, module) {
//{{TBrViewXFER_USE}}
   ValueComment = new TStatic(this, IDC_V_COMMENT, 255);
   ValueNSect = new TStatic(this, IDC_V_NSECT, 255);
   ValueSNo = new TStatic(this, IDC_V_SNO, 255);
   ValueStep = new TStatic(this, IDC_V_STEP, 255);
   ValueSTime = new TEdit(this, IDC_V_STIME, 255);
   ValueTime = new TStatic(this, IDC_V_TIME, 255);
   ScrollTime = new TScrollBar(this, IDC_SCR_TIME);
   ValueNHoles = new TStatic(this, IDC_V_NHOLES, 255);
   ValueNPoly = new TStatic(this, IDC_V_NPOLY, 255);

   SetTransferBuffer(&TBrViewData);
//{{TBrViewXFER_USE_END}}

}

TBrView::~TBrView() {
   Destroy(IDCANCEL);
}


bool TBrView::Create() {
   // ������-�� ������ ������������ ��� ������ � wps-������
   if( !TBrData::wps )
      return false;

   // ������� ����� �������
   if( !TDialog::Create() )
      return false;

   // ������� ��� ������
   TWPS::WPSHeader header = TBrData::wps->getHeader();
   // ����������� � ������
   ValueComment->SetText(header.comment);
   // �������� �� �������
   char buf[100];
   long double tstart =
      TBrData::wps->getSectionTime( TBrData::wps->getSectCount()-1 );
   sprintf( buf, "[ %Lg ; %Lg ]", (ldb)(EQU(tstart,0)? 0:tstart), header.T );
   ValueTime->SetText(buf);
   // ���������� �������
   sprintf( buf, "%ld", (long)TBrData::wps->getSectCount() );
   ValueNSect->SetText(buf);
   // ��� �� �������
   sprintf( buf, "%Lg", header.delta );
   ValueStep->SetText(buf);
   // ������������� ��������� ������ ����������
   ScrollTime->SetRange(1,(int)TBrData::wps->getSectCount());
   ScrollTime->SetPosition((int)TBrData::wps->getSectCount());
   // ��������� ��������� (����� ������ �� �������)������� �� ����� �����
   displaySection( (unsigned)TBrData::wps->getSectCount()-1 );

   // ��������!  ���� �� ��������� �������� ���� � ���� ����� �������
   ValueSTime->SetReadOnly(true);

   return true;
}

// ������� �� ��������� - ���� �����������...
void TBrView::EvScroll(uint /*code*/) {
   displaySection( ScrollTime->GetPosition()-1 );
}

// ���������� n-��� �������
void TBrView::displaySection(unsigned n) {
   char buf[100];
   // ������� ����� �������
   sprintf( buf, "%u", n );
   ValueSNo->SetText(buf);
   // ������� ����� �������
   ldb t = TBrData::wps->getSectionTime( n );

   /************** ��������� ����� *************
      char bbb[50];
      sprintf(bbb,"T=%Lg",(ldb)t);
      putenv(bbb);
   /************** ����� *************/

   sprintf(buf,"%Lg",(ldb)(EQU(t,0)?0:t));
   ValueSTime->SetText(buf);
   // ���������� ���� �������
   TField *fprev = field;
   field = TBrData::wps->getSection(n);
   delete fprev;
   // �������������� ����
	TYPESAFE_DOWNCAST(GetParentO(),TPolyWindow)->redrawField();
   // ������� ���������� ���������������
   sprintf(buf,"%ld",(long)(field->nPoly()));
   ValueNPoly->SetText(buf);
   // ������� � ������� ���������� �����
   long nh = 0;
   for( long n=0; n<(long)field->nPoly(); n++ ) {
      nh += field->getPoly(n)->nHoles();
   }
   sprintf(buf,"%ld",nh);
   ValueNHoles->SetText(buf);
}

