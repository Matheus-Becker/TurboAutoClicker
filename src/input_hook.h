#pragma once
#include <windows.h>
#include <functional>
#include <atomic>

// Representa qualquer input: tecla OU botão do mouse
struct InputAction {
    enum class Type { None, Keyboard, Mouse } type = Type::None;
    DWORD code = 0;          // VK code para teclado, MOUSEEVENTF_* para mouse
    bool   isExtended = false;
};

class InputHook {
public:
    static void Install();
    static void Uninstall();

    // Quando verdadeiro, o próximo input é capturado como "ação a repetir"
    static std::atomic<bool> captureMode;
    static std::function<void(InputAction)> onCapture;

private:
    static HHOOK kbHook;
    static HHOOK msHook;
    static LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);
    static LRESULT CALLBACK LowLevelMouseProc(int, WPARAM, LPARAM);
};