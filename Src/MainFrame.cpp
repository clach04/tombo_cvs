#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#if defined(PLATFORM_BE500)
#include <CSO.h>
#endif

#include "Tombo.h"
#include "VarBuffer.h"
#include "MainFrame.h"
#include "resource.h"
#include "Message.h"
#include "Property.h"
#include "TString.h"
#include "SipControl.h"
#include "TreeViewItem.h"
#include "GrepDialog.h"
#include "FilterCtlDlg.h"
#include "VFManager.h"
#include "BookMark.h"
#include "DialogTemplate.h"
#include "BookMarkDlg.h"
#include "StatusBar.h"
#include "PlatformLayer.h"
#include "Win32Platform.h"
#include "PocketPCPlatform.h"
#include "PsPCPlatform.h"
#include "HPCPlatform.h"
#include "LagendaPlatform.h"
#if defined(PLATFORM_PKTPC)
#include "DialogTemplate.h"
#include "DetailsViewDlg.h"
#endif

#ifdef _WIN32_WCE
#if defined(PLATFORM_PKTPC)
#include <Aygshell.h>
#include <Imm.h>
#endif
#if defined(PLATFORM_PSPC)
#include <Aygshell.h>
extern "C" {
	// ?? may be deleted Imm.h ??
UINT WINAPI ImmGetVirtualKey(HWND);
};
#endif
#endif

#include "MemoNote.h"
#include "AboutDialog.h"
#include "SearchDlg.h"
#include "SearchEngine.h"
#include "SearchTree.h"
#include "TomboURI.h"

//#include "YAEditor.h"

LPCTSTR MainFrame::pClassName = TOMBO_MAIN_FRAME_WINDOW_CLSS;

static LRESULT CALLBACK MainFrameWndProc(HWND, UINT, WPARAM, LPARAM);
static HIMAGELIST CreateSelectViewImageList(HINSTANCE hInst);

#define SHGetMenu(hWndMB)  (HMENU)SendMessage((hWndMB), SHCMBM_GETMENU, (WPARAM)0, (LPARAM)0)
#define SHGetSubMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)
#define SHSetSubMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_SETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)

// splitter width
#if defined(PLATFORM_WIN32)
#define BORDER_WIDTH 2
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
#define BORDER_WIDTH 5
#endif

// Bookmark menu ID base value
#define BOOKMARK_ID_BASE 41000

///////////////////////////////////////
// ctor
///////////////////////////////////////

MainFrame::MainFrame() : bResizePane(FALSE), //bSelectViewActive(FALSE), 
	vtFocusedView(VT_Unknown),
	pBookMark(NULL), pDetailsView(NULL), pPlatform(NULL), lCurrentLayout(LT_Unknown)
{
}

///////////////////////////////////////
// dtor
///////////////////////////////////////

MainFrame::~MainFrame()
{
	delete pDetailsView;
	delete pBookMark;
	delete pPlatform;
}

///////////////////////////////////////
// Regist window class
///////////////////////////////////////

BOOL MainFrame::RegisterClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)MainFrameWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInst;
	wc.hIcon = NULL;
#if defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#else
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
#endif
#if defined(PLATFORM_WIN32)
	wc.hCursor = LoadCursor(NULL, IDC_SIZEWE);
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
#endif
	wc.lpszMenuName = NULL;
	wc.lpszClassName = pClassName;

	::RegisterClass(&wc);
	return TRUE;
}

///////////////////////////////////////////////////
// Event Handler
///////////////////////////////////////////////////

static LRESULT CALLBACK MainFrameWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (nMessage == WM_CREATE) {
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		MainFrame *frm = (MainFrame*)pCS->lpCreateParams;
		SetWindowLong(hWnd, 0, (LONG)frm);
		frm->OnCreate(hWnd, wParam, lParam);
		return 0;
	}

	MainFrame *frm = (MainFrame*)GetWindowLong(hWnd, 0);
	if (frm == NULL) {
		return DefWindowProc(hWnd, nMessage, wParam, lParam);
	}

	BOOL bRes;
	switch(nMessage) {
	case WM_CLOSE:
		return frm->OnExit();
	case WM_COMMAND:
		frm->OnCommand(hWnd, wParam, lParam);
		return 0;
	case WM_NOTIFY:
		bRes = frm->OnNotify(hWnd, wParam, lParam);
		if (bRes != 0xFFFFFFFF) return bRes;
		break;
	case MWM_SWITCH_VIEW:
		if (wParam == OPEN_REQUEST_MDVIEW_ACTIVE) {
			frm->ActivateView(MainFrame::VT_DetailsView);
		} else if (wParam == OPEN_REQUEST_MSVIEW_ACTIVE) {
			frm->ActivateView(MainFrame::VT_SelectView);
		}
		return 0;
	case WM_SETFOCUS:
		frm->SetFocus();
		return 0;
	case WM_SETTINGCHANGE:
		frm->OnSettingChange(wParam);
		return 0;
	case WM_TIMER:
		frm->OnTimer(wParam);
		return 0;
	case WM_SIZE:
		frm->OnResize(wParam, lParam);
		return 0;
	case WM_HOTKEY:
		// �n���h���ł�����̂ɂ��Ă̂݃n���h��
		if (frm->OnHotKey(wParam, lParam)) return 0;
		break;
	case WM_LBUTTONDOWN:
		if (g_Property.IsUseTwoPane()) {
			// �y�C���z���̕ύX�J�n
			frm->OnLButtonDown(wParam, lParam);
			return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if (g_Property.IsUseTwoPane()) {
			// �y�C���z���̕ύX��
			frm->OnMouseMove(wParam, lParam);
			return 0;
		}
		break;
	case WM_LBUTTONUP:
		if (g_Property.IsUseTwoPane()) {
			// �y�C���z���̕ύX�I��
			frm->OnLButtonUp(wParam, lParam);
			return 0;
		}
	case MWM_RAISE_MAINFRAME:
		frm->OnMutualExecute();
		return 0;
	}
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
}

//////////////////////////////////////
// message loop
//////////////////////////////////////
// �׍H������Action�{�^�����������ۂ�VK_RETURN���������Ȃ��悤�ɂ��Ă���

//�ʏ��ACTION�V�[�P���X

//KD	VK_F23 1
//KU	VK_F23 1
//KD	VK_RETURN  1
//KU	VK_RETRN  1

//KD	VK_F23(86)	 1
//KU	VK_F23(86)	 c0000001
//KD	VK_RETURN(d) 1
//KU	VK_RETURN(d) c0000001

//KD	VK_PROCESSKEY(e5)	1
//KU	VK_F23(86)			c0000001
//KD	VK_RETURN(d) 1
//KU	VK_RETURN(d) c0000001

#include "File.h"
#include "Uniconv.h"

int MainFrame::MainLoop() {
	MSG msg;

	HACCEL hAccelSv = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL_SELECT));
	HACCEL hAccelDv = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL_DETAIL));

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	BOOL bIgnoreReturnKeyDown = FALSE;
	BOOL bIgnoreReturnKeyUp = FALSE;
	BOOL bIgnoreEscKeyDown = FALSE;
	BOOL bIgnoreEscKeyUp = FALSE;
