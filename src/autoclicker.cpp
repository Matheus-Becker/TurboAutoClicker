#include "autoclicker.h"
#include <windows.h>
#include <thread>
#include <chrono>

// NOTA: A função mouse_event e as constantes MOUSEEVENTF_* já são 
// fornecidas nativamente pelo <windows.h> (winuser.h). 
// As redeclarações manuais foram removidas para evitar conflitos.

void Autoclicker::SendInputAction(const InputAction& action, bool down) {
    if (action.type == InputAction::Type::Mouse) {
        // Usa mouse_event direto (mais rápido)
        unsigned long flags = 0;
        switch (action.code) {
            case 1: flags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP; break;
            case 2: flags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP; break;
            case 3: flags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP; break;
        }
        mouse_event(flags, 0, 0, 0, 0);
    } else if (action.type == InputAction::Type::Keyboard) {
        // Para teclado, usa SendInput normal
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = (WORD)action.code;
        input.ki.wScan = (WORD)MapVirtualKeyW(action.code, MAPVK_VK_TO_VSC);
        input.ki.dwFlags = down ? (action.isExtended ? KEYEVENTF_EXTENDEDKEY : 0)
                                : (KEYEVENTF_KEYUP | (action.isExtended ? KEYEVENTF_EXTENDEDKEY : 0));
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;
        SendInput(1, &input, sizeof(INPUT));
    }
}

void Autoclicker::SendBurst(const InputAction& action, int count) {
    if (action.type == InputAction::Type::Mouse) {
        // mouse_event loop (igual ao PowerShell!)
        unsigned long downFlag = 0, upFlag = 0;
        switch (action.code) {
            case 1: downFlag = MOUSEEVENTF_LEFTDOWN; upFlag = MOUSEEVENTF_LEFTUP; break;
            case 2: downFlag = MOUSEEVENTF_RIGHTDOWN; upFlag = MOUSEEVENTF_RIGHTUP; break;
            case 3: downFlag = MOUSEEVENTF_MIDDLEDOWN; upFlag = MOUSEEVENTF_MIDDLEUP; break;
        }
        
        for (int i = 0; i < count; ++i) {
            mouse_event(downFlag, 0, 0, 0, 0);
            mouse_event(upFlag, 0, 0, 0, 0);
        }
    } else {
        // Teclado: pré-calcula scan code UMA VEZ
        INPUT inputs[100];  // Array estático (sem alocação dinâmica)
        int idx = 0;
        
        WORD vk = (WORD)action.code;
        WORD scan = (WORD)MapVirtualKeyW(action.code, MAPVK_VK_TO_VSC);
        DWORD extendedFlag = action.isExtended ? KEYEVENTF_EXTENDEDKEY : 0;
        
        for (int i = 0; i < count && idx < 100; ++i) {
            // DOWN
            inputs[idx].type = INPUT_KEYBOARD;
            inputs[idx].ki.wVk = vk;
            inputs[idx].ki.wScan = scan;
            inputs[idx].ki.dwFlags = extendedFlag;
            inputs[idx].ki.time = 0;
            inputs[idx].ki.dwExtraInfo = 0;
            idx++;
            
            // UP
            inputs[idx].type = INPUT_KEYBOARD;
            inputs[idx].ki.wVk = vk;
            inputs[idx].ki.wScan = scan;
            inputs[idx].ki.dwFlags = KEYEVENTF_KEYUP | extendedFlag;
            inputs[idx].ki.time = 0;
            inputs[idx].ki.dwExtraInfo = 0;
            idx++;
        }
        
        SendInput(idx, inputs, sizeof(INPUT));
    }
}

void Autoclicker::Worker() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    
    while (!stopRequested.load()) {
        if (!running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        SendBurst(action, burstCount.load());

        auto rate = repeatRateMs.load();
        if (rate > 0) {
            // Busy-wait para precisão extrema (não usa sleep)
            auto start = std::chrono::high_resolution_clock::now();
            while (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start).count() < rate) {
                // Spin wait (gasta CPU mas é preciso)
            }
        }
    }
}