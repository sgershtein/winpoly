//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         paramdlg.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TParamDlg (TDialog).
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/validate.h>
#include "paramdlg.h"
#include "brdata.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef SG_TArray_REDEFINED
   #error "TArray class used in this program conflicts with one in <classlib/arrays.h>"
   #error "You need to comment out TArray in <classlib/arrays.h>"
   // В стандартном инклуднике TArray - это просто алиас для TArrayAsVector
   // все стандартные библиотеки пользуются TArrayAsVector, так что
   // обычный TArray не нужен. Его можно закомментировать. Конечно,
   // правильнее было бы переименовать мой TArray, но сейчас это слишком
   // сложно, он используется просто везде...
#endif

// класс-валидатор (проверка того факта, что введено число)
class TValidateLD : public TValidator {
   public:
      virtual bool IsValid(const char far* str) {
         char *ep = (char*)str;
         _strtold(str, &ep);
         return ( *ep == ' ' || *ep == 0 );
      }
      virtual void Error(TWindow* owner) {
         owner->MessageBox("Параметр должен быть числовым",
            "Ошибка ввода!",MB_OK | MB_ICONSTOP);
      }
};

//
// Build a response table for all messages/commands handled by the application.
//
DEFINE_RESPONSE_TABLE1(TParamDlg, TDialog)
//{{TParamDlgRSP_TBL_BEGIN}}
  EV_BN_CLICKED(IDC_B_CANCEL, BNCancel),
  EV_BN_CLICKED(IDC_B_OK, BNOk),
//{{TParamDlgRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TParamDlg Implementation}}

static TParamDlgXfer TParamDlgData;

TParamDlg::TParamDlg(TField *fld, TWindow* parent, TResId resId,
   TModule* module) : TDialog(parent, resId, module), Field(fld) {
//{{TParamDlgXFER_USE}}
   edtH = new TEdit(this, IDC_EDELTA, 255);
   edtSieve = new TEdit(this, IDC_ESIEVE, 255);
   edtT = new TEdit(this, IDC_ET, 255);
   rbMcur = new TRadioButton(this, IDC_RB_M_CURRENT, 0);
   rbMsq2 = new TRadioButton(this, IDC_RB_M_SQUARE2, 0);
   rbFixedT = new TRadioButton(this, IDC_RB_FIXEDT, 0);
   pComment = new TStatic(this, IDC_TPROBLEM, 255);
   rbNfixedT = new TRadioButton(this, IDC_RB_NFIXEDT, 0);
  rbNSurvive = new TRadioButton(this, IDC_RB_NSURVIVE, 0);
  rbSurvive = new TRadioButton(this, IDC_RB_SURVIVE, 0);

   SetTransferBuffer(&TParamDlgData);
//{{TParamDlgXFER_USE_END}}

}

TParamDlg::~TParamDlg() {
  Destroy(IDCANCEL);
}

// нажали кнопку Cancel
void TParamDlg::BNCancel() {
   CmCancel();
}

// OK clicked
void TParamDlg::BNOk() {
   // нужно проверить правильность всех значений

   // устанавливаем параметры в TBrData
   char buf[100];

   // основные параметры нельзя менять, если продолжаем
   // прерванный счет
   if( !TBrData::wps || TBrData::wps->getSectCount() <= 0 ) {
      edtT->GetText(buf,100); TBrData::T = _atold(buf);
      TBrData::fixedT = (rbFixedT->GetCheck() == BF_CHECKED);
      TBrData::tstart = TBrData::T;
      if( rbMcur->GetCheck() == BF_CHECKED ) {
         delete TBrData::M;
         TBrData::M = new TField(*Field);
      } else if( !TBrData::M ) {
         // берем стандартное множество - квадратик
         TBrData::M = new TField;
         TWinPoly *box = new TWinPoly();
         *box += CPoint(1,1);
         *box += CPoint(-1,1);
         *box += CPoint(-1,-1);
         *box += CPoint(1,-1);
         *TBrData::M += box;
      }
   } else {
         delete TBrData::M;
         TBrData::M = TBrData::wps->getSection(0);
   }

   edtH->GetText(buf,100); TBrData::delta = _atold(buf);
   edtSieve->GetText(buf,100); TBrData::sieve = _atold(buf);

   TBrData::survive = rbSurvive->GetCheck() == BF_CHECKED;

   CmOk();
}

void TParamDlg::SetupWindow() {
   TDialog::SetupWindow();

   // устанавливаем начальные значения переменных и т.п.
   pComment->SetText( TBrData::pComment );  // комментарий
   if( TBrData::fixedT ) {
      rbFixedT->Check();   // фиксированный момент времени T
      rbNfixedT->Uncheck();
   } else {
      rbNfixedT->Check();  // к моменту времени T
      rbFixedT->Uncheck();
   }

   // выживающие траетории?
   if( TBrData::survive ) {
      rbSurvive->Check();
      rbNSurvive->Uncheck();
   } else {
      rbNSurvive->Check();
      rbSurvive->Uncheck();
      rbSurvive->EnableWindow(false);
   }

   if( Field->nPoly() > 0 && !TBrData::M ) {
      rbMcur->Check();     // текущее поле в качестве M
      rbMsq2->Uncheck();
   } else {
      rbMsq2->Check();     // квадрат со стороной 2 (или заданное в dll)
      rbMcur->Uncheck();   // множество в качестве M
   }
   if( TBrData::M )
      rbMsq2->SetWindowText("Заданное в DLL множество M");
   else
      rbMsq2->SetWindowText("Квадрат с центром (0,0) и стороной 2");
   if( Field->nPoly() == 0 )
      rbMcur->EnableWindow(false);

   // устанавливаем числовые значения
   char buf[100];
   sprintf(buf,"%Lg",TBrData::T); edtT->SetText(buf);
   sprintf(buf,"%Lg",TBrData::delta); edtH->SetText(buf);
   sprintf(buf,"%Lg",TBrData::sieve); edtSieve->SetText(buf);

   // устанавливаем валидаторов
   edtT->SetValidator(new TValidateLD());
   edtH->SetValidator(new TValidateLD());
   edtSieve->SetValidator(new TValidateLD());

   // нельзя менять основные параметры при продолжении счета
   if( TBrData::wps && TBrData::wps->getSectCount() > 0 ) {
      edtT->EnableWindow(false);
      rbMcur->EnableWindow(false);
      rbMsq2->EnableWindow(false);
      rbMsq2->Uncheck();
      rbMcur->Uncheck();
      rbFixedT->EnableWindow(false);
      rbNfixedT->EnableWindow(false);
      rbSurvive->EnableWindow(false);
      rbNSurvive->EnableWindow(false);
   }

}

