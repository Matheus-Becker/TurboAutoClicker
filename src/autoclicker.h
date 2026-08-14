#pragma once
#include "input_hook.h"
#include <atomic>
#include <thread>

class Autoclicker {
public:
    void Start() {
        if (!worker.joinable()) {
            stopRequested = false;
            worker = std::thread(&Autoclicker::Worker, this);
        }
        running = true;
    }

    void Stop() { running = false; }
    void Shutdown() { stopRequested = true; if (worker.joinable()) worker.join(); }

    static void SendInputAction(const InputAction&, bool down);
    static void SendBurst(const InputAction&, int count);

    std::atomic<bool>        running{false};
    std::atomic<int>         repeatRateMs{0};    // ← 0ms (sem delay!)
    std::atomic<int>         burstCount{5};
    InputAction              action{};
    std::atomic<bool>        holdMode{false};

private:
    void Worker();
    std::thread worker;
    std::atomic<bool> stopRequested{false};
};