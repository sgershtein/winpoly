//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         stepdlg.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TStepDlg (TDialog).
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <stdio.h>   // sprintf

#include "stepdlg.h"
#include "polywin.h"
#include "brdata.h"


//
// Build a response table for all messages/commands handled by the application.
//
DEFINE_RESPONSE_TABLE1(TStepDlg, TDialog)
//{{TStepDlgRSP_TBL_BEGIN}}
  EV_BN_CLICKED(IDC_START, BNStart),
  EV_WM_CLOSE,
  EV_BN_CLICKED(IDC_STOP, BNCancel),
//{{TStepDlgRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TStepDlg Implementation}}

static TStepDlgXfer TStepDlgData;

TStepDlg::TStepDlg(TField* &fld, TDialog* dview,
   TWindow* parent, TResId resId, TModule* module) :
   field( fld ), Dview( dview ), thread( NULL ),
   TDialog(parent, resId, module) {
//{{TStepDlgXFER_USE}}
  valueT = new TStatic(this, IDC_VALUE_T, 255);
  valueDelta = new TStatic(this, IDC_VALUE_DELTA, 255);
  valueTime = new TStatic(this, IDC_VALUE_TIME, 255);
  valueVC = new TStatic(this, IDC_VALUE_VC, 255);
  prBar = new TGauge(this, IDC_PROGRESSBAR);
  valuePcnt = new TStatic(this, IDC_VALUE_PERCENT, 255);
  bStart = new TButton(this, IDC_START);
  bCancel = new TButton(this, IDC_STOP);
  cbPause = new TCheckBox(this, IDC_CB_PAUSE, 0);
  valuePC = new TStatic(this, IDC_VALUE_PC, 255);

  SetTransferBuffer(&TStepDlgData);
//{{TStepDlgXFER_USE_END}}

}

TStepDlg::~TStepDlg() {
   Destroy(IDC_STOP);
}

void TStepDlg::SetInfo() {
   // ��������� �������� �������� �������, ��������, ���������� ������,
   // �������������� ����
   char buf[20];
   if( EQU(t, 0) )
   	sprintf(buf,"0");
   else
   	sprintf(buf,"%Lg",t);
   valueTime->SetText(buf);
   if( vc )
      sprintf(buf,"%ld",vc);
   else
      sprintf(buf,"����������");
   valueVC->SetText(buf);
   if( field )
      sprintf(buf,"%ld",(long)field->nPoly());
   else
      sprintf(buf,"-");
   valuePC->SetText(buf);
   sprintf(buf,"%d%%",int((brd.T-t)*100/brd.T)); valuePcnt->SetText(buf);
   prBar->SetValue(int((brd.T-t)/brd.delta));
   // ���������� �������� (���� ��������� ����������� ������� ��� �����������)
  	TYPESAFE_DOWNCAST(GetParentO(),TPolyWindow)->redrawField();
}

void TStepDlg::EvClose() {
   Stop(); // ������������� ����������, ���� ��� ��� ����
	if( !Dview->Create() )
      MessageBox("�� ������� ���������������� "
         "���� ������� ��������� �����",
         "������!",MB_OK | MB_ICONSTOP);
   TDialog::EvClose();
}

void TStepDlg::Destroy(int retVal) {
   if( thread ) {
      delete thread;
      thread = NULL;
   }

   TDialog::Destroy(retVal);
}

bool TStepDlg::Create() {
   if( !TDialog::Create() ) // ������� ���� �������
      return false;

   if( !thread )
      thread = new TStepThread( this );   // ������� ���� ����������

   t = brd.tstart;      // �����
   vc = 0;              // ��������� ���������� ������ �� ����

   if( !brd.wps || brd.wps->getSectCount() <= 0 ) {
      // ����� �������
      if( field )
         delete field;
      field = new TField(*brd.M);   // �������� � �������� ���������
   }

   char buf[20];  // ������� �������� ���������� ����������
   sprintf(buf,"%Lg",brd.T);  valueT->SetText(buf);
   sprintf(buf,"%Lg",brd.delta); valueDelta->SetText(buf);
   prBar->SetRange(0,int(brd.T/brd.delta));
   prBar->SetStep(1);
   SetInfo();   // ������� ����� � ��������� �������� (���������� ���������)

   return true; // ������ ������� ������
}

