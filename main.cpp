#include "Cheat/FrameCore.h"
#include "Framework/Overlay/Overlay.h"

Overlay*	overlay = new Overlay;
CFramework* cheat = new CFramework;

// DEBUG MODE
#if _DEBUG
int main()
#else 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	// DPI
	ImGui_ImplWin32_EnableDpiAwareness();

	// Perform initialization based on the Apex window
	if (!m.AttachProcess("grcWindow", WINDOW_CLASS))
		return 1;

	m.GetBaseAddress("None.", "citizen-playernames-five.dll");

	// Overlay
	if (!overlay->InitOverlay("grcWindow", WINDOW_CLASS))
		return 2;

	// Load The Cheat
	if (!cheat->Init())
		return 3;

	overlay->OverlayLoop();
	overlay->DestroyOverlay();
	m.DetachProcess();
	g.Run = false;
	delete cheat, overlay;

	return 0;
}

void Overlay::OverlayUserFunction() // The function that will be called in the overlay loop
{
	cheat->MiscAll();

	cheat->RenderInfo();

	if (g.ESP)
		cheat->RenderESP();

	if (g.ShowMenu)
		cheat->RenderMenu();
}