#endif

	while(GetMessage(&msg, NULL, 0, 0)) {
		// �p�X���[�h�^�C���A�E�g����
		pmPasswordMgr.ForgetPasswordIfNotAccessed();
		if (msg.message == WM_KEYDOWN || msg.message == WM_LBUTTONDOWN) {
			pmPasswordMgr.UpdateAccess();
		}
	
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
		// �A�N�V�����L�[�����ɔ���VK_RETURN�̖���

#if defined(PLATFORM_PKTPC)
		// On PocketPC devices, you can select enable/disable about this feature.
		if (!g_Property.DisableExtraActionButton()) {
		//disable logic begin
#endif

		if (msg.message == WM_KEYDOWN) {
			WPARAM w = msg.wParam;
			if (w == VK_PROCESSKEY) {
				w = ImmGetVirtualKey(msg.hwnd);
			}
			if (w == VK_F23) {
				bIgnoreReturnKeyDown = bIgnoreReturnKeyUp = TRUE;
				continue;
			}
			if (w == VK_F24) {
				bIgnoreEscKeyDown = bIgnoreEscKeyUp = TRUE;
				continue;
			}
			if (bIgnoreReturnKeyDown && w == VK_RETURN) {
				bIgnoreReturnKeyDown = FALSE;
				continue;
			}
			if (bIgnoreEscKeyDown && w == VK_ESCAPE) {
				bIgnoreEscKeyDown = FALSE;
				continue;
			}
		}
		if (msg.message == WM_KEYUP) {
			if (msg.wParam == VK_F23) {
				continue;
			}
			if (bIgnoreReturnKeyUp && msg.wParam == VK_RETURN) {
				bIgnoreReturnKeyUp = FALSE;
				PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_ACTIONBUTTON, 0), 0);
				continue;
			}
			if (msg.wParam == VK_F24) {
				continue;
			}
			if (bIgnoreEscKeyUp && msg.wParam == VK_ESCAPE) {
				bIgnoreEscKeyUp = FALSE;
				PostMessage(hMainWnd, WM_COMMAND, MAKEWPARAM(IDM_RETURNLIST, 0), 0);
				continue;
			}
		}
#if defined(PLATFORM_PKTPC)
		} // disable logic end
#endif

#endif
		// �{���̏���
		if (!TranslateAccelerator(hMainWnd, SelectViewActive() ? hAccelSv : hAccelDv, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

///////////////////////////////////////////////////
// DetailsView callback
///////////////////////////////////////////////////

class MFDetailsViewCallback : public MemoDetailsViewCallback {
	MainFrame *pMainFrame;
public:
	MFDetailsViewCallback(MainFrame *p) : pMainFrame(p) {}
	void GetFocusCallback(MemoDetailsView *pView);
	void SetModifyStatusCallback(MemoDetailsView *pView);
	SearchEngineA *GetSearchEngine(MemoDetailsView *pView);
	void SetReadOnlyStatusCallback(MemoDetailsView *pView);
	void GetCurrentSelectedPath(MemoDetailsView *pView, TString *pPath);

	void SetSearchFlg(BOOL bFlg);
};

void MFDetailsViewCallback::GetFocusCallback(MemoDetailsView *pView)
{
	if (!g_Property.IsUseTwoPane()) return;

	MainFrame *pMf = pMainFrame;
	if (pMf) {
		pMf->SetFocus(MainFrame::VT_DetailsView);

		// menu control
		pMf->EnableDelete(FALSE);
		pMf->EnableRename(FALSE);
		pMf->EnableEncrypt(FALSE);
		pMf->EnableDecrypt(FALSE);
		pMf->EnableNewFolder(FALSE);
		pMf->EnableGrep(FALSE);

		pMf->EnableCut(TRUE);
		pMf->EnableCopy(TRUE);
		pMf->EnablePaste(TRUE);
	}
	pView->SetModifyStatus();
}

void MFDetailsViewCallback::SetModifyStatusCallback(MemoDetailsView *pView)
{
	pMainFrame->SetModifyStatus(pView->IsModify());
}

void MFDetailsViewCallback::SetReadOnlyStatusCallback(MemoDetailsView *pView)
{
	pMainFrame->SetReadOnlyStatus(pView->IsReadOnly());
}

SearchEngineA *MFDetailsViewCallback::GetSearchEngine(MemoDetailsView *pView)
{
	return pMainFrame->GetManager()->GetSearchEngine();
}

void MFDetailsViewCallback::GetCurrentSelectedPath(MemoDetailsView *pView, TString *pPath)
{
	pMainFrame->GetManager()->GetCurrentSelectedPath(pPath);
}

void MFDetailsViewCallback::SetSearchFlg(BOOL bFlg)
{
	pMainFrame->GetManager()->SetMDSearchFlg(bFlg);
}

///////////////////////////////////////////////////
// Create main window
///////////////////////////////////////////////////

BOOL MainFrame::Create(LPCTSTR pWndName, HINSTANCE hInst, int nCmdShow)
{
	hInstance = hInst;

	SimpleEditor::RegisterClass(hInst);

	MFDetailsViewCallback *pCb = new MFDetailsViewCallback(this);
	SimpleEditor *pSe = new SimpleEditor(pCb);
	pDetailsView = pSe;

//	YAEditor *pYAE = new YAEditor();
//	pDetailsView = pYAE;

	mmMemoManager.Init(this, pDetailsView, &msView);
	msView.Init(&mmMemoManager);

	pSe->Init(&mmMemoManager, IDC_MEMODETAILSVIEW, IDC_MEMODETAILSVIEW_NF);
//	pYAE->Init(&mmMemoManager, IDC_TOMBOEDIT);

	pVFManager = new VFManager();
	if (!pVFManager || !pVFManager->Init()) return FALSE;

	pBookMark = new BookMark();
	if (!pBookMark || !pBookMark->Init(BOOKMARK_ID_BASE)) return FALSE;


#ifdef _WIN32_WCE
	hMainWnd = CreateWindow(pClassName, pWndName,
						WS_VISIBLE,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL,
						NULL, 
						hInst,
						this);
#else
#if defined(PLATFORM_WIN32)
	hMainWnd = CreateWindow(pClassName, pWndName,
						WS_OVERLAPPEDWINDOW,
						0,
						0,
						640,
						320,
						NULL,
						Win32Platform::LoadMainMenu(),
						hInst,
						this);
#else 
	// debug mode 
	hMainWnd = CreateWindow(pClassName, pWndName,
						WS_SYSMENU | WS_THICKFRAME,
						0,
						0,
						240,
						320,
						NULL,
						LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_MAIN)), 
						hInst,
						this);
#endif
#endif

	// set application icon
	SetAppIcon(hInstance, hMainWnd);

	// load window positions
#if defined(PLATFORM_WIN32)
	WINDOWPLACEMENT wpl;
	wpl.length = sizeof(wpl);
	WORD nSelectViewWidth;

	if (Property::GetWinSize(&(wpl.flags), &(wpl.showCmd), &(wpl.rcNormalPosition), &nSelectViewWidth)) {
		if (!SetWindowPlacement(hMainWnd, &wpl)) {
			UpdateWindow(hMainWnd);
		}
	} else {
		ShowWindow(hMainWnd, nCmdShow);
		UpdateWindow(hMainWnd);
	}
#else
	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);
#endif

	return TRUE;
}

///////////////////////////////////////////////////
// Initialize window
///////////////////////////////////////////////////

