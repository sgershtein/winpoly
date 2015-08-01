//----------------------------------------------------------------------------
//  Project winpoly
//  
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    winpoly.apx Application
//  FILE:         stepdlg.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TStepDlg (TDialog).
//
//----------------------------------------------------------------------------
#if !defined(stepdlg_h)  // Sentry, use file only if it's not already included.
#define stepdlg_h

#include <owl/dialog.h>


#include <owl/button.h>
#include <owl/commctrl.h>
#include "polymain.rh"   // Definition of all resources.
#include <owl/gauge.h>
#include <owl/static.h>
#include <owl/checkbox.h>
#include "tfield.h"
#include "brdata.h"

class TStepDlg;   // forward definition

class TStepThread : public TThread {
   public:
      TStepThread( TStepDlg *const dlg ) :
         sd( dlg ), terminating( false ), working( false ), sts( Created ) {};
      void Stop();      // остановить выполнение если оно идет
      void BNStart();   // отработать нажатие на кнопку "старт" диалога
      volatile TStatus sts;  // мой статус, в отличие от глюкавого системного

      ulong Suspend() {
         if( sts != Running )
            return 0;
         sts = Suspended;
         return TThread::Suspend();
      }
      ulong Resume() {
         if( sts != Suspended )
            return 0;
         sts = Running;
         return TThread::Resume();
      }
      TStatus GetStatus() {
         return sts;
      }

   private:
      int Run();        // расчет очередного шага
                        // (виртуальна€ функци€ из TThread)
      TStepDlg *const sd;
      volatile bool terminating; // необходимо завершить выполнение
      volatile bool working;     // функци€ Run() работает
};

struct TStepDlgXfer {
//{{TStepDlgXFER_DATA}}
    char    valueT[ 255 ];
    char    ValueDelta[ 255 ];
    char    valueTime[ 255 ];
    char    valueVC[ 255 ];
    char    valuePcnt[ 255 ];
    uint    cbPause;
    char    valuePC[ 255 ];
//{{TStepDlgXFER_DATA_END}}
};

//{{TDialog = TStepDlg}}
class TStepDlg : public TDialog {
   public:
      bool save2file;   // сохран€ть ли сечение в файл?
      
      TStepDlg(TField* &fld, TDialog* dview,
         TWindow* parent, TResId resId = IDD_STEP, TModule* module = 0);

      virtual ~TStepDlg();
      ulong Suspend() { // приостановить вычислени€
         SetInfo();
         bStart->SetWindowText("–асчет");
         return thread? thread->Suspend() : 0;
      };
      ulong Resume() { // продожить вычислени€
         bStart->SetWindowText("ѕауза");
         return thread? thread->Resume() : 0;
      };
      void* Start() { // начать вычислени€
         bStart->SetWindowText("ѕауза");
         return thread? thread->Start() : NULL;
      };
      void Stop() { // остановить вычислени€
         bStart->EnableWindow( false );
         bCancel->EnableWindow( false );
         if( thread )
            thread->Stop();
         SetInfo();
      }
      bool doPause() { // делать паузу на каждом сечении?
         return cbPause? (cbPause->GetCheck() == BF_CHECKED) : false;
      }
      // сделать шаг поп€тной процедуры.  ¬озвращает true если нужны еще шаги
      // и false, если это был последний шаг. ¬ качестве аргумента передаетс€
      // адрес функции, котора€ возвращает флаг необходимости завершени€
      // вычислений
      bool doStep( bool volatile *ShouldTerminate );
      void SetInfo();   // установить правильные значени€ параметров дл€
                        // вывода в окне диалога и перерисовать диалог

   private:
      TBrData brd;      // параметры задачи
      TField* &field;   // указатель на рабочее поле
      TDialog* Dview;   // указатель на диалог просмотра результатов
      long double t;    // текущее врем€
      long vc;          // текущее количество вершин
      TStepThread *thread; // нить проведени€ расчетов

//{{TStepDlgXFER_DEF}}
  protected:
    TStatic* valueT;
    TStatic* valueDelta;
    TStatic* valueTime;
    TStatic* valueVC;
    TGauge* prBar;
    TStatic* valuePcnt;
    TButton* bStart;
    TButton* bCancel;
    TCheckBox* cbPause;
    TStatic* valuePC;

//{{TStepDlgXFER_DEF_END}}

//{{TStepDlgVIRTUAL_BEGIN}}
  public:
    virtual bool Create();
    virtual void Destroy(int retVal = 0);
    virtual bool IdleAction(long idleCount);
//{{TStepDlgVIRTUAL_END}}

//{{TStepDlgRSP_TBL_BEGIN}}
  protected:
    void BNStart();
    void EvClose();
    void BNCancel();
//{{TStepDlgRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TStepDlg);
};    //{{TStepDlg}}

#endif  // stepdlg_h sentry.

