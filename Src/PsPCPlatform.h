#ifndef PSPCPLATFORM_H
#define PSPCPLATFORM_H
#if defined(PLATFORM_PSPC)

#define PLATFORM_TYPE PsPCPlatform

class PsPCPlatform : public PlatformLayer {
public:
	HWND hMSCmdBar;
	HWND hMDCmdBar;

	void Create(HWND hWnd, HINSTANCE hInst);

	HMENU GetMDToolMenu() { return CommandBar_GetMenu(hMDCmdBar, 0); }
	HMENU GetMSEditMenu() { return CommandBar_GetMenu(hMSCmdBar, 0); }
	HMENU GetMSBookMarkMenu() {return GetSubMenu(CommandBar_GetMenu(hMSCmdBar, 0), BOOKMARK_MENU_POS); }

	void EnableMenu(UINT uid, BOOL bEnable);
	void EnableSearchNext();

	void OpenDetailsView();
	void CloseDetailsView();

	void AdjustUserRect(RECT *r);

	static PsPCPlatform *PlatformFactory() { return new PsPCPlatform(); }
};

#endif // PLATFORM_PSPC
#endif