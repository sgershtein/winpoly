//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright © 1998. All Rights Reserved.
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
   // установим значение текущего времени, ползунок, количество вершин,
   // перерисовываем поле
   char buf[20];
   if( EQU(t, 0) )
   	sprintf(buf,"0");
   else
   	sprintf(buf,"%Lg",t);
   valueTime->SetText(buf);
   if( vc )
      sprintf(buf,"%ld",vc);
   else
      sprintf(buf,"неизвестно");
   valueVC->SetText(buf);
   if( field )
      sprintf(buf,"%ld",(long)field->nPoly());
   else
      sprintf(buf,"-");
   valuePC->SetText(buf);
   sprintf(buf,"%d%%",int((brd.T-t)*100/brd.T)); valuePcnt->SetText(buf);
   prBar->SetValue(int((brd.T-t)/brd.delta));
   // перерисуем картинку (если рисование предыдущего сечения уже завершилось)
  	TYPESAFE_DOWNCAST(GetParentO(),TPolyWindow)->redrawField();
}

void TStepDlg::EvClose() {
   Stop(); // останавливаем вычисления, если они еще идут
	if( !Dview->Create() )
      MessageBox("Не удалось инициализировать "
         "окно диалога просмотра моста",
         "Ошибка!",MB_OK | MB_ICONSTOP);
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
   if( !TDialog::Create() ) // создаем окно диалога
      return false;

   if( !thread )
      thread = new TStepThread( this );   // создаем нить вычислений

   t = brd.tstart;      // время
   vc = 0;              // суммарное количество вершин на поле

   if( !brd.wps || brd.wps->getSectCount() <= 0 ) {
      // новые расчеты
      if( field )
         delete field;
      field = new TField(*brd.M);   // начинаем с целевого множества
   }

   char buf[20];  // выведем значения постоянных параметров
   sprintf(buf,"%Lg",brd.T);  valueT->SetText(buf);
   sprintf(buf,"%Lg",brd.delta); valueDelta->SetText(buf);
   prBar->SetRange(0,int(brd.T/brd.delta));
   prBar->SetStep(1);
   SetInfo();   // выведем также и остальные значение (переменные параметры)

   return true; // диалог успешно создан
}

void TStepDlg::BNStart() {
   if( thread )
      thread->BNStart();
}

void TStepDlg::BNCancel() {
   Stop();
   try {
   	if( !Dview->Create() )
         MessageBox("Не удалось инициализировать "
            "окно диалога просмотра моста",
            "Ошибка!",MB_OK | MB_ICONSTOP);
   } catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"Программное исключение (%s)",e.where());
		MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
  	}
   Destroy(IDCANCEL);
}

bool TStepDlg::doStep( bool volatile *ShouldTerminate ) {

   try {
      // сохраняем сечение в файл
      // необходимо сохранять начальное состояние ТОЛЬКО
      // если это целевое множество. В случае продолжения
      // расчетов нам не нужны в файле два одинаковых сечения
      // (с одинаковым временем) подряд
      if( TBrData::wps ) {
         if( save2file )
            TBrData::wps->storeSection( t, field );
         else
            save2file = true;
      }

      // совершаем шаг попятной процедуры
      TField *fold = field;
      TField *res = field->Wprev(t, ShouldTerminate);
      if( res ) {
         field = res;   // заменяем текущее поле
         delete fold;
      } else
         return false; // преждевременное завершение
      t -= brd.delta;

      if( ShouldTerminate && *ShouldTerminate )
         return false;

      // просеиваем сечение
      vc = field->sift(ShouldTerminate);

  	} catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"Программное исключение (%s)",e.where());
		MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      bStart->SetWindowText("Расчет");
      bStart->EnableWindow(false);
      bCancel->SetWindowText("Готово");
      return false;
  	}

   if( t <= 0 || field->nPoly() == 0 ) {  // вычисления завершены
      bStart->SetWindowText("Расчет");
      bStart->EnableWindow(false);
      bCancel->SetWindowText("Готово");
      if( TBrData::wps ) {
         // записываем последнее сечение
         TBrData::wps->storeSection( t, field );
         // помечаем в заголовке wps-файла, что вычисления завершены
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
      // приостанавливаем выполнение после каждого шага если указано
      if( doPause() )
         Suspend();
   }
   if( ( ++ctt & 127 ) == 0 )
      TDialog::IdleAction(idleCount);
   return true;
}

//----------------------------------------------------------------------------

int TStepThread::Run() {   // собственно расчеты моста
   working = true;
   sts = Running;

   // определяем, нужно ли сохранить первое сечение в файл
   // это нужно делать, если мы только начинаем расчеты, и не нужно,
   // если мы их продолжаем.
   sd->save2file = TBrData::wps && ( TBrData::wps->getSectCount() == 0 );

   while( sd->doStep( &terminating ) ) {
      // ничего не делаем
   }
   working = false;
   sts = Finished;
   return 0;
}

void TStepThread::Stop() {     // остановить выполнение если оно идет
   if ( GetStatus() == Suspended )
      sd->Resume();
   terminating = true;
   while( working )
      Sleep(200); // ждем завершения
}

void TStepThread::BNStart() {  // отработать нажатие на кнопку "старт" диалога
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


