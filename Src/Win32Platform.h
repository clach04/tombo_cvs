#ifndef WIN32PLATFORM_H
#define WIN32PLATFORM_H
#if defined(PLATFORM_WIN32)

#define PLATFORM_TYPE Win32Platform

class Win32Platform : public PlatformLayer {
protected:
	void ControlMenu(BOOL bTreeActive);
	void ControlToolbar(BOOL bTreeActive);

public:
	HWND hRebar;
	HWND hToolBar;

	void Create(HWND hWnd, HINSTANCE hInst);

	HWND GetMainToolBar() { return hToolBar; }

	HMENU GetMainMenu() { return GetMenu(hMainWnd); }
	HMENU GetMDToolMenu() { return GetMainMenu(); }
	HMENU GetMSEditMenu() { return GetMainMenu(); }
	HMENU GetMSBookMarkMenu() { return GetSubMenu(GetMainMenu(), BOOKMARK_MENU_POS); }

	void EnableMenu(UINT uid, BOOL bEnable);


	void OpenDetailsView();
	void CloseDetailsView();

	void EnableSearchNext();

	void AdjustUserRect(RECT *r);

	static Win32Platform *PlatformFactory() { return new Win32Platform(); }
};

#endif // PLATFORM_WIN32
#endif