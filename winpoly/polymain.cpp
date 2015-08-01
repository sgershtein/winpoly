//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         polymain.cpp
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TPolyApp (TApplication).
//
//----------------------------------------------------------------------------

#include <owl/pch.h>

#include <owl/statusba.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream.h>

#include "polymain.h"
#include "polywin.h"                         // Definition of client class.
#include "aboutdlg.h"                        // Definition of about dialog.

#include "texcept.h"							      // ��������� ����������
#include "brdata.h"                          // ��������� �����
#include "paramdlg.h"                        // ������ ���������� ����������
#include "twps.h"                            // ������ � ������ WPS
#include "broutdlg.h"                        // ���������� ���������� ��������

//
// Generated help file.
//
const char HelpFileName[] = "Winpoly.hlp";


//{{TPolyApp Implementation}}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TPolyApp, TApplication)
//{{TPolyAppRSP_TBL_BEGIN}}
  EV_COMMAND(CM_FILENEW, CmFileNew),
  EV_COMMAND(CM_FILEOPEN, CmFileOpen),
  EV_COMMAND(CM_FILEPRINT, CmFilePrint),
  EV_COMMAND(CM_FILEPRINTERSETUP, CmFilePrintSetup),
  EV_COMMAND(CM_FILEPRINTPREVIEW, CmFilePrintPreview),
  EV_COMMAND_ENABLE(CM_FILEPRINT, CmPrintEnable),
  EV_COMMAND_ENABLE(CM_FILEPRINTERSETUP, CmPrintEnable),
  EV_COMMAND_ENABLE(CM_FILEPRINTPREVIEW, CmPrintEnable),
  EV_COMMAND(CM_HELPCONTENTS, CmHelpContents),
  EV_COMMAND(CM_HELPUSING, CmHelpUsing),
  EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
  EV_WM_WININICHANGE,
  EV_COMMAND(CM_INTERSECT, CmIntersect),
  EV_COMMAND(CM_UNITE, CmUnite),
  EV_COMMAND(CM_FILESAVEAS, CmFilesaveas),
  EV_COMMAND(CM_BUILDBRIDGE, CmBuildbridge),
  EV_COMMAND_ENABLE(CM_BUILDBRIDGE, CeBuildbridge),
  EV_COMMAND_ENABLE(CM_FILENEW, CeBuildbridge),
  EV_COMMAND_ENABLE(CM_INTERSECT, CeBuildbridge),
  EV_COMMAND_ENABLE(CM_UNITE, CeBuildbridge),
  EV_COMMAND_ENABLE(CM_FILEOPEN, CeBuildbridge),
  EV_COMMAND(CM_BUILDROUTE, CmBuildroute),
  EV_COMMAND_ENABLE(CM_BUILDROUTE, CeBuildbridge),
  EV_COMMAND(CM_EDITCOPY, CmEditcopy),
  EV_COMMAND_ENABLE(CM_EDITCOPY, CeEditcopy),
//{{TPolyAppRSP_TBL_END}}
END_RESPONSE_TABLE;


//--------------------------------------------------------
// TPolyApp
//
TPolyApp::TPolyApp() : TApplication("Windows Polygons")
{
  HelpState = false;
  ContextHelp = false;
  HelpCursor = 0;

  Printer = 0;
  Printing = 0;

  // Common file file flags and filters for Open/Save As dialogs.  Filename and directory are
  // computed in the member functions CmFileOpen, and CmFileSaveAs.
  //
  FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  FileData.SetFilter("����� ������ (*.wpl;*.wps)|*.wpl;*.wps|"
    "����� ��������������� (*.wpl)|*.wpl|"
    "����� ������� ������ (*.wps)|*.wps|"
    "��� ����� (*.*)|*.*|");
  FileData.DefExt = "wpl";

  // ��������� �������������
  Field = new TField;
  Dstep = NULL;
  Dview = NULL;

  // �������������� ��������� ��������� �����
  randomize();
}


