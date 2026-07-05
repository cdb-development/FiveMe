#include "Overlay.h"
#include <TlHelp32.h>

void LoadStyle();
LONG MenuStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
LONG ESPStyle = WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
bool CreateDeviceD3D(HWND hWnd);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool Overlay::CreateOverlay()
{
    // Get ClientSize
    GetClientRect(g.GameHwnd, &g.GameRect);
    ClientToScreen(g.GameHwnd, &g.GamePoint);

    // Create Overlay
    wc = { sizeof(WNDCLASSEXA), 0, WndProc, 0, 0, NULL, NULL, NULL, NULL, Title, Class, NULL};
    RegisterClassExA(&wc);
    Hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, wc.lpszClassName, wc.lpszMenuName, WS_POPUP | WS_VISIBLE, g.GamePoint.x, g.GamePoint.y, g.GameRect.right, g.GameRect.bottom, NULL, NULL, wc.hInstance, NULL);
    SetLayeredWindowAttributes(Hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(Hwnd, &margin);

    if (!CreateDeviceD3D(Hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        
        return false;
    }

    ShowWindow(Hwnd, SW_SHOWDEFAULT);
    UpdateWindow(Hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    LoadStyle();
    ImGui_ImplWin32_Init(Hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
}

void Overlay::OverlayLoop()
{
    while (g.Run)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        OverlayManager("grcWindow");

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        OverlayUserFunction();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }
}

void Overlay::DestroyOverlay()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(Hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);
}

bool Overlay::InitOverlay(const char* targetName, int mode)
{
    if (mode == WINDOW_TITLE || mode == WINDOW_CLASS) {
        TargetHwnd = WINDOW_TITLE ? FindWindowA(nullptr, targetName) : FindWindowA(targetName, nullptr);

        if (!TargetHwnd)
            return false;
    }
    else if (mode == PROCESS) {
        TargetHwnd = GetTargetWindow(targetName);

        if (!TargetHwnd)
            return false;
    }
    else {
        return false;
    }

    return CreateOverlay();
}

// Adjust the size and position of the overlay to match the target window.
void Overlay::OverlayManager(const char* targetName)
{
    // Window Check
    HWND CheckHwnd = FindWindowA(targetName, nullptr);
    if (!CheckHwnd) {
        g.Run = false;
        return;
    }

    // StreamProof
    DWORD Flag = NULL;
    GetWindowDisplayAffinity(Hwnd, &Flag);
    if (g.StreamProof && Flag == WDA_NONE)
        SetWindowDisplayAffinity(Hwnd, WDA_EXCLUDEFROMCAPTURE);
    else if (!g.StreamProof && Flag == WDA_EXCLUDEFROMCAPTURE)
        SetWindowDisplayAffinity(Hwnd, WDA_NONE);

    // Window Style Changer
    HWND ForegroundWindow = GetForegroundWindow();
    LONG TmpLong = GetWindowLong(Hwnd, GWL_EXSTYLE);

    if (g.ShowMenu && MenuStyle != TmpLong)
        SetWindowLong(Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    else if (!g.ShowMenu && ESPStyle != TmpLong)
        SetWindowLong(Hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

    // ShowMenu
    static bool menu_key = false;
    if (IsKeyDown(g.MenuKey) && !menu_key)
    {
        g.ShowMenu = !g.ShowMenu;

        if (g.ShowMenu && ForegroundWindow != Hwnd)
            SetForegroundWindow(Hwnd);
        else if (!g.ShowMenu && ForegroundWindow != CheckHwnd)
            SetForegroundWindow(CheckHwnd);

        menu_key = true;
    }
    else if (!IsKeyDown(g.MenuKey) && menu_key)
    {
        menu_key = false;
    }

    // Window Resizer
    RECT TmpRect{};
    POINT TmpPoint{};
    GetClientRect(CheckHwnd, &TmpRect);
    ClientToScreen(CheckHwnd, &TmpPoint);

    // Resizer
    if (TmpRect.left != g.GameRect.left || TmpRect.bottom != g.GameRect.bottom || TmpRect.top != g.GameRect.top || TmpRect.right != g.GameRect.right || TmpPoint.x != g.GamePoint.x || TmpPoint.y != g.GamePoint.y)
    {
        g.GameRect = TmpRect;
        g.GamePoint = TmpPoint;
        SetWindowPos(Hwnd, nullptr, TmpPoint.x, TmpPoint.y, g.GameRect.right, g.GameRect.bottom, SWP_NOREDRAW);
    }
}

HWND Overlay::GetTargetWindow(const std::string processName)
{
    DWORD PID = NULL;
    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);
    const auto snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    while (Process32Next(snapShot, &entry))
    {
        if (!processName.compare(entry.szExeFile))
        {
            PID = entry.th32ProcessID;
            break;
        }
    }

    CloseHandle(snapShot);

    HWND hwnd = GetTopWindow(NULL);
    do {
        if (GetWindowLong(hwnd, 8) != 0 || !IsWindowVisible(hwnd))
            continue;
        DWORD ProcessID;
        GetWindowThreadProcessId(hwnd, &ProcessID);
        if (PID == ProcessID)
            return hwnd;
    } while ((hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) != NULL);

    return NULL;
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// Imgui Style
void LoadStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Border
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 1.f;
    style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 0.f;
    style.TabBorderSize = 1.f;
    style.TabBarBorderSize = 0.f;

    // Rounding
    style.WindowRounding = 0.f;
    style.ChildRounding = 0.f;
    style.FrameRounding = 0.f;
    style.PopupRounding = 0.f;
    style.TabRounding = 0.f;

    // Sizing
    style.ScrollbarSize = 3.f;
    style.GrabMinSize = 5.f;
    style.SeparatorTextBorderSize = 1.f;

    // Alignment
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

    // Padding
    style.WindowPadding = ImVec2(8.f, 8.f);
    style.FramePadding = ImVec2(4.f, 3.f);
    style.ItemSpacing = ImVec2(8.f, 4.f);
    style.ItemInnerSpacing = ImVec2(4.f, 4.f);
    style.CellPadding = ImVec2(4.f, 2.f);
    style.IndentSpacing = 21.f;
    style.ScrollbarRounding = 0.f;
    style.GrabRounding = 0.f;
    style.TabRounding = 0.f;
    style.WindowRounding = 0.f;
    style.ChildRounding = 0.f;
    style.FrameRounding = 0.f;
    style.PopupRounding = 0.f;
    style.ScrollbarRounding = 0.f;
    style.GrabRounding = 0.f;
    style.TabRounding = 0.f;

    // Display
    style.DisplayWindowPadding = ImVec2(0.f, 0.f);
    style.DisplaySafeAreaPadding = ImVec2(50.f, 50.f);

    // ===== COLORS =====
    ImVec4* colors = style.Colors;

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (Input, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);

    // Title Bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);

    // Menu Bar
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);

    // Checkbox / Radio
    colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);

    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.78f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.35f, 0.78f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // Headers (TreeNode, etc.)
    colors[ImGuiCol_Header] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);

    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.45f, 0.45f, 0.81f, 0.50f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.13f, 0.13f, 0.97f);
    colors[ImGuiCol_TabActive] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.45f, 0.45f, 0.81f, 0.50f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.45f, 0.45f, 0.81f, 0.35f);


    // Plot Lines
    colors[ImGuiCol_PlotLines] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.60f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.60f, 0.60f, 0.90f, 1.00f);

    // Table
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
}