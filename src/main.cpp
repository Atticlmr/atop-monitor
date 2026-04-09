#include <thread>
#include <chrono>
#include <atomic>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include "cpu_monitor.h"
#include "memory_monitor.h"
#include "gpu_monitor.h"
#include "ui/dashboard.h"

int main() {
    CPUMonitor cpu_monitor;
    MemoryMonitor mem_monitor;
    GPUMonitor gpu_monitor;
    
    gpu_monitor.initialize();
    
    std::atomic<bool> running{true};
    
    auto screen = ftxui::ScreenInteractive::FitComponent();
    Dashboard dashboard(cpu_monitor, mem_monitor, gpu_monitor);
    
    auto renderer = dashboard.render();
    
    auto component = renderer | ftxui::CatchEvent([&](ftxui::Event event) {
        if (event == ftxui::Event::Character('q')) {
            running = false;
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });
    
    std::thread refresh_thread([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (running) {
                cpu_monitor.update();
                mem_monitor.update();
                screen.Post(ftxui::Event::Custom);
            }
        }
    });
    
    screen.Loop(component);
    
    if (refresh_thread.joinable()) {
        refresh_thread.join();
    }
    
    gpu_monitor.shutdown();
    
    return 0;
}