TPolyApp::~TPolyApp()
{
  delete Printer;
  TBrData brdata;
  brdata.setWPS(NULL);
}


bool TPolyApp::CanClose()
{
  bool result = TApplication::CanClose();

  // Close the help engine if we used it.
  //
  if (result && HelpState)
    GetMainWindow()->WinHelp(HelpFileName, HELP_QUIT, 0);

  return result;
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~
// Application intialization.
//
void TPolyApp::InitMainWindow()
{
  if (nCmdShow != SW_HIDE)
    nCmdShow = (nCmdShow != SW_SHOWMINNOACTIVE) ? SW_SHOWNORMAL : nCmdShow;

  TSDIDecFrame* frame = new TSDIDecFrame(0, GetName(), 0, true);

  // Assign icons for this application.
  //
  frame->SetIcon(this, IDI_SDIAPPLICATION);
  frame->SetIconSm(this, IDI_SDIAPPLICATION);

  // Menu associated with window and accelerator table associated with table.
  //
  frame->AssignMenu(IDM_SDI);

  // Associate with the accelerator table.
  //
  frame->Attr.AccelTable = IDM_SDI;

  TStatusBar* sb = new TStatusBar(frame, TGadget::Recessed,
                                  TStatusBar::CapsLock        |
                                  TStatusBar::NumLock         |
                                  TStatusBar::ScrollLock);
  frame->Insert(*sb, TDecoratedFrame::Bottom);

  SetMainWindow(frame);

  frame->SetMenuDescr(TMenuDescr(IDM_SDI));

  Dview = new TBrView(Field,GetMainWindow()->GetClientWindow());
  Dstep = new TStepDlg(Field,Dview,GetMainWindow()->GetClientWindow());
}



//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~~
// Menu File New command
//
void TPolyApp::CmFileNew() {
   // ������ ��������� ������������� - ������ ���������
   *FileData.FileName = 0;
  	if( !Field )
  		return;
   TWinPoly *box = new TWinPoly;
   *box += CPoint(2,1);
   *box += CPoint(-2,1);
   *box += CPoint(-2,-1);
   *box += CPoint(2,-1);
   Field->clearAll();
   *Field += box;
	TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(), TPolyWindow)->redrawField();
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~~
// Menu File Open command
//
void TPolyApp::CmFileOpen()
{
  // Display standard Open dialog box to select a file name.
  //
  *FileData.FileName = 0;

  TPolyWindow* client =
   TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(),
   TPolyWindow);     // Client window for the frame.
  if (client->CanClose())
    if (TFileOpenDialog(GetMainWindow(), FileData).Execute() == IDOK)
      OpenFile();
}


void TPolyApp::OpenFile(const char* fileName)
{
  	if (fileName)
    	strcpy(FileData.FileName, fileName);
   if( strcmp(FileData.FileName+strlen(FileData.FileName)-4,".wps") ==0 ) {
      // ��������� .wps ����
      TBrData brdata;
      brdata.setWPS(new TWPS);
      try {
         brdata.wps->open(FileData.FileName);
         if( brdata.wps->getSectCount() < 1 ) {
      		GetMainWindow()->MessageBox("���� �� �������� �� ������ �������",
               fileName,MB_OK | MB_ICONSTOP);
         } else {
            // �������� ������ ��������� �����
            if( !Dview->Create() )
               GetMainWindow()->MessageBox("�� ������� ���������������� "
                  "���� ������� ��������� �����",
                  "������!",MB_OK | MB_ICONSTOP);
         }
      } catch( Exception &e ) {
         char buf[100];
         sprintf(buf,"����������� ���������� (%s)",e.where());
         GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      }
   } else {
      // ��������� ���� �������
      if( !Field )
         Throw("TPolyApp::OpenFile: Field is not yet created!");
      try {
         Field->fileOpen(FileData.FileName);	// ������ ������ �� �����
      } catch( Exception &e ) {
         char buf[100];
         sprintf(buf,"����������� ���������� (%s)",e.where());
         GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      }
      // �������� ��, ��� ���������
      TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(), TPolyWindow)->redrawField();
   }
}