void MainFrame::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	hMainWnd = hWnd;
	LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

	pPlatform = PLATFORM_TYPE::PlatformFactory();
	if (!pPlatform || !pPlatform->Init(hMainWnd)) return;

	RECT r;
	GetClientRect(hWnd, &r);

	// load properties
	BOOL bResult, bStrict;
	bResult = g_Property.Load(&bStrict);
	if (!(bResult && bStrict)) {
		BOOL bPrev = bDisableHotKey;
		bDisableHotKey = TRUE;
		DWORD nResult = g_Property.Popup(pcs->hInstance, hWnd, TEXT(""));
		bDisableHotKey = bPrev;
		if (nResult == IDCANCEL) {
			PostQuitMessage(1);
			return;
		}
	}
	// create toolbar
	pPlatform->Create(hWnd, pcs->hInstance);

	// adjust client area to remove toolbar area
	pPlatform->AdjustUserRect(&r);

	// Status Bar
	SetNewMemoStatus(g_Property.IsUseTwoPane());
	SetModifyStatus(FALSE);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	// control show/hide status bar
	HMENU hMenu = pPlatform->GetMainMenu();
	if (g_Property.HideStatusBar()) {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_UNCHECKED);
	} else {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_CHECKED);
		pPlatform->ShowStatusBar(TRUE);
	}	
#endif

	pPlatform->CheckMenu(IDM_TOGGLEPANE, g_Property.IsUseTwoPane());

	// Create edit view
	pDetailsView->Create(TEXT("MemoDetails"), r, hWnd,  hInstance, g_Property.DetailsViewFont());

	if (!g_Property.WrapText()) {
		SetWrapText(g_Property.WrapText());
	}

	// Create tree view
	msView.Create(TEXT("MemoSelect"), r, hWnd, IDC_MEMOSELECTVIEW, hInstance, g_Property.SelectViewFont());
	msView.InitTree(pVFManager);

	// set auto switch mode
	if (g_Property.IsUseTwoPane()) {
		msView.SetAutoLoadMode(g_Property.AutoSelectMemo());
		msView.SetSingleClickMode(g_Property.SingleClickOpenMemo());
	}

	LoadWinSize(hWnd);
	pDetailsView->SetMemo(TEXT(""), 0, FALSE);

	if (!EnableApplicationButton(hWnd)) {
		TomboMessageBox(hMainWnd, MSG_INITAPPBTN_FAIL, TEXT("Warning"), MB_ICONEXCLAMATION | MB_OK);
	}

	if (g_Property.IsUseTwoPane()) {
		// set new notes
		mmMemoManager.NewMemo();
	}
#if defined(PLATFORM_WIN32)
	SetTopMost();
#endif
//	ActivateView(TRUE);
	ActivateView(VT_SelectView);

	// init password manager
	pmPasswordMgr.Init(hMainWnd, hInstance);
	mmMemoManager.SetPasswordManager(&pmPasswordMgr);
	g_pPasswordManager = &pmPasswordMgr;

	// load bookmark
	LPTSTR pBM = LoadBookMarkFromReg();
	if (pBM) {
		LoadBookMark(pBM);
		delete [] pBM;
	}

	// open top page
	if (_tcslen(g_Property.GetDefaultNote()) != 0) {
		msView.ShowItemByURI(g_Property.GetDefaultNote());
	}
}

///////////////////////////////////////////////////
// set status indicator on statusbar
///////////////////////////////////////////////////

void MainFrame::SetModifyStatus(BOOL bModify)
{
	EnableSaveButton(bModify);
	pPlatform->SetStatusIndicator(3, MSG_UPDATE, bModify);
}

void MainFrame::SetReadOnlyStatus(BOOL bReadOnly) 
{
	pPlatform->SetStatusIndicator(1, MSG_RONLY, bReadOnly); 
}

void MainFrame::SetNewMemoStatus(BOOL bNew)
{
	pPlatform->SetStatusIndicator(2, MSG_NEW, bNew); 
}

///////////////////////////////////////////////////
// exiting
///////////////////////////////////////////////////

BOOL MainFrame::OnExit()
{
	DWORD nYNC;
	if (!mmMemoManager.SaveIfModify(&nYNC, FALSE)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
		TomboMessageBox(hMainWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
//		ActivateView(FALSE);
		ActivateView(VT_DetailsView);
		return FALSE;
	}
	if (nYNC == IDCANCEL) return FALSE;
	pmPasswordMgr.ForgetPassword();

	SaveWinSize();
	g_Property.SaveStatusBarStat();

	g_Property.SaveWrapTextStat();

#if defined(PLATFORM_WIN32)
	g_Property.SaveTopMostStat();
#endif
#if defined(PLATFORM_HPC)
	// save rebar info
	COMMANDBANDSRESTOREINFO cbri[2];
	cbri[0].cbSize = cbri[1].cbSize = sizeof(COMMANDBANDSRESTOREINFO);
	CommandBands_GetRestoreInformation(pPlatform->hMSCmdBar, SendMessage(pPlatform->hMSCmdBar, RB_IDTOINDEX, ID_CMDBAR_MAIN, 0), &cbri[0]);
	CommandBands_GetRestoreInformation(pPlatform->hMSCmdBar, SendMessage(pPlatform->hMSCmdBar, RB_IDTOINDEX, ID_BUTTONBAND, 0), &cbri[1]);
	SetCommandbarInfo(cbri, 2);
#endif

	// save bookmarks
	LPTSTR pBM = pBookMark->ExportToMultiSZ();
	StoreBookMarkToReg(pBM);
	delete [] pBM;

	PostQuitMessage(0);
	return TRUE;
}

///////////////////////////////////////////////////
// WM_COMMAND handling
///////////////////////////////////////////////////

void MainFrame::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// First, current active view tries to handle WM_COMMAND
	switch(vtFocusedView) {
	case VT_SelectView:
		if (msView.OnCommand(hWnd, wParam, lParam)) return;
		break;
	case VT_DetailsView:
		if (pDetailsView->OnCommand(hWnd, wParam, lParam)) return;
		break;
	}

	// if active view can't handle, try to handle main window.
	switch(LOWORD(wParam)) {
#if defined(PLATFORM_BE500)
	case CSOBAR_ADORNMENTID_CLOSE:
		/* fall through */
#endif
	case IDM_EXIT:
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case IDM_NEWMEMO:
		NewMemo();
		break;
	case IDM_NEWFOLDER:
		NewFolder(NULL);
		break;
	case IDM_ABOUT:
		About();
		break;
	case IDM_RETURNLIST:
		LeaveDetailsView(TRUE);
		break;
	case IDM_PROPERTY:
		OnProperty();
		break;
	case IDM_FORGETPASS:
		OnForgetPass();
		break;
	case IDM_SELALL:
		mmMemoManager.SelectAll();
		break;
	case IDM_SAVE:
		if (!mmMemoManager.SaveIfModify(NULL, FALSE)) {
			TCHAR buf[1024];
			wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
			TomboMessageBox(NULL, buf, TEXT("ERROR"), MB_ICONERROR | MB_OK);
		}
		break;
	case IDM_DETAILS_HSCROLL:
		g_Property.SetWrapText(!g_Property.WrapText());
		SetWrapText(g_Property.WrapText());
		break;
	case IDM_TOGGLEPANE:
		TogglePane();
		break;
#if defined(PLATFORM_WIN32)
	case IDM_TOPMOST:
		g_Property.ToggleStayTopMost();
		SetTopMost();
		break;
#endif
	case IDM_SEARCH:
		OnSearch();
		break;
	case IDM_SEARCH_NEXT:
		OnSearchNext(TRUE);
		break;
	case IDM_SEARCH_PREV:
		OnSearchNext(FALSE);
		break;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	case IDM_SHOWSTATUSBAR:
		ToggleShowStatusBar();
		break;
#endif
	case IDM_GREP:
		OnGrep();
		break;
	case IDM_VFOLDER_DEF:
		OnVFolderDef();
		break;
	case IDM_BOOKMARK_ADD:
		OnBookMarkAdd(hWnd, wParam, lParam);
		break;
	case IDM_BOOKMARK_CONFIG:
		OnBookMarkConfig(hWnd, wParam, lParam);
		break;
	}

	if (pBookMark->IsBookMarkID(LOWORD(wParam))) {
		OnBookMark(hWnd, wParam, lParam);
	}
	return;
}

