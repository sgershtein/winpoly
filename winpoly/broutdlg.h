//----------------------------------------------------------------------------
//  Project winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         broutdlg.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TBRouteDlg (TDialog).
//
//----------------------------------------------------------------------------
#if !defined(broutdlg_h)  // Sentry, use file only if it's not already included.
#define broutdlg_h

#include <owl/dialog.h>


#include <owl/listbox.h>
#include <owl/edit.h>
#include <owl/checkbox.h>
#include <owl/commctrl.h>
#include "polymain.rh"            // Definition of all resources.
#include <owl/slider.h>
#include <owl/button.h>
#include <owl/static.h>
#include "tarray.h"
#include "tpoly.h"

class TPicture : public TButton {
 public:
   TPicture( TWindow* parent, int id = IDC_PICTURE ) :
      mDC( NULL ), mBitmap( NULL ),
      TButton( parent, id ) {};
   void ODADrawEntire(DRAWITEMSTRUCT far& drawInfo);
   TMemoryDC* GetDC() { return mDC; }
   TBitmap* GetBitmap() { return mBitmap; }
   TRect GetRect() { return rect; }
 private:
   TMemoryDC *mDC;  						// DC для рисования поля
   TBitmap *mBitmap;						// Bitmap для рисования поля
   TRect rect;                      // Регион DC для рисования поля

};

struct TBRouteDlgXfer {
//{{TBRouteDlgXFER_DATA}}
    TListBoxData  LPTrace;
    uint    CBColor;
    uint    CBGrid;
    char    EPAdd[ 255 ];
    uint    CBDrawCond;
//{{TBRouteDlgXFER_DATA_END}}
};

// тип данных для хранения информации по каждому шагу траектории
class CRPoint {
   public:
       CPoint xrt;   // Точка нашей (реальной) траектории
       CPoint yrt;   // точка траектории поводыря
       CPoint xv,yv,xu,yu; // управления обоих игроков на обоих траекториях)
       bool done;    // истина, если задача уже решена (попали в целевое мн-во)
   CRPoint() : done(false) {};
};

// тип данных для хранения всей траектории
typedef TArray<CRPoint> TPArray;

class TBRouteDlg;

// класс, реализующий нить вычислений траектории
class TCalcRouteThread : public TThread {
  public:
   TCalcRouteThread( TBRouteDlg *par ) : parent(par) {};
  private:
   int Run();  // вести вычисления
   TBRouteDlg *parent;
};

// основной класс диалога
class TBRouteDlg : public TDialog {
   friend class TCalcRouteThread;
  public:
   TBRouteDlg(TWindow* parent, TResId resId = IDD_BUILDROUTE,
      TModule* module = 0);
   virtual ~TBRouteDlg();
  private:
   void drawSection();        // нарисовать следующее ненарисованное сечение
   void drawRoute();          // нарисовать следующую ненарисованную траекторию
   void buildRouteStep();     // построить один шаг траектории
   int plainsDrawStep;        // плотность сечений на картинке
   unsigned plainsDrawn;      // сколько сечений уже нарисовано
   int routesDrawn;           // сколько траекторий уже нарисовано
   int volatile routesBuilt;  // сколько траекторий уже построено
   TArray<TPArray> route;     // массив траекторий
   void EvDensityScroll(uint code);  // произошел скроллинг
   TCalcRouteThread *crthread;   // нить вычисления траекторий

//{{TBRouteDlgXFER_DEF}}
  protected:
    TPicture* Picture;
    THSlider* HSDensity;
    TCheckBox* CBColor;
    TCheckBox* CBGrid;
    TEdit* EPAdd;
    TListBox* LPTrace;
    TButton* BDel;
    TButton* BAdd;
    TCheckBox* CBDrawCond;

//{{TBRouteDlgXFER_DEF_END}}

//{{TBRouteDlgVIRTUAL_BEGIN}}
  public:
    virtual bool IdleAction(long idleCount);
    virtual void SetupWindow();
//{{TBRouteDlgVIRTUAL_END}}

//{{TBRouteDlgRSP_TBL_BEGIN}}
  protected:
    void BClose();
    void BNPicture();
    void EvCBColor();
    void EvCBGrid();
    void BPAdd();
    void BPDel();
    void LBNSelchange();
    void EPAddChange();
    void EvCBDrawCond();
//{{TBRouteDlgRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TBRouteDlg);
};    //{{TBRouteDlg}}

#endif  // broutdlg_h sentry.