// ��������� ������� ��������� ���� � �����
void TPolyApp::CmFilesaveas()
{
   if( !Field )
   	Throw("TPolyApp::CmFilesaveas: Field is not yet created!");

   // ����� ����� ����� ��� ����������
   if (TFileSaveDialog(GetMainWindow(), FileData).Execute() != IDOK)
      return;

   // ��������� ������ � ����
  	try {
   	Field->fileSave(FileData.FileName);
  	} catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
		GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
  	}

}

// ������ ����������� �������� ���� � ����������� �� �����
void TPolyApp::CmUnite()
{
  // Display standard Open dialog box to select a file name.
  //
  *FileData.FileName = 0;

  TPolyWindow* client = TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(), TPolyWindow);     // Client window for the frame.
  if (client->CanClose())
    if (TFileOpenDialog(GetMainWindow(), FileData).Execute() != IDOK)
      return;

   // ������ ������ �� ����� �� ��������� ����
   TField *iField = new TField;
  	try {
   	iField->fileOpen(FileData.FileName);	// ������ ������ �� �����
      // ������ ���������� ����....
      TList<TWinPoly*> uni = Field->unite(iField);
      TField *newField = new TField(uni);
      uni.deletelist();
      delete iField;
      delete Field;
      Field = newField;
  	} catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
		GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
  	}
   // �������� ��, ��� ����������
	TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(), TPolyWindow)->redrawField();
}

// ������ ����������� �������� ���� � ����������� �� �����
void TPolyApp::CmIntersect()
{
  // Display standard Open dialog box to select a file name.
  //
  *FileData.FileName = 0;

  TPolyWindow* client = TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(), TPolyWindow);     // Client window for the frame.
  if (client->CanClose())
    if (TFileOpenDialog(GetMainWindow(), FileData).Execute() != IDOK)
      return;

   // ������ ������ �� ����� �� ��������� ����
   TField *iField = new TField;
  	try {
   	iField->fileOpen(FileData.FileName);	// ������ ������ �� �����
      // ������ ���������� ����....
      TList<TWinPoly*> intersection = Field->intersect(iField);
      TField *newField = new TField(intersection);
      intersection.deletelist();
      delete iField;
      delete Field;
      Field = newField;
  	} catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
		GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
  	}
   // �������� ��, ��� ����������
	TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(), TPolyWindow)->redrawField();
}

// ���������/���������� ������ ���� '���������� �����'
// ����� ���� �������� ���� �������� Dstep � Dview ���
void TPolyApp::CeBuildbridge(TCommandEnabler &tce) {
   tce.Enable( !Dstep->IsWindow() && !Dview->IsWindow() );
}