///////////////////////////////////////////////////
// WM_NOTIFY
///////////////////////////////////////////////////
// basically dispatch to each view.
// return false if dispatch is failed.

BOOL MainFrame::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_MEMOSELECTVIEW) {
		return msView.OnNotify(hWnd, wParam, lParam);
	}
#if defined(PLATFORM_HPC)
	if (wParam == IDC_CMDBAND) {
		// when move commandbar, Realign MS/MD view.
		NMREBAR *pnm = (NMREBAR*)lParam;
		if (pnm->hdr.code == RBN_HEIGHTCHANGE) {
			RECT r;
			GetClientRect(hMainWnd, &r);
			OnResize(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
		}
	}
#endif
	return FALSE;
}

///////////////////////////////////////////////////
// WM_SETTINGCHANGE
///////////////////////////////////////////////////

void MainFrame::OnSettingChange(WPARAM wParam)
{
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	BOOL bStat;
	SipControl sc;
	if (!sc.Init()) return;
	if (!sc.GetSipStat(&bStat)) return;

	RECT r = sc.GetRect();
	OnSIPResize(bStat, &r);
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_PSPC)
	if (wParam == SPI_SETWORKAREA) {
		// Change taskbar size
		RECT r;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
		MoveWindow(hMainWnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
	}
#endif
}

///////////////////////////////////////////////////
// IM��ON/OFF�ɔ������T�C�Y
///////////////////////////////////////////////////

void MainFrame::OnSIPResize(BOOL bImeOn, RECT *pSipRect)
{

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500)
	SetLayout();
#endif

#ifdef COMMENT
#if defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	DWORD nTop, nBottom;
	DWORD nClientBottom = pSipRect->top;

#if defined(PLATFORM_PSPC)
	DWORD nHOffset = CommandBar_Height(pPlatform->hMDCmdBar);
#else
	DWORD nHOffset = CSOBar_Height(pPlatform->hMDCmdBar);
#endif
	nTop = nHOffset;
	nBottom = 320 - nHOffset * 2;
	if (bImeOn) {
		RECT rx;
		GetWindowRect(hMainWnd, &rx);
		msView.MoveWindow(0, nTop, 240, nClientBottom - rx.top - nTop);
		pDetailsView->MoveWindow(0, nTop, 240, nClientBottom - rx.top - nTop);
	} else {
		msView.MoveWindow(0, nTop, 240, nBottom);
		pDetailsView->MoveWindow(0, nTop, 240, nBottom);
	}
#endif
#endif
}

///////////////////////////////////////////////////
// hotkey events
///////////////////////////////////////////////////

BOOL MainFrame::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	if (::bDisableHotKey) return FALSE;
	switch (vtFocusedView) {
	case VT_SelectView:
		return msView.OnHotKey(hMainWnd, wParam);
	case VT_DetailsView:
		return pDetailsView->OnHotKey(hMainWnd, wParam);
	}
	return FALSE;
}

///////////////////////////////////////////////////
//  Tooltips
///////////////////////////////////////////////////

void MainFrame::OnTooltip(WPARAM wParam, LPARAM lParam)
{
}

///////////////////////////////////////////////////
//  Press left button
///////////////////////////////////////////////////
//
// start splitter moving

void MainFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	WORD fwKeys = wParam;
	WORD xPos = LOWORD(lParam);

	if (!(fwKeys & MK_LBUTTON)) return;
	bResizePane = TRUE;
	SetCapture(hMainWnd);
}

///////////////////////////////////////////////////
//  Dragging
///////////////////////////////////////////////////

void MainFrame::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD fwKeys = wParam;
	WORD xPos = LOWORD(lParam);

	if (!(fwKeys & MK_LBUTTON) && !bResizePane) return;

	RECT rClient;
	GetClientRect(hMainWnd, &rClient);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)

	SHORT yPos = (SHORT)HIWORD(lParam);

	SHORT yLimit = (SHORT)rClient.bottom;
#ifdef COMMENT
	if (!g_Property.HideStatusBar()) {
		RECT rStatBar;
		pPlatform->GetStatusWindowRect(&rStatBar);
		yLimit -= (SHORT)rStatBar.bottom;
	}
#endif
	if (yPos < 0 || yPos > yLimit) return;
	MovePane(xPos);
#endif
}

///////////////////////////////////////////////////
//  Left button up
///////////////////////////////////////////////////
//
// end splitter move

void MainFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD fwKeys = wParam;
	WORD xPos = LOWORD(lParam);
	WORD yPos = HIWORD(lParam);

	if (!(fwKeys & MK_LBUTTON) && !bResizePane) return;
	bResizePane = FALSE;
	ReleaseCapture();

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	RECT r;
	GetClientRect(hMainWnd, &r);
	WORD wTotalWidth = (WORD)(r.right - r.left);
	if (xPos < 20) {
		xPos = 20;
	}
	if (xPos > wTotalWidth - 20) {
		xPos = wTotalWidth - 20;
	}
	MovePane(xPos);
#endif 
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	RECT r;
	GetClientRect(hMainWnd, &r);
	WORD wTotalHeight = (WORD)(r.bottom - r.top);
	if (yPos < 20) {
		yPos = 20;
	}
	if (yPos > wTotalHeight - 20) {
		yPos = wTotalHeight - 20;
	}
	MovePane(yPos);
#endif 
}

///////////////////////////////////////////////////
//  move pane spliter
///////////////////////////////////////////////////

void MainFrame::MovePane(WORD nSplit)
{
	if (!g_Property.IsUseTwoPane()) return;
	nSplitterSize = nSplit;
	SetLayout();
}

///////////////////////////////////////////////////
// Focus window
///////////////////////////////////////////////////

void MainFrame::SetFocus(ViewType vt)
{
	if (vt != VT_Unknown) {
		vtFocusedView = vt;
	}

	switch(vtFocusedView) {
	case VT_SelectView:
		pPlatform->CloseDetailsView();
		msView.SetFocus();
		break;
	case VT_DetailsView:
		pPlatform->OpenDetailsView();
		pDetailsView->SetFocus();
		break;
	}
}

///////////////////////////////////////////////////
// Create new notes
///////////////////////////////////////////////////