void TStepDlg::BNStart() {
   if( thread )
      thread->BNStart();
}

void TStepDlg::BNCancel() {
   Stop();
   try {
   	if( !Dview->Create() )
         MessageBox("�� ������� ���������������� "
            "���� ������� ��������� �����",
            "������!",MB_OK | MB_ICONSTOP);
   } catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
		MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
  	}
   Destroy(IDCANCEL);
}

bool TStepDlg::doStep( bool volatile *ShouldTerminate ) {

   try {
      // ��������� ������� � ����
      // ���������� ��������� ��������� ��������� ������
      // ���� ��� ������� ���������. � ������ �����������
      // �������� ��� �� ����� � ����� ��� ���������� �������
      // (� ���������� ��������) ������
      if( TBrData::wps ) {
         if( save2file )
            TBrData::wps->storeSection( t, field );
         else
            save2file = true;
      }

      // ��������� ��� �������� ���������
      TField *fold = field;
      TField *res = field->Wprev(t, ShouldTerminate);
      if( res ) {
         field = res;   // �������� ������� ����
         delete fold;
      } else
         return false; // ��������������� ����������
      t -= brd.delta;

      if( ShouldTerminate && *ShouldTerminate )
         return false;

      // ���������� �������
      vc = field->sift(ShouldTerminate);

  	} catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
		MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      bStart->SetWindowText("������");
      bStart->EnableWindow(false);
      bCancel->SetWindowText("������");
      return false;
  	}

   if( t <= 0 || field->nPoly() == 0 ) {  // ���������� ���������
      bStart->SetWindowText("������");
      bStart->EnableWindow(false);
      bCancel->SetWindowText("������");
      if( TBrData::wps ) {
         // ���������� ��������� �������
         TBrData::wps->storeSection( t, field );
         // �������� � ��������� wps-�����, ��� ���������� ���������
         TWPS::WPSHeader hdr = TBrData::wps->getHeader();
         hdr.isComplete = true;
         TBrData::wps->putHeader(hdr);
      }
      return false;
   }

   return true;
}

bool TStepDlg::IdleAction(long idleCount) {
   static long double tprev = 0;
   static unsigned short int ctt = 0;
   if( !thread )
      return TDialog::IdleAction(idleCount);
   if( tprev != t ) {
      SetInfo();
      tprev =t;
      // ���������������� ���������� ����� ������� ���� ���� �������
      if( doPause() )
         Suspend();
   }
   if( ( ++ctt & 127 ) == 0 )
      TDialog::IdleAction(idleCount);
   return true;
}

//----------------------------------------------------------------------------

int TStepThread::Run() {   // ���������� ������� �����
   working = true;
   sts = Running;

   // ����������, ����� �� ��������� ������ ������� � ����
   // ��� ����� ������, ���� �� ������ �������� �������, � �� �����,
   // ���� �� �� ����������.
   sd->save2file = TBrData::wps && ( TBrData::wps->getSectCount() == 0 );

   while( sd->doStep( &terminating ) ) {
      // ������ �� ������
   }
   working = false;
   sts = Finished;
   return 0;
}

void TStepThread::Stop() {     // ���������� ���������� ���� ��� ����
   if ( GetStatus() == Suspended )
      sd->Resume();
   terminating = true;
   while( working )
      Sleep(200); // ���� ����������
}

void TStepThread::BNStart() {  // ���������� ������� �� ������ "�����" �������
   switch( GetStatus() ) {
      case Running:
         sd->Suspend();
         break;
      case Suspended:
         sd->Resume();
         break;
      case Created:
         sd->Start();
         break;
   }
}