// ���������� ����������� �����
void TPolyApp::CmBuildbridge() {
   TBrData brdata;
   brdata.setWPS(new TWPS);
   TWPS::WPSHeader wpsh;
   try {
      // �������� ���� ��� ������
      TOpenSaveDialog::TData FileData;
      *FileData.FileName = 0;
      FileData.Flags = OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN |
         OFN_HIDEREADONLY;
      FileData.SetFilter("����� ������� ������ (*.wps)|*.wps");
      FileData.DefExt = "wps";
      if (TFileSaveDialog(GetMainWindow(), FileData, 0, "������� ��� ����� "
         "��� ���������� ����������� �����").Execute() != IDOK)
         return;

      FileData.SetFilter("������ �������� ������ (*.dll)|*.dll");

      // ���������� ������� ���� ��� ������, ���� �� �������, �� ������� ���
      ifstream rtest( FileData.FileName );
      if( rtest ) {
         rtest.close();
         // ���� ����������
         int ans = GetMainWindow()->MessageBox(
            "���������� ������� ����� �����? "
            "���� �� �������� ���, �� ���� "
            "����� �����������","���� ����������!",
            MB_YESNOCANCEL | MB_ICONQUESTION );
         switch( ans ) {
            case IDCANCEL:
               return;
            case IDNO:
               // ������� ����� ����
               brdata.wps->create( FileData.FileName );
               break;
            case IDYES:
               brdata.wps->open( FileData.FileName );
               wpsh = brdata.wps->getHeader();
               if( wpsh.isComplete ) {
                  GetMainWindow()->MessageBox(
                     "���� ���� �������� ��������� ����������� ����.",
                     "���������� ���������",MB_OK | MB_ICONINFORMATION);
                  // �������� ������ ��������� �����
                  if( !Dview->Create() )
                     GetMainWindow()->MessageBox("�� ������� ���������������� "
                        "���� ������� ��������� �����",
                        "������!",MB_OK | MB_ICONSTOP);
                  return;
               }
               char buf[500];
               sprintf(buf, "%s|%s", wpsh.dllname, wpsh.dllname);
               FileData.SetFilter(buf);
               // ��������� ��������� ������� �� ����� �����
               TField *fprev = Field;
               long double tstart=0;
               Field = brdata.wps->getSection(tstart);
               delete fprev;
         	   TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(),
                  TPolyWindow)->redrawField();
               // ������������� �������� ����������
               brdata.T = wpsh.T;
               brdata.sieve = wpsh.sieve;
               brdata.delta = wpsh.delta;
               brdata.fixedT = wpsh.fixedT;
               brdata.tstart = tstart;
               break;
         }
      } else {
         // ������� ����� ����
         brdata.wps->create( FileData.FileName );
      }

      // �������� ������� ������
      // ����������� ������ ������ ����� �����
      *FileData.FileName = 0;
      FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
      FileData.DefExt = "dll";
      if (TFileOpenDialog(GetMainWindow(), FileData, 0,
         "�������� ������ �������� ������").Execute() != IDOK) {
         brdata.wps->close();
         return;
      }
      // ���������� ��������� DLL
      brdata.loadProblemDLL( FileData.FileName );
      // ��������� ������������, ���� ���� WPS ��� ����������...
      if( brdata.wps->getSectCount() >= 0 ) {
         TWPS::WPSHeader wpsh = brdata.wps->getHeader();
         if( strcmp( wpsh.comment, brdata.pComment ) != 0 ) {
            GetMainWindow()->MessageBox(
               "��������� ������ �� ������������� ������ "
               "� ����� ������� �����",
               "�������� ������!",
               MB_OK | MB_ICONSTOP);
               brdata.wps->close();
               return;
         }
      } else {
         // ������� � ��������� �������� ������ � ��� DLL
         wpsh.comment = new char[ strlen(brdata.pComment)+1 ];
         strcpy( wpsh.comment, brdata.pComment );
         // ������� ������ ��� DLL (��� ����)
         char *pathname = new char[300];
         char *basename;
         if( GetFullPathName( FileData.FileName, 300,
            pathname, &basename ) >= 300 )
            Throw("������� ������� ���� �����");
         wpsh.dllname = new char[ strlen( basename )+1 ];
         strcpy( wpsh.dllname, basename );
         delete pathname;
         // �������� ������, ��� �������������
         wpsh.isComplete = false;
      }
  	} catch( Exception &e ) {
  		char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
		GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
      brdata.wps->close();
      return;
  	}

   // ����������� ��������� ������
   if( TParamDlg(Field,GetMainWindow()->GetClientWindow()).Execute() != IDOK )
      return;

   // �������������� ����
	TYPESAFE_DOWNCAST(GetMainWindow()->GetClientWindow(),
      TPolyWindow)->redrawField();

   // ���������� ��������� wps-�����
   wpsh.T = brdata.T;
   wpsh.delta = brdata.delta;
   wpsh.sieve = brdata.sieve;
   wpsh.fixedT = brdata.fixedT;
   brdata.wps->putHeader(wpsh);

   // ������ �������� ����
   if( !Dstep->Create() )
		GetMainWindow()->MessageBox("�� ������� ���������������� ���� �������",
         "������!",MB_OK | MB_ICONSTOP);
}

