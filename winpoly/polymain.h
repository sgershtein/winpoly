//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         polymain.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TPolyApp (TApplication).
//
//----------------------------------------------------------------------------
#if !defined(polymain_h)              // Sentry, use file only if it's not already included.
#define polymain_h

#include <owl/opensave.h>
#include <owl/printer.h>

#include "apxprint.h"
#include "apxprev.h"

#include "polymain.rh"            // Definition of all resources.
#include "tfield.h"					 // Working field definition
#include "stepdlg.h"              // диалог пошагового построения моста
#include "brview.h"               // диалог просмотра моста

//
// FrameWindow must be derived to override Paint for Preview and Print.
//
//{{TDecoratedFrame = TSDIDecFrame}}
class TSDIDecFrame : public TDecoratedFrame {
  public:
    TSDIDecFrame(TWindow* parent, const char far* title, TWindow* clientWnd, bool trackMenuSelection = false, TModule* module = 0);
    ~TSDIDecFrame();
};    //{{TSDIDecFrame}}


//{{TApplication = TPolyApp}}
class TPolyApp : public TApplication {
  private:
    bool            HelpState;                          // Has the help engine been used.
    bool            ContextHelp;                        // SHIFT-F1 state(context sensitive HELP)
    TCursor*        HelpCursor;                         // Context sensitive help cursor.


  public:
    TPolyApp();
    virtual ~TPolyApp();

    TOpenSaveDialog::TData FileData;                    // Data to control open/saveas standard dialog.
    void OpenFile(const char* fileName = 0);

    // Public data members used by the print menu commands and Paint routine in MDIChild.
    //
    TPrinter*       Printer;     // Printer support.
    int             Printing;    // Printing in progress.
    TField			  *Field;		// Field where everything happens
    TStepDlg        *Dstep;      // дилаг пошагового построения моста
    TBrView         *Dview;      // диалог просмотра моста

//{{TPolyAppVIRTUAL_BEGIN}}
  public:
    virtual void InitMainWindow();
    virtual bool CanClose();
    virtual bool ProcessAppMsg(MSG& msg);
//{{TPolyAppVIRTUAL_END}}

//{{TPolyAppRSP_TBL_BEGIN}}
  protected:
    void CmFileNew();
    void CmFileOpen();
    void CmFilePrint();
    void CmFilePrintSetup();
    void CmFilePrintPreview();
    void CmPrintEnable(TCommandEnabler& tce);
    void CmHelpContents();
    void CmHelpUsing();
    void CmHelpAbout();
    void EvWinIniChange(char far* section);
    void CmIntersect();
    void CmUnite();
    void CmFilesaveas();
    void CmBuildbridge();
    void CeBuildbridge(TCommandEnabler &tce);
    void CmBuildroute();
    void CmEditcopy();
    void CeEditcopy(TCommandEnabler &tce);
//{{TPolyAppRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TPolyApp);
};    //{{TPolyApp}}


#endif  // polymain_h sentry.
