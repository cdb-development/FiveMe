#pragma once
#include "..\ImGui\imgui.h"
#include "..\ImGui\imgui_impl_win32.h"
#include "..\ImGui\imgui_impl_dx11.h"
#include "..\Utils\Utils.h";
#include <dwmapi.h>
#include <thread>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

class Overlay
{
private:
	// Information to register in the overlay
	WNDCLASSEXA wc{};
	HWND Hwnd;
	char Title[32] = "The Overlay";
	char Class[32] = "WND_CLS";

	HWND TargetHwnd;
	bool CreateOverlay();
	HWND GetTargetWindow(const std::string processName);	// Obtaining a window handle from an executable file name
public:
	void OverlayUserFunction();
	bool InitOverlay(const char* targetName, int mode);
	void OverlayLoop();
	void DestroyOverlay();
	void OverlayManager(const char* targetName);
};

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_mainRenderTargetView;