// ���������� ���������� ��� ��� ������������ �����
void TPolyApp::CmBuildroute() {
   try {
      // �������� ���� ��� ������
      TOpenSaveDialog::TData FileData;
      *FileData.FileName = 0;
      FileData.Flags = OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN |
         OFN_HIDEREADONLY;
      FileData.SetFilter("����� ������� ������ (*.wps)|*.wps");
      FileData.DefExt = "wps";
      if (TFileSaveDialog(GetMainWindow(), FileData, 0, "������� ��� ����� "
         ", ����������� ������� �����").Execute() != IDOK)
         return;
      TBrData brdata;
      brdata.setWPS(new TWPS);
      brdata.wps->open(FileData.FileName);
      if( brdata.wps->getSectCount() < 1 ) {
 		   GetMainWindow()->MessageBox("���� �� �������� �� ������ �������",
            FileData.FileName,MB_OK | MB_ICONSTOP);
         brdata.wps->close();
         return;
      }
      // ��������� DLL ������ - ��� ����� ��� ���������� ���������
      TWPS::WPSHeader wpsh = brdata.wps->getHeader();
      char buf[500];
      sprintf(buf, "%s|%s", wpsh.dllname, wpsh.dllname);
      FileData.SetFilter(buf);
      *FileData.FileName = 0;
      FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
      FileData.DefExt = "dll";
      if (TFileOpenDialog(GetMainWindow(), FileData, 0,
         "�������� ������ �������� ������").Execute() != IDOK) {
         brdata.wps->close();
         return;
      }
      // ���������� ��������� DLL
      brdata.loadProblemDLL( FileData.FileName );
      // ��������� ������������ ��������� DLL
      if( strcmp( wpsh.comment, brdata.pComment ) != 0 ) {
         GetMainWindow()->MessageBox(
            "��������� ������ �� ������������� ������ "
            "� ����� ������� �����",
            "�������� ������!",
            MB_OK | MB_ICONSTOP);
            brdata.wps->close();
            return;
      }

      // ��������� ������ ���������� ����������
      TBRouteDlg(GetMainWindow()->GetClientWindow()).Execute();

      // ������ ��������. ��������� ����.
      brdata.wps->close();

   } catch( Exception &e ) {
      char buf[100];
      sprintf(buf,"����������� ���������� (%s)",e.where());
      GetMainWindow()->MessageBox(e.message(),buf,MB_OK | MB_ICONSTOP);
   }
}

void TPolyApp::CmEditcopy() {
   TClipboard clb = GetMainWindow()->OpenClipboard();
   clb.EmptyClipboard();
   TBitmap *fbm = Field->getPictureBitmap();
   if( fbm )
      fbm->ToClipboard( clb );
   clb.CloseClipboard();
}

void TPolyApp::CeEditcopy(TCommandEnabler &tce) {
   tce.Enable( Field->PictureReady() );
}

//{{TSDIDecFrame Implementation}}

TSDIDecFrame::TSDIDecFrame(TWindow* parent, const char far* title, TWindow* clientWnd, bool trackMenuSelection, TModule* module)
:
  TDecoratedFrame(parent, title, !clientWnd ? new TPolyWindow(0, "") : clientWnd, trackMenuSelection, module)
{
  // INSERT>> Your constructor code here.

}


TSDIDecFrame::~TSDIDecFrame()
{
  // INSERT>> Your destructor code here.

}


