//----------------------------------------------------------------------------
//  Project Winpoly
//  
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         polywin.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TPolyWindow (TWindow).
//
//----------------------------------------------------------------------------

#include <owl/pch.h>

#include "polymain.h"

#include "polywin.h"

#include <stdio.h>

#include "tdraw.h"


//{{TPolyWindow Implementation}}


//
// Build a response table for all messages/commands handled
// by TPolyWindow derived from TWindow.
//
DEFINE_RESPONSE_TABLE1(TPolyWindow, TWindow)
//{{TPolyWindowRSP_TBL_BEGIN}}
  EV_WM_GETMINMAXINFO,
  EV_WM_SIZE,
//{{TPolyWindowRSP_TBL_END}}
END_RESPONSE_TABLE;


//--------------------------------------------------------
// TPolyWindow
// ~~~~~~~~~~
// Construction/Destruction handling.
//
TPolyWindow::TPolyWindow(TWindow* parent, const char far* title, TModule* module)
: FieldDirty(false), TWindow(parent, title, module)
{
  // INSERT>> Your constructor code here.

}


TPolyWindow::~TPolyWindow()
{
  Destroy();

  // INSERT>> Your destructor code here.

}


//
// Paint routine for Window, Printer, and PrintPreview for an TWindow client.
//
void TPolyWindow::Paint(TDC& /*DC*/, bool, TRect& rect)
{
  TPolyApp* theApp = TYPESAFE_DOWNCAST(GetApplication(), TPolyApp);
  if (theApp) {
    // Only paint if we're printing and we have something to paint, otherwise do nothing.
    //
    if (theApp->Printing && theApp->Printer && !rect.IsEmpty()) {
      // Use pageSize to get the size of the window to render into.  For a Window it's the client area,
      // for a printer it's the printer DC dimensions and for print preview it's the layout window.
      //
      TSize   pageSize(rect.right - rect.left, rect.bottom - rect.top);

      TPrintDialog::TData& printerData = theApp->Printer->GetSetup();

      // Compute the number of pages to print.
      //
      printerData.MinPage = 1;
      printerData.MaxPage = 1;

      // INSERT>> Special printing code goes here.

    }
    else {
      // INSERT>> Normal painting code goes here.

      TField *Field = theApp->Field;	// рабочее поле
      if( !Field )
      	return;
      TClientDC DC(*this); 				// клиентский DC
      TRect Client = GetClientRect(); // место для рисования

      // копируем нарисованную картинку (если она готова) на экран
      if( Field->PictureReady() )
	      DC.BitBlt(0,0,Client.Width(),Client.Height(),
         	*Field->getPictureDC(),0,0);
    }
  }
}

void TPolyWindow::EvGetMinMaxInfo(MINMAXINFO far& minmaxinfo)
{
  TPolyApp* theApp = TYPESAFE_DOWNCAST(GetApplication(), TPolyApp);
  if (theApp) {
    if (theApp->Printing) {
      minmaxinfo.ptMaxSize = TPoint(32000, 32000);
      minmaxinfo.ptMaxTrackSize = TPoint(32000, 32000);
      return;
    }
  }
  TWindow::EvGetMinMaxInfo(minmaxinfo);
}

void TPolyWindow::EvSize(uint sizeType, TSize& size)
{
   TWindow::EvSize(sizeType, size);
	redrawField();
}

void TPolyWindow::redrawField() { // поднять флаг перерисовки поля
	FieldDirty = true;
}

bool TPolyWindow::IdleAction(long idleCount)
{
  bool result = TWindow::IdleAction(idleCount);

  if( FieldDirty ) { // перерисовать поле если поднят флаг
		FieldDirty = false;
      TField* Field = TYPESAFE_DOWNCAST(GetApplication(),
         TPolyApp)->Field;
      if( Field ) {
	      TClientDC DC(*this); 				// клиентский DC
   	   TRect Client = GetClientRect();  // место для рисования
      	Field->preparePicture(DC, Client, this);
      }
  }

  return result;
}

