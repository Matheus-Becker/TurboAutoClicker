#include <windows.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "autoclicker.h"
#include "input_hook.h"
#include <string>

// Globais do DX
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// Estado compartilhado
static Autoclicker g_clicker;
static int   g_toggleVk = VK_F9;
static int   g_hotkeyId = 1;
static bool  g_holdWasDown = false;

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    switch (msg) {
    case WM_HOTKEY:
        // Hotkey global de toggle
        if ((int)wParam == g_hotkeyId) {
            if (g_clicker.holdMode.load()) {
                // Modo hold: alterna estado
                if (g_clicker.running.load()) g_clicker.Stop();
                else g_clicker.Start();
            } else {
                if (g_clicker.running.load()) g_clicker.Stop();
                else g_clicker.Start();
            }
        }
        return 0;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        if (g_pSwapChain) {
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, L"TurboClicker", nullptr };
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Turbo Clicker",
        WS_OVERLAPPEDWINDOW, 100, 100, 520, 420, nullptr, nullptr, hInstance, nullptr);

    // Init DX11 + ImGui (omito boilerplate por espaço; é padrão do imgui/examples)
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    CreateRenderTarget();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Registra hotkey global
    RegisterHotKey(hwnd, g_hotkeyId, 0, g_toggleVk);

    // Hook de captura
    InputHook::Install();
    InputHook::onCapture = [](InputAction act) { g_clicker.action = act; };

    ShowWindow(hwnd, SW_SHOWDEFAULT);

    std::string capturedName = "Nenhum";
    int repeatRate = 50, burstCount = 5, toggleKey = VK_F9;
    bool holdMode = false;

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Turbo Clicker", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize(ImVec2(500, 380));

        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "TURBO MODE");
        ImGui::Separator();

        ImGui::SliderInt("Repeat rate (ms)", &repeatRate, 1, 1000);
        g_clicker.repeatRateMs = repeatRate;

        ImGui::SliderInt("Actions por repeat", &burstCount, 1, 100);
        g_clicker.burstCount = burstCount;

        ImGui::Spacing();

        if (ImGui::Button("Capturar tecla/botao de REPETICAO")) {
            InputHook::captureMode = true;
            capturedName = "Aguardando input...";
        }
        ImGui::SameLine();
        if (g_clicker.action.type == InputAction::Type::Keyboard)
            capturedName = "Tecla: VK " + std::to_string(g_clicker.action.code);
        else if (g_clicker.action.type == InputAction::Type::Mouse)
            capturedName = "Mouse botao: " + std::to_string(g_clicker.action.code);
        ImGui::Text("%s", capturedName.c_str());

        ImGui::Spacing();
        ImGui::Text("Toggle hotkey (hex): 0x%X", toggleKey);
        ImGui::InputInt("VK code toggle", &toggleKey);
        if (ImGui::Button("Re-registrar hotkey")) {
            UnregisterHotKey(hwnd, g_hotkeyId);
            RegisterHotKey(hwnd, g_hotkeyId, 0, toggleKey);
            g_toggleVk = toggleKey;
        }

        ImGui::Spacing();
        if (ImGui::RadioButton("Toggle (apertar 1x liga, 1x desliga)", !holdMode)) holdMode = false;
        if (ImGui::RadioButton("Hold (enquanto tecla toggle pressionada)", holdMode)) holdMode = true;
        g_clicker.holdMode = holdMode;

        // Modo hold: precisa monitorar estado da tecla
        if (holdMode) {
            bool down = (GetAsyncKeyState(toggleKey) & 0x8000) != 0;
            if (down && !g_holdWasDown) g_clicker.Start();
            else if (!down && g_holdWasDown) g_clicker.Stop();
            g_holdWasDown = down;
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button, g_clicker.running.load() ? ImVec4(0.1f,0.6f,0.1f,1) : ImVec4(0.6f,0.1f,0.1f,1));
        if (ImGui::Button(g_clicker.running.load() ? "PARAR" : "INICIAR", ImVec2(-1, 50))) {
            if (g_clicker.running.load()) g_clicker.Stop();
            else g_clicker.Start();
        }
        ImGui::PopStyleColor();

        ImGui::Text("Status: %s", g_clicker.running.load() ? "ATIVO" : "parado");
        ImGui::End();

        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float[]){0.08f, 0.08f, 0.10f, 1.0f});
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    // Cleanup
    InputHook::Uninstall();
    UnregisterHotKey(hwnd, g_hotkeyId);
    g_clicker.Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_mainRenderTargetView->Release();
    g_pSwapChain->Release();
    g_pd3dDeviceContext->Release();
    g_pd3dDevice->Release();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, hInstance);
    return 0;
}