//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~
// Menu File Print command
//
void TPolyApp::CmFilePrint()
{
  // Create Printer object if not already created.
  //
  if (!Printer)
    Printer = new TPrinter(this);

  TAPointer<char> docName = new char[_MAX_PATH];
  GetMainWindow()->GetWindowText(docName, _MAX_PATH);

  // Create Printout window and set characteristics.
  //
  TApxPrintout printout(Printer, docName, GetMainWindow()->GetClientWindow());
  printout.SetBanding(true);

  Printing++;

  // Bring up the Print dialog and print the document.
  //
  Printer->Print(GetWindowPtr(GetActiveWindow()), printout, true);

  Printing--;
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~
// Menu File Print Setup command
//
void TPolyApp::CmFilePrintSetup()
{
  if (!Printer)
    Printer = new TPrinter(this);

  // Bring up the Print Setup dialog.
  //
  Printer->Setup(GetMainWindow());
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~
// Menu File Print Preview command
//
void TPolyApp::CmFilePrintPreview()
{
  TSDIDecFrame* sdiFrame = TYPESAFE_DOWNCAST(GetMainWindow(), TSDIDecFrame);
  if (sdiFrame) {
    if (!Printer)
      Printer = new TPrinter(this);

    Printing++;

    TApxPreviewWin* prevW = new TApxPreviewWin(sdiFrame, Printer, sdiFrame->GetClientWindow(), "Print Preview", new TLayoutWindow(0));
    prevW->Create();

    // Here we resize the preview window to take the size of the MainWindow, then
    // hide the MainWindow.
    //
    TFrameWindow * mainWindow =  GetMainWindow();
    TRect r = mainWindow->GetWindowRect();
    prevW->MoveWindow(r);
    prevW->ShowWindow(SW_SHOWNORMAL);
    mainWindow->ShowWindow(SW_HIDE);

    BeginModal(GetMainWindow());

    // After the user closes the preview Window, we take its size and use it
    // to size the MainWindow, then show the MainWindow again.
    //
    r = prevW->GetWindowRect();
    mainWindow->MoveWindow(r);
    mainWindow->ShowWindow(SW_SHOWNORMAL);

    Printing--;

    // Now that printing is off we can invalidate because the edit window to repaint.
    //
    GetMainWindow()->SetRedraw(true);
    GetMainWindow()->Invalidate();

    // We must destroy the preview window explicitly.  Otherwise, the window will
    // not be destroyed until it's parent the MainWindow is destroyed.
    //
    prevW->Destroy();
    delete prevW;
  }
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~
// Menu enabler used by Print, Print Setup and Print Preview.
//
void TPolyApp::CmPrintEnable(TCommandEnabler& tce)
{
  // If we have a Printer already created just test if all is okay.
  // Otherwise create a Printer object and make sure the printer really exists
  // and then delete the Printer object.
  //
  if (!Printer) {
    Printer = new TPrinter(this);
      tce.Enable(!Printer->GetSetup().Error);
  }
  else
    tce.Enable(!Printer->GetSetup().Error);
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~
// Menu Help Contents command
//
void TPolyApp::CmHelpContents()
{
  // Show the help table of contents.
  //
  HelpState = GetMainWindow()->WinHelp(HelpFileName, HELP_CONTENTS, 0);
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~
// Menu Help Using Help command
//
void TPolyApp::CmHelpUsing()
{
  // Display the contents of the Windows help file.
  //
  HelpState = GetMainWindow()->WinHelp(HelpFileName, HELP_HELPONHELP, 0);
}


//--------------------------------------------------------
// TPolyApp
// ~~~~~~~~~~~
// Menu Help About Winpoly command
//
void TPolyApp::CmHelpAbout()
{
  // Show the modal dialog.
  //
  TAboutDlg(MainWindow).Execute();
}



//
// Process application messages to provide context sensitive help
//
bool TPolyApp::ProcessAppMsg(MSG& msg)
{
  if (msg.message == WM_COMMAND) {
    if (ContextHelp || ::GetKeyState(VK_F1) < 0) {
      ContextHelp = false;
      GetMainWindow()->WinHelp(HelpFileName, HELP_CONTEXT, msg.wParam);
      return true;
    }
  }
  else
    switch(msg.message) {
    case WM_KEYDOWN:
      if (msg.wParam == VK_F1) {
        // If the Shift/F1 then set the help cursor and turn on the modal help state.
        //
        if (::GetKeyState(VK_SHIFT) < 0) {
          ContextHelp = true;
          HelpCursor = new TCursor(GetMainWindow()->GetModule()->GetInstance(), TResId(IDC_HELPCURSOR));
          ::SetCursor(*HelpCursor);
          return true;        // Gobble up the message.
        }
        else {
          // If F1 w/o the Shift key then bring up help's main index.
          //
          GetMainWindow()->WinHelp(HelpFileName, HELP_INDEX, 0);
          return true;        // Gobble up the message.
        }
      }
      else {
        if (ContextHelp && msg.wParam == VK_ESCAPE) {
          if (HelpCursor)
            delete HelpCursor;
          HelpCursor = 0;
          ContextHelp = false;
          GetMainWindow()->SetCursor(0, IDC_ARROW);
          return true;    // Gobble up the message.
        }
      }
      break;

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:
      if (ContextHelp) {
        ::SetCursor(*HelpCursor);
        return true;        // Gobble up the message.
      }
      break;

    case WM_INITMENU:
      if (ContextHelp) {
        ::SetCursor(*HelpCursor);
        return true;    // Gobble up the message.
      }
      break;
    case WM_ENTERIDLE:
      if (msg.wParam == MSGF_MENU)
        if (GetKeyState(VK_F1) < 0) {
          ContextHelp = true;
          GetMainWindow()->PostMessage(WM_KEYDOWN, VK_RETURN, 0);
          return true;     // Gobble up the message.
        }
      break;
    default:
      ;
    }  // End of message switch

  // Continue normal processing.
  //
  return TApplication::ProcessAppMsg(msg);
}


void TPolyApp::EvWinIniChange(char far* section)
{
  if (strcmp(section, "windows") == 0) {
    // If the device changed in the WIN.INI file then the printer
    // might have changed.  If we have a TPrinter(Printer) then
    // check and make sure it's identical to the current device
    // entry in WIN.INI.
    //
    if (Printer) {
      const int bufferSize = 255;
      char printDBuffer[bufferSize];
      LPSTR printDevice = printDBuffer;
      LPSTR devName;
      LPSTR driverName = 0;
      LPSTR outputName = 0;
      if (::GetProfileString("windows", "device", "", printDevice, bufferSize)) {
        // The string which should come back is something like:
        //
        //      HP LaserJet III,hppcl5a,LPT1:
        //
        // Where the format is:
        //
        //      devName,driverName,outputName
        //
        devName = printDevice;
        while (*printDevice) {
          if (*printDevice == ',') {
            *printDevice++ = 0;
            if (!driverName)
              driverName = printDevice;
            else
              outputName = printDevice;
          }
          else
            printDevice = ::AnsiNext(printDevice);
        }

        if (Printer->GetSetup().Error != 0                ||
            strcmp(devName, Printer->GetSetup().GetDeviceName()) != 0  ||
            strcmp(driverName, Printer->GetSetup().GetDriverName()) != 0 ||
            strcmp(outputName, Printer->GetSetup().GetOutputName()) != 0 ) {
          // New printer installed so get the new printer device now.
          //
          delete Printer;
          Printer = new TPrinter(this);
        }
      }
      else {
        // No printer installed(GetProfileString failed).
        //
        delete Printer;
        Printer = new TPrinter(this);
      }
    }
  }
}

int OwlMain(int , char* [])
{
  TPolyApp   app;         
  return app.Run();
}


