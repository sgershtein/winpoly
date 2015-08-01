//----------------------------------------------------------------------------
//  Project winpoly
//  
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         brview.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TBrView (TDialog).
//
//----------------------------------------------------------------------------
#if !defined(brview_h)              // Sentry, use file only if it's not already included.
#define brview_h

#include <owl/dialog.h>

#include "polymain.rh"            // Definition of all resources.
#include <owl/scrollba.h>
#include <owl/edit.h>
#include <owl/static.h>
#include "tfield.h"


//{{TDialog = TBrView}}
struct TBrViewXfer {
//{{TBrViewXFER_DATA}}
    char    ValueComment[ 255 ];
    char    ValueNSect[ 255 ];
    char    ValueSNo[ 255 ];
    char    ValueStep[ 255 ];
    char    ValueSTime[ 255 ];
    char    ValueTime[ 255 ];
    TScrollBarData    ScrollTime;
    char    ValueNHoles[ 255 ];
    char    ValueNPoly[ 255 ];
//{{TBrViewXFER_DATA_END}}
};

class TBrView : public TDialog {
   public:
      TBrView(TField* &fld,
         TWindow* parent, TResId resId = IDC_BRVIEW, TModule* module = 0);
      virtual ~TBrView();
   private:
      TField* &field;            // указатель на рабочее поле
      void EvScroll(uint code);  // произошел скроллинг
      void displaySection(unsigned n); // отобразить n-ное сечение

//{{TBrViewXFER_DEF}}
   protected:
      TStatic* ValueComment;
      TStatic* ValueNSect;
      TStatic* ValueSNo;
      TStatic* ValueStep;
      TEdit* ValueSTime;
      TStatic* ValueTime;
      TScrollBar* ScrollTime;
    TStatic* ValueNHoles;
    TStatic* ValueNPoly;

//{{TBrViewXFER_DEF_END}}

//{{TBrViewVIRTUAL_BEGIN}}
  public:
    virtual bool Create();
//{{TBrViewVIRTUAL_END}}

  DECLARE_RESPONSE_TABLE(TBrView);

};    //{{TBrView}}

DEFINE_RESPONSE_TABLE1(TBrView, TDialog)
  EV_CHILD_NOTIFY_ALL_CODES(IDC_SCR_TIME, EvScroll),
END_RESPONSE_TABLE;

#endif  // brview_h sentry.