void MainFrame::NewMemo()
{
	SipControl sc;
	if (!sc.Init() || !sc.SetSipStat(TRUE)) {
		TomboMessageBox(hMainWnd, MSG_GETSIPSTAT_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}

	// if modifying notes, confirm save.
	if (g_Property.IsUseTwoPane()) {
		LeaveDetailsView(TRUE);
	}
	SetNewMemoStatus(TRUE);
	mmMemoManager.NewMemo();

//	ActivateView(FALSE);
	ActivateView(VT_DetailsView);
}

///////////////////////////////////////////////////
// Create new folder
///////////////////////////////////////////////////

void MainFrame::NewFolder(TreeViewItem *pItem)
{
	if (!mmMemoManager.MakeNewFolder(hMainWnd, pItem)) {
		TomboMessageBox(hMainWnd, MSG_CREATEFOLDER_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
}

///////////////////////////////////////////////////
// version dialog
///////////////////////////////////////////////////

void MainFrame::About()
{
	AboutDialog dlg;
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	dlg.Popup(hInstance, hMainWnd);
	bDisableHotKey = bPrev;
}

///////////////////////////////////////////////////
// Set note's headline to window title
///////////////////////////////////////////////////

void MainFrame::SetWindowTitle(TomboURI *pURI)
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	if (g_Property.SwitchWindowTitle()) {
		// change window title
		LPCTSTR pPrefix = TEXT("Tombo - ");
		LPCTSTR pBase;
		TString sHeadLine;
		if (pURI->GetHeadLine(&sHeadLine)) {
			pBase = sHeadLine.Get();
		} else {
			pBase = TEXT("");
		}
		LPCTSTR pWinTitle;
#if defined(PLATFORM_PKTPC)
		pWinTitle = pBase;
#else
		TString sWinTitle;
		if (sWinTitle.Join(pPrefix, pBase)) {
			pWinTitle = sWinTitle.Get();
		} else {
			pWinTitle = pPrefix;
		}
#endif
		SetWindowText(hMainWnd, pWinTitle);
	}
#endif
}

///////////////////////////////////////////////////
// popup edit dialog
///////////////////////////////////////////////////

#ifdef COMMENT
void MainFrame::PopupEditViewDlg()
{
#if defined(PLATFORM_PKTPC)
	LPTSTR p = pDetailsView->GetMemo(); // p is deleted by DetailsViewDlg

	DetailsViewDlg dlg;
	int result = dlg.Popup(g_hInstance, hMainWnd, &mmMemoManager, p);

	if (result == IDYES) {
	}

#endif
}
#endif

///////////////////////////////////////////////////
// Request open the note
///////////////////////////////////////////////////
// switch edit view when bSwitchView is TRUE

void MainFrame::OpenDetailsView(LPCTSTR pURI, DWORD nSwitchView)
{
	TomboURI uri;
	if (!uri.Init(pURI)) return;

	if (((nSwitchView & OPEN_REQUEST_MSVIEW_ACTIVE) == 0) && (uri.IsEncrypted() && !pmPasswordMgr.IsRememberPassword())) {
		// bSwitchView��FALSE�ŁA�������J�����߂Ƀp�X���[�h��₢���킹��K�v������ꍇ�ɂ�
		// �����͊J���Ȃ�
		return;
	}
	mmMemoManager.SetMemo(&uri);
	SetNewMemoStatus(FALSE);

	SetWindowTitle(&uri);

	if (g_Property.IsUseTwoPane()) {
		if (nSwitchView & OPEN_REQUEST_MSVIEW_ACTIVE) {
			ActivateView(VT_DetailsView);
		}
	} else {
		ActivateView(VT_DetailsView);
	}
}

///////////////////////////////////////////////////
// load notes
///////////////////////////////////////////////////

void MainFrame::LoadMemo(LPCTSTR pURI, BOOL bAskPass)
{
	TomboURI uri;
	if (!uri.Init(pURI)) return;

	if (uri.IsEncrypted() && 
		!pmPasswordMgr.IsRememberPassword() &&
		bAskPass == FALSE) {
		// if TOMBO doesn't keep password even though it is need
		// and caller don't want to ask password, nothing to do
		return;
	}
	mmMemoManager.SetMemo(&uri);
	SetNewMemoStatus(FALSE);

	SetWindowTitle(&uri);
}

///////////////////////////////////////////////////
// leave edit view and return to treeview
///////////////////////////////////////////////////

void MainFrame::LeaveDetailsView(BOOL bAskSave)
{
	SipControl sc;
	if (!sc.Init() || !sc.SetSipStat(FALSE)) {
		TomboMessageBox(hMainWnd, MSG_GETSIPSTAT_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}

	DWORD nYNC;
	BOOL bResult;
	if (bAskSave) {
		bResult = mmMemoManager.SaveIfModify(&nYNC, FALSE);
	} else {
		nYNC = IDYES;
		bResult = mmMemoManager.SaveIfModify(NULL, TRUE);
	}
	if (!bResult) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
		TomboMessageBox(hMainWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
//		ActivateView(FALSE);
		ActivateView(VT_DetailsView);
		return;
	}
	if (nYNC == IDCANCEL) return;

//	ActivateView(TRUE);
	ActivateView(VT_SelectView);

	if (g_Property.IsUseTwoPane()) {
		// clear encrypted notes if two pane mode
		if (nYNC == IDNO) {
			// discard current note and load old one.
			if (mmMemoManager.GetCurrentURI()) {
				OpenDetailsView(mmMemoManager.GetCurrentURI(), OPEN_REQUEST_MDVIEW_ACTIVE);
			} else {
				mmMemoManager.NewMemo();
			}
		} else {
			// nYNC == YES so note has been saved.
			if (mmMemoManager.GetCurrentURI()) {
				TomboURI uri;
				if (!uri.Init(mmMemoManager.GetCurrentURI())) return;
				if (uri.IsEncrypted()) {
					mmMemoManager.NewMemo();
				}
			}
		}
	} else {
		mmMemoManager.NewMemo();
		SetNewMemoStatus(FALSE);
	}

#if defined(PLATFORM_PKTPC)
	SetTitle(TOMBO_APP_NAME);
#endif
	SetFocus();
}

///////////////////////////////////////////////////
// switch view
///////////////////////////////////////////////////

void MainFrame::ActivateView(ViewType vt)
{
	if (vt == vtFocusedView) {
		SetFocus();
		return;
	}

	vtFocusedView = vt;
	SetLayout();
	SetFocus();
}

///////////////////////////////////////////////////
// change layout
///////////////////////////////////////////////////

void MainFrame::SetLayout()
{
	if (g_Property.IsUseTwoPane()) {
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
		ChangeLayout(LT_TwoPane);
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
		switch(vtFocusedView) {
		case VT_SelectView:
			ChangeLayout(LT_TwoPane);
			break;
		case VT_DetailsView:
			ChangeLayout(LT_OnePaneDetailsView);
			break;
		}
#endif
	} else {
		switch (vtFocusedView) {
		case VT_SelectView:
			ChangeLayout(LT_OnePaneSelectView);
			break;
		case VT_DetailsView:
			ChangeLayout(LT_OnePaneDetailsView);
			break;
		}
	}
}

///////////////////////////////////////////////////
// Switch pane mode
///////////////////////////////////////////////////
void MainFrame::TogglePane()
{
	pPlatform->CheckMenu(IDM_TOGGLEPANE, !g_Property.IsUseTwoPane());

	if (g_Property.IsUseTwoPane()) {
		SaveWinSize();
	}

	DWORD nPane = g_Property.IsUseTwoPane() ? MF_UNCHECKED : MF_CHECKED;
	g_Property.SetUseTwoPane(nPane);

	SetLayout();
}

///////////////////////////////////////////////////
// Switch pane
///////////////////////////////////////////////////

void MainFrame::ChangeLayout(LayoutType layout)
{
	pPlatform->ShowStatusBar(!g_Property.HideStatusBar());

	// get tree/edit view area
	RECT r, rc;
	GetClientRect(hMainWnd, &r);
	rc = r;
	pPlatform->AdjustUserRect(&rc);

	switch(layout) {
	case LT_TwoPane:
		{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
			// split vertical
			msView.MoveWindow(rc.left, rc.top , nSplitterSize, rc.bottom);
			pDetailsView->MoveWindow(nSplitterSize + BORDER_WIDTH, rc.top, rc.right - nSplitterSize - BORDER_WIDTH, rc.bottom);
#else
			// split horizontal
			msView.MoveWindow(rc.left, rc.top , rc.right, nSplitterSize);
//			pDetailsView->MoveWindow(rc.left, rc.top + nSplitterSize + BORDER_WIDTH, rc.right, rc.bottom - nSplitterSize - BORDER_WIDTH - rc.top);
			pDetailsView->MoveWindow(
				rc.left, rc.top + nSplitterSize + BORDER_WIDTH, 
				rc.right, rc.bottom - nSplitterSize - BORDER_WIDTH);
#endif

			msView.Show(SW_SHOW);
			pDetailsView->Show(SW_SHOW);
		}
		break;
	case LT_OnePaneSelectView:
		{
			WORD wLeftWidth, wHeight;
			msView.GetSize(&wLeftWidth, &wHeight);

			msView.MoveWindow(rc.left, rc.top, rc.right, rc.bottom);
			pDetailsView->MoveWindow(rc.left, rc.top, rc.right, rc.bottom);

			pDetailsView->Show(SW_HIDE);
			msView.Show(SW_SHOW);
		}
		break;
	case LT_OnePaneDetailsView:
		{
			WORD wLeftWidth, wHeight;
			msView.GetSize(&wLeftWidth, &wHeight);

			msView.MoveWindow(rc.left, rc.top, rc.right, rc.bottom);
			pDetailsView->MoveWindow(rc.left, rc.top, rc.right, rc.bottom);

			pDetailsView->Show(SW_SHOW);
			msView.Show(SW_HIDE);
		}
		break;
	}
	lCurrentLayout = layout;

	// Staus bar & rebar
	pPlatform->ResizeStatusBar(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
}

///////////////////////////////////////////////////
//  Resize window
///////////////////////////////////////////////////

void MainFrame::OnResize(WPARAM wParam, LPARAM lParam)
{
	SetLayout();
}

///////////////////////////////////////////////////
// Menu control
///////////////////////////////////////////////////
void MainFrame::EnableEncrypt(BOOL bEnable) { pPlatform->EnableMenu(IDM_ENCRYPT, bEnable);}
void MainFrame::EnableDecrypt(BOOL bEnable) { pPlatform->EnableMenu(IDM_DECRYPT, bEnable);}
void MainFrame::EnableDelete(BOOL bEnable) { pPlatform->EnableMenu(IDM_DELETEITEM, bEnable); }
void MainFrame::EnableRename(BOOL bEnable) { pPlatform->EnableMenu(IDM_RENAME, bEnable); }
void MainFrame::EnableNew(BOOL bEnable) { pPlatform->EnableMenu(IDM_NEWMEMO, bEnable); }
void MainFrame::EnableCut(BOOL bEnable) { pPlatform->EnableMenu(IDM_CUT, bEnable); }
void MainFrame::EnableCopy(BOOL bEnable) { pPlatform->EnableMenu(IDM_COPY, bEnable); }
void MainFrame::EnablePaste(BOOL bEnable) { pPlatform->EnableMenu(IDM_PASTE, bEnable); }
void MainFrame::EnableNewFolder(BOOL bEnable) { pPlatform->EnableMenu(IDM_NEWFOLDER, bEnable); }
void MainFrame::EnableGrep(BOOL bEnable) { pPlatform->EnableMenu(IDM_GREP, bEnable);}
void MainFrame::EnableSaveButton(BOOL bEnable) { pPlatform->EnableMenu(IDM_SAVE, bEnable); }

///////////////////////////////////////////////////
// erase password information
///////////////////////////////////////////////////

void MainFrame::OnForgetPass()
{
	DWORD nYNC;
	if (!mmMemoManager.SaveIfModify(&nYNC, FALSE)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
		TomboMessageBox(hMainWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
//		ActivateView(FALSE);
		ActivateView(VT_DetailsView);
		return;
	}
	if (nYNC == IDCANCEL) return;

	pmPasswordMgr.ForgetPassword();
	TomboMessageBox(hMainWnd, MSG_ERASE_PW, MSG_ERASE_PW_TITLE, MB_ICONINFORMATION | MB_OK);
	mmMemoManager.NewMemo();
}

///////////////////////////////////////////////////
// change property
///////////////////////////////////////////////////

void MainFrame::OnProperty()
{
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;

	mmMemoManager.NewMemo();

	TString sPath;
	if (!msView.GetURI(&sPath)) {
		sPath.Set(TEXT(""));
	}

	int nResult = g_Property.Popup(hInstance, hMainWnd, sPath.Get());
	bDisableHotKey = bPrev;
	if (nResult != IDOK) return;

	// font setting
	msView.SetFont(g_Property.SelectViewFont());
	pDetailsView->SetFont(g_Property.DetailsViewFont());

	// tabstop setting
	pDetailsView->SetTabstop();

	// reload notes and folders
	msView.DeleteAllItem();
	msView.InitTree(pVFManager);
#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC)
	if (!g_Property.SwitchWindowTitle()) {
		SetWindowText(hMainWnd, TOMBO_APP_NAME);
	}
#endif
}

///////////////////////////////////////////////////
// timer events
///////////////////////////////////////////////////

void MainFrame::OnTimer(WPARAM nTimerID)
{
	if (nTimerID == 0) {
		if (!SelectViewActive()) {
			if (mmMemoManager.GetCurrentURI()) {
				TomboURI uri;
				if (!uri.Init(mmMemoManager.GetCurrentURI())) return;
				if (uri.IsEncrypted()) {
					LeaveDetailsView(FALSE);
				}
			}
		}
		pmPasswordMgr.ForgetPassword();
	} else if (nTimerID == ID_PASSWORDTIMER) {
		pmPasswordMgr.ForgetPasswordIfNotAccessed();
	}
}

///////////////////////////////////////////////////
// suppress mutual execution
///////////////////////////////////////////////////
void MainFrame::OnMutualExecute()
{
	SetForegroundWindow(hMainWnd);
#if defined(PLATFORM_WIN32)
	BringWindowToTop(hMainWnd);
	ShowWindow(hMainWnd, SW_RESTORE);
#endif
	OnSettingChange(NULL);
}

///////////////////////////////////////////////////
// enable application button handling
///////////////////////////////////////////////////
// http://www.pocketpcdn.com/qa/handle_hardware_keys.html

typedef BOOL (__stdcall *UnregisterFunc1Proc)(UINT, UINT);

BOOL MainFrame::EnableApplicationButton(HWND hWnd)
{
#if defined(PLATFORM_PKTPC)
	HINSTANCE hCoreDll;
	UnregisterFunc1Proc procUnregisterFunc;
	hCoreDll = LoadLibrary(TEXT("coredll.dll"));
	if (!hCoreDll) return FALSE;
	procUnregisterFunc = (UnregisterFunc1Proc)GetProcAddress(hCoreDll, TEXT("UnregisterFunc1"));
	if (!procUnregisterFunc) {
		FreeLibrary(hCoreDll);
		return FALSE;
	}
	if (g_Property.AppButton1()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON1);
		RegisterHotKey(hWnd, APP_BUTTON1, MOD_WIN, APP_BUTTON1);
	}
	if (g_Property.AppButton2()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON2);
		RegisterHotKey(hWnd, APP_BUTTON2, MOD_WIN, APP_BUTTON2);
	}
	if (g_Property.AppButton3()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON3);
		RegisterHotKey(hWnd, APP_BUTTON3, MOD_WIN, APP_BUTTON3);
	}
	if (g_Property.AppButton4()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON4);
		RegisterHotKey(hWnd, APP_BUTTON4, MOD_WIN, APP_BUTTON4);
	}
	if (g_Property.AppButton5()) {
		procUnregisterFunc(MOD_WIN, APP_BUTTON5);
		RegisterHotKey(hWnd, APP_BUTTON5, MOD_WIN, APP_BUTTON5);
	}

	FreeLibrary(hCoreDll);
	return TRUE;
#else
	return TRUE;
#endif
}

///////////////////////////////////////////////////
// Save window size
///////////////////////////////////////////////////

void MainFrame::SaveWinSize()
{
	RECT r;
	UINT flags, showCmd;

#if defined(PLATFORM_HPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	GetWindowRect(hMainWnd,&r);
	flags = showCmd = 0;
#else
	WINDOWPLACEMENT wpl;
	wpl.length = sizeof(wpl);
	GetWindowPlacement(hMainWnd, &wpl);
	r = wpl.rcNormalPosition;
	flags = wpl.flags;
	showCmd = wpl.showCmd;
#endif

	WORD nPane;
	if (g_Property.IsUseTwoPane()) {
		nPane = nSplitterSize;
	} else {
		UINT u1, u2;
		RECT r2;
		if (!Property::GetWinSize(&u1, &u2, &r2, &nPane)) {
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
			nPane = (r.bottom - r.top) / 3 * 2;
#else
			nPane = (r.right - r.left) / 3;	
#endif
		}
	}
	Property::SaveWinSize(flags, showCmd, &r, nPane);
}

///////////////////////////////////////////////////
// Restore window size
///////////////////////////////////////////////////

void MainFrame::LoadWinSize(HWND hWnd)
{
	RECT rMainFrame;
	RECT rClientRect;
	GetClientRect(hWnd, &rClientRect);

	UINT u1, u2;
	if (!Property::GetWinSize(&u1, &u2, &rMainFrame, &nSplitterSize)) {
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_BE500) || defined(PLATFORM_PSPC)
		nSplitterSize = (rClientRect.right - rClientRect.left) / 3 * 2;
#else
		nSplitterSize = (rClientRect.right - rClientRect.left) / 3;
#endif
	}
}

