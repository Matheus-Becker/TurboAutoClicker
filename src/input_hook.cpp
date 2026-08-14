#include "input_hook.h"
#include <iostream>

HHOOK InputHook::kbHook = nullptr;
HHOOK InputHook::msHook = nullptr;
std::atomic<bool> InputHook::captureMode{false};
std::function<void(InputAction)> InputHook::onCapture;

void InputHook::Install() {
    kbHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
    msHook = SetWindowsHookExW(WH_MOUSE_LL,    LowLevelMouseProc,    GetModuleHandle(nullptr), 0);
}

void InputHook::Uninstall() {
    if (kbHook) UnhookWindowsHookEx(kbHook);
    if (msHook) UnhookWindowsHookEx(msHook);
}

LRESULT CALLBACK InputHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && captureMode.load()) {
        KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        // Só captura no KEYDOWN, ignora shift/ctrl/alt puro
        if (wParam == WM_KEYDOWN && kb->vkCode != VK_SHIFT && kb->vkCode != VK_CONTROL && kb->vkCode != VK_MENU) {
            InputAction act{InputAction::Type::Keyboard, kb->vkCode, (kb->flags & LLKHF_EXTENDED) != 0};
            if (onCapture) onCapture(act);
            captureMode = false;
            return 1; // bloqueia o input
        }
    }
    return CallNextHookEx(kbHook, nCode, wParam, lParam);
}

LRESULT CALLBACK InputHook::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && captureMode.load()) {
        InputAction act{InputAction::Type::Mouse, 0, false};
        switch (wParam) {
            case WM_LBUTTONDOWN: act.code = 1; break;
            case WM_RBUTTONDOWN: act.code = 2; break;
            case WM_MBUTTONDOWN: act.code = 3; break;
            default: return CallNextHookEx(msHook, nCode, wParam, lParam);
        }
        if (onCapture) onCapture(act);
        captureMode = false;
        return 1;
    }
    return CallNextHookEx(msHook, nCode, wParam, lParam);
}