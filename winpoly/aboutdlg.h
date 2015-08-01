//----------------------------------------------------------------------------
//  Project Winpoly
//  
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         aboutdlg.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TAboutDlg (TDialog).
//
//----------------------------------------------------------------------------
#if !defined(aboutdlg_h)              // Sentry, use file only if it's not already included.
#define aboutdlg_h

#include <owl/static.h>

#include "polymain.rh"                  // Definition of all resources.


//{{TDialog = TAboutDlg}}
class TAboutDlg : public TDialog {
  public:
    TAboutDlg(TWindow* parent, TResId resId = IDD_ABOUT, TModule* module = 0);
    virtual ~TAboutDlg();

//{{TAboutDlgVIRTUAL_BEGIN}}
  public:
    void SetupWindow();
//{{TAboutDlgVIRTUAL_END}}
};    //{{TAboutDlg}}


// Reading the VERSIONINFO resource.
//
class TProjectRCVersion {
  public:
    TProjectRCVersion(TModule* module);
    virtual ~TProjectRCVersion();

    bool GetProductName(LPSTR& prodName);
    bool GetProductVersion(LPSTR& prodVersion);
    bool GetCopyright(LPSTR& copyright);
    bool GetDebug(LPSTR& debug);

  protected:
    uint8 far*  TransBlock;
    void far*   FVData;

  private:
    // Don't allow this object to be copied.
    //
    TProjectRCVersion(const TProjectRCVersion&);
    TProjectRCVersion& operator = (const TProjectRCVersion&);
};


#endif  // aboutdlg_h sentry.
