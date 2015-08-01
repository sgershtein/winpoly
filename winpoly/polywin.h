//----------------------------------------------------------------------------
//  Project Winpoly
//  
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         polywin.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TPolyWindow (TWindow).
//
//----------------------------------------------------------------------------
#if !defined(polywin_h)              // Sentry, use file only if it's not already included.
#define polywin_h

#include "polymain.rh"            // Definition of all resources.


//{{TWindow = TPolyWindow}}
class TPolyWindow : public TWindow {
  public:
    TPolyWindow(TWindow* parent, const char far* title = 0, TModule* module = 0);
    virtual ~TPolyWindow();

    void redrawField(); // перерисовать и вывести рабочее поле

//{{TPolyWindowVIRTUAL_BEGIN}}
  public:
    virtual void Paint(TDC& dc, bool erase, TRect& rect);
    virtual bool IdleAction(long idleCount);
//{{TPolyWindowVIRTUAL_END}}
//{{TPolyWindowRSP_TBL_BEGIN}}
  protected:
    void EvGetMinMaxInfo(MINMAXINFO far& minmaxinfo);
    void EvSize(uint sizeType, TSize& size);
//{{TPolyWindowRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TPolyWindow);
	private:
		bool FieldDirty;	// флаг необходимости перерисовать поле
};    //{{TPolyWindow}}


#endif  // polywin_h sentry.