///////////////////////////////////////////////////
// set wrpping text or not
///////////////////////////////////////////////////

void MainFrame::SetWrapText(BOOL bWrap)
{
	// Change edit view status
	if (!pDetailsView->SetFolding(bWrap)) {
		TomboMessageBox(NULL, MSG_FOLDING_FAILED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		return;
	}

	UINT uCheckFlg;

	HMENU hMenu = pPlatform->GetMDToolMenu();
	if (bWrap) {
		uCheckFlg = MF_CHECKED;
	} else {
		uCheckFlg = MF_UNCHECKED;
	}

	// CheckMenuItem is superseded, but CE don't have SetMenuItemInfo.
	CheckMenuItem(hMenu, IDM_DETAILS_HSCROLL, MF_BYCOMMAND | uCheckFlg);
}

///////////////////////////////////////////////////
// Change window title
///////////////////////////////////////////////////

void MainFrame::SetTitle(LPCTSTR pTitle) {
	if (!g_Property.SwitchWindowTitle()) return;
	SetWindowText(hMainWnd, pTitle);
}

///////////////////////////////////////////////////
// Searching
///////////////////////////////////////////////////

void MainFrame::OnSearch()
{
	SearchDialog sd;
	if (sd.Popup(g_hInstance, hMainWnd, SelectViewActive()) != IDOK) return;

	SearchEngineA *pSE = new SearchEngineA();
	if(!pSE->Init(sd.IsSearchEncryptMemo(), sd.IsFileNameOnly(), &pmPasswordMgr)) {
		delete pSE;
		return;
	}
	const char *pReason;
	if (!pSE->Prepare(sd.SearchString(), sd.IsCaseSensitive(), &pReason)) {
		LPTSTR p = ConvSJIS2Unicode(pReason);
		if (p) {
			MessageBox(p, TOMBO_APP_NAME, MB_OK | MB_ICONEXCLAMATION);
			delete [] p;
		} else {
			MessageBox(MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONEXCLAMATION);
		}
		delete pSE;
		return;
	}
	mmMemoManager.SetSearchEngine(pSE);

	// Enable FindNext/Prev button
	pPlatform->EnableSearchNext();

	bSearchStartFromTreeView = SelectViewActive();

	// execute searching
	if (SelectViewActive()) {
		DoSearchTree(TRUE, !sd.IsSearchDirectionUp());
		mmMemoManager.SetMSSearchFlg(FALSE);
	} else {
		pDetailsView->Search(TRUE, TRUE, TRUE, FALSE);
		mmMemoManager.SetMDSearchFlg(FALSE);
	}
}

void MainFrame::DoSearchTree(BOOL bFirst, BOOL bForward)
{

	SearchEngineA *pSE = mmMemoManager.GetSearchEngine();

	TString sPath, sFullPath;
	// Get path of selecting on treeview.
	TreeViewItem *pItem = msView.GetCurrentItem();
	if (pItem == NULL || !pItem->GetLocationPath(&msView, &sPath)) return;
	if (!sFullPath.Join(g_Property.TopDir(), TEXT("\\"), sPath.Get())) return;

	// Create dialog and do search.
	SearchTree st;
	st.Init(pSE, sFullPath.Get(), _tcslen(g_Property.TopDir()), bForward, !bFirst);
	st.Popup(g_hInstance, hMainWnd);

	TCHAR buf[1024];

	switch(st.GetResult()) {
	case SR_FOUND:
		msView.ShowItem(st.GetPartPath());
		mmMemoManager.SearchDetailsView(TRUE, TRUE, TRUE, TRUE);
		break;
	case SR_NOTFOUND:
		MessageBox(MSG_STRING_NOT_FOUND, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
		break;
	case SR_CANCELED:
		msView.ShowItem(st.GetPartPath());
		MessageBox(MSG_STRING_SEARCH_CANCELED, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
		break;
	case SR_FAILED:
		wsprintf(buf, MSG_SEARCH_FAILED, GetLastError());
		msView.ShowItem(st.GetPartPath());
		MessageBox(buf, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		break;
	}
}

///////////////////////////////////////////////////
// Search next one
///////////////////////////////////////////////////

void MainFrame::OnSearchNext(BOOL bForward)
{
	if (mmMemoManager.GetSearchEngine() == NULL) {
		OnSearch();
		return;
	}

	if (SelectViewActive()) {
		DoSearchTree(mmMemoManager.MSSearchFlg(), bForward);
		mmMemoManager.SetMSSearchFlg(FALSE);
	} else {
		// if search starts at edit view, show message when match failed.
		// if starts at tree view, search next item.
		BOOL bMatched = pDetailsView->Search(mmMemoManager.MDSearchFlg(), bForward, !bSearchStartFromTreeView, FALSE);
		mmMemoManager.SetMDSearchFlg(FALSE);
		if (bSearchStartFromTreeView && !bMatched) {
//			ActivateView(TRUE);
			ActivateView(VT_SelectView);
			DoSearchTree(mmMemoManager.MSSearchFlg(), bForward);
			mmMemoManager.SetMSSearchFlg(FALSE);
		}
	}
}

///////////////////////////////////////////////////
// show/hide status bar
///////////////////////////////////////////////////

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
void MainFrame::ToggleShowStatusBar()
{
	g_Property.ToggleShowStatusBar();

	HMENU hMenu = pPlatform->GetMainMenu();

	if (g_Property.HideStatusBar()) {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_UNCHECKED);
	} else {
		CheckMenuItem(hMenu, IDM_SHOWSTATUSBAR, MF_BYCOMMAND | MF_CHECKED);
	}

	RECT r;
	GetClientRect(hMainWnd, &r);
	OnResize(0, MAKELPARAM(r.right - r.left, r.bottom - r.top));
}
#endif

///////////////////////////////////////////////////
// get command bar from command band by ID
///////////////////////////////////////////////////

int MainFrame::MessageBox(LPCTSTR pText, LPCTSTR pCaption, UINT uType)
{
	return TomboMessageBox(hMainWnd, pText, pCaption, uType);
}

///////////////////////////////////////////////////
// Grep
///////////////////////////////////////////////////

void MainFrame::OnGrep()
{
	HTREEITEM hItem;
	TString sPath;
	hItem = msView.GetPathForNewItem(&sPath);
	if (hItem == NULL) return;

	GrepDialog gd;
	if (!gd.Init(sPath.Get())) return;
	if (gd.Popup(hInstance, hMainWnd) == IDOK) {
		const VFInfo *pInfo;
		pInfo = pVFManager->GetGrepVFInfo(gd.GetPath(), gd.GetMatchString(),
				gd.IsCaseSensitive(), gd.IsCheckCryptedMemo(),
				gd.IsCheckFileName(), gd.IsNegate());
		if (pInfo == NULL) return;
		if (!msView.InsertVirtualFolder(pInfo)) {
			MessageBox(MSG_INSERTVFOLDER_FAIL, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		}
	}
}

///////////////////////////////////////////////////
// stay topmost of the screen
///////////////////////////////////////////////////

void MainFrame::SetTopMost()
{
#if defined(PLATFORM_WIN32)
	HMENU hMenu = GetMenu(hMainWnd);

	if (g_Property.StayTopMost()) {
		CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
		SendMessage(pPlatform->hToolBar, TB_SETSTATE, IDM_TOPMOST, MAKELONG(TBSTATE_ENABLED |TBSTATE_PRESSED, 0)); 

		SetWindowPos(hMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else {
		CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);
		SendMessage(pPlatform->hToolBar, TB_SETSTATE, IDM_TOPMOST, MAKELONG(TBSTATE_ENABLED, 0)); 

		SetWindowPos(hMainWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
#endif
}

///////////////////////////////////////////////////
// stay topmost of the screen
///////////////////////////////////////////////////

void MainFrame::OnVFolderDef()
{
	FilterCtlDlg dlg;
	if (!dlg.Init(&msView, pVFManager)) return;
	msView.CloseVFRoot();
	dlg.Popup(g_hInstance, hMainWnd, msView.GetImageList());
}

///////////////////////////////////////////////////
// Bookmark related members
///////////////////////////////////////////////////

void MainFrame::OnBookMarkAdd(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// get note's path
	TString sURI;
	if (!msView.GetURI(&sURI)) return;

	// add to bookmark manager
	const BookMarkItem *pItem = pBookMark->Assign(sURI.Get());
	if (pItem == NULL) return;

	AppendBookMark(pPlatform->GetMSBookMarkMenu(), pItem);
}

void MainFrame::OnBookMarkConfig(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BookMarkDlg dlg;
	if (!dlg.Init(pBookMark)) return;

	// release current bookmark
	const BookMarkItem *p;
	HMENU hBookMark = pPlatform->GetMSBookMarkMenu();
	DWORD n = pBookMark->NumItems();
	for (DWORD i = 0; i < n; i++) {
		p = pBookMark->GetUnit(i);
		DeleteMenu(hBookMark, p->nID, MF_BYCOMMAND);
	}

	// popup dialog and get info
	dlg.Popup(g_hInstance, hMainWnd);

	// set bookmarks
	LPTSTR pBM = pBookMark->ExportToMultiSZ();
	LoadBookMark(pBM);
	delete [] pBM;
}

void MainFrame::OnBookMark(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const BookMarkItem *pItem = pBookMark->Find(LOWORD(wParam));
	if (pItem) {
		msView.ShowItemByURI(pItem->pPath);
	}
}

void MainFrame::AppendBookMark(HMENU hMenu, const BookMarkItem *pItem)
{
	// add to menu
	MENUITEMINFO mii;

	memset(&mii, 0, sizeof(mii));

	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_ID | MIIM_DATA | MIIM_STATE | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;

	mii.wID = pItem->nID;
	mii.dwTypeData = pItem->pName;
	mii.cch = _tcslen(pItem->pName);

#if defined(PLATFORM_WIN32)
	if (!InsertMenuItem(hMenu, pItem->nID - pBookMark->GetBaseID() + NUM_BOOKMARK_SUBMENU_DEFAULT, TRUE, &mii)) return;
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
	if (!AppendMenu(hMenu, MF_STRING, pItem->nID, pItem->pName)) return;
#endif
}

void MainFrame::LoadBookMark(LPCTSTR pBookMarks)
{
	const BookMarkItem *p;

	HMENU hBookMark = pPlatform->GetMSBookMarkMenu();

	// release current bookmark
	DWORD n = pBookMark->NumItems();
	for (DWORD i = 0; i < n; i++) {
		p = pBookMark->GetUnit(i);
		DeleteMenu(hBookMark, p->nID, MF_BYCOMMAND);
	}

	// load bookmark list
	pBookMark->ImportFromMultiSZ(pBookMarks);

	// add to menu
	n = pBookMark->NumItems();
	for (i = 0; i < n; i++) {
		p = pBookMark->GetUnit(i);
		AppendBookMark(hBookMark, p);
	}
}
