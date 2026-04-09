#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "cpu_monitor.h"
#include "memory_monitor.h"
#include "gpu_monitor.h"

class Dashboard {
public:
    Dashboard(CPUMonitor& cpu_monitor, MemoryMonitor& mem_monitor, GPUMonitor& gpu_monitor);
    
    ftxui::Component render();
    
private:
    CPUMonitor& cpu_monitor_;
    MemoryMonitor& mem_monitor_;
    GPUMonitor& gpu_monitor_;
    
    ftxui::Element renderChart(const std::string& label, const std::deque<double>& history);
    ftxui::Element renderGauge(const std::string& label, double percent, 
                               const std::string& suffix = "");
    ftxui::Element renderCPU();
    ftxui::Element renderMemory();
    ftxui::Element renderGPU();
};
