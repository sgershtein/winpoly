//----------------------------------------------------------------------------
//  Project winpoly
//  
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         paramdlg.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TParamDlg (TDialog).
//
//----------------------------------------------------------------------------
#if !defined(paramdlg_h)              // Sentry, use file only if it's not already included.
#define paramdlg_h

#include <owl/dialog.h>
#include <owl/static.h>
#include <owl/radiobut.h>
#include <owl/edit.h>
#include <owl/commctrl.h>
#include "polymain.rh"            // Definition of all resources.
#include "tfield.h"


//{{TDialog = TParamDlg}}
struct TParamDlgXfer {
//{{TParamDlgXFER_DATA}}
    char    edtH[ 255 ];
    char    edtSieve[ 255 ];
    char    edtT[ 255 ];
    uint    rbMcur;
    uint    rbMsq2;
    uint    rbFixedT;
    char    pComment[ 255 ];
    uint    rbNfixedT;
    uint    rbNSurvive;
    uint    rbSurvive;
//{{TParamDlgXFER_DATA_END}}
};

class TParamDlg : public TDialog {
  public:
    TParamDlg(TField *fld, TWindow* parent, TResId resId = IDD_PARAM,
      TModule* module = 0);
    virtual ~TParamDlg();

//{{TParamDlgVIRTUAL_BEGIN}}
  public:
    virtual void SetupWindow();
//{{TParamDlgVIRTUAL_END}}

//{{TParamDlgRSP_TBL_BEGIN}}
  protected:
    void BNCancel();
    void BNOk();
//{{TParamDlgRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TParamDlg);

//{{TParamDlgXFER_DEF}}
  protected:
    TField *Field;   // указатель на текущее поле
    TEdit* edtH;
    TEdit* edtSieve;
    TEdit* edtT;
    TRadioButton* rbMcur;
    TRadioButton* rbMsq2;
    TRadioButton* rbFixedT;
    TStatic* pComment;
    TRadioButton* rbNfixedT;
    TRadioButton* rbNSurvive;
    TRadioButton* rbSurvive;

//{{TParamDlgXFER_DEF_END}}
};    //{{TParamDlg}}


#endif  // paramdlg_h sentry.

