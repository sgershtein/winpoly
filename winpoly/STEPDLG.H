//----------------------------------------------------------------------------
//  Project winpoly
//  
//  Copyright � 1998. All Rights Reserved.
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
      void Stop();      // ���������� ���������� ���� ��� ����
      void BNStart();   // ���������� ������� �� ������ "�����" �������
      volatile TStatus sts;  // ��� ������, � ������� �� ��������� ����������

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
      int Run();        // ������ ���������� ����
                        // (����������� ������� �� TThread)
      TStepDlg *const sd;
      volatile bool terminating; // ���������� ��������� ����������
      volatile bool working;     // ������� Run() ��������
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
      bool save2file;   // ��������� �� ������� � ����?
      
      TStepDlg(TField* &fld, TDialog* dview,
         TWindow* parent, TResId resId = IDD_STEP, TModule* module = 0);

      virtual ~TStepDlg();
      ulong Suspend() { // ������������� ����������
         SetInfo();
         bStart->SetWindowText("������");
         return thread? thread->Suspend() : 0;
      };
      ulong Resume() { // ��������� ����������
         bStart->SetWindowText("�����");
         return thread? thread->Resume() : 0;
      };
      void* Start() { // ������ ����������
         bStart->SetWindowText("�����");
         return thread? thread->Start() : NULL;
      };
      void Stop() { // ���������� ����������
         bStart->EnableWindow( false );
         bCancel->EnableWindow( false );
         if( thread )
            thread->Stop();
         SetInfo();
      }
      bool doPause() { // ������ ����� �� ������ �������?
         return cbPause? (cbPause->GetCheck() == BF_CHECKED) : false;
      }
      // ������� ��� �������� ���������.  ���������� true ���� ����� ��� ����
      // � false, ���� ��� ��� ��������� ���. � �������� ��������� ����������
      // ����� �������, ������� ���������� ���� ������������� ����������
      // ����������
      bool doStep( bool volatile *ShouldTerminate );
      void SetInfo();   // ���������� ���������� �������� ���������� ���
                        // ������ � ���� ������� � ������������ ������

   private:
      TBrData brd;      // ��������� ������
      TField* &field;   // ��������� �� ������� ����
      TDialog* Dview;   // ��������� �� ������ ��������� �����������
      long double t;    // ������� �����
      long vc;          // ������� ���������� ������
      TStepThread *thread; // ���� ���������� ��������

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

