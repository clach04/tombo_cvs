#ifndef HPCPLATFORM_H
#define HPCPLATFORM_H
#if defined(PLATFORM_HPC)

#define PLATFORM_TYPE HPCPlatform

class HPCPlatform : public PlatformLayer {
protected:
	void ControlMenu(BOOL bTreeActive);
	void ControlToolbar(BOOL bTreeActive);

public:
	HWND hMSCmdBar;

	void Create(HWND hWnd, HINSTANCE hInst);

	HWND GetMainToolBar();

	HMENU GetMainMenu();
	HMENU GetMDToolMenu();
	HMENU GetMSEditMenu();
	HMENU GetMSBookMarkMenu();

	void EnableMenu(UINT uid, BOOL bEnable);
	void EnableSearchNext();

	void OpenDetailsView();
	void CloseDetailsView();

	void AdjustUserRect(RECT *r);

	static HPCPlatform *PlatformFactory() { return new HPCPlatform(); }
};

#endif // PLATFORM_HPC
#endif