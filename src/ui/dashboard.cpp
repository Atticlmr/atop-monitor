#include "dashboard.h"
#include <iomanip>
#include <sstream>
#include <ftxui/dom/canvas.hpp>

Dashboard::Dashboard(CPUMonitor& cpu_monitor, MemoryMonitor& mem_monitor, GPUMonitor& gpu_monitor)
    : cpu_monitor_(cpu_monitor), mem_monitor_(mem_monitor), gpu_monitor_(gpu_monitor) {}

ftxui::Element Dashboard::renderChart(const std::string& label, const std::deque<double>& history) {
    if (history.empty()) {
        return ftxui::text(label + ": No data");
    }
    
    int width = 50;
    int height = 10;
    
    ftxui::Canvas canvas(width, height);
    
    double max_val = 100.0;
    double min_val = 0.0;
    
    auto it = history.begin();
    int x = 0;
    int prev_x = 0;
    int prev_y = height - 1;
    
    while (it != history.end() && x < width) {
        int y = height - 1 - static_cast<int>((*it / max_val) * (height - 1));
        y = std::max(0, std::min(height - 1, y));
        
        if (x > 0) {
            canvas.DrawPointLine(prev_x, prev_y, x, y, ftxui::Color::Green);
        }
        canvas.DrawPoint(x, y, true, ftxui::Color::Green);
        
        prev_x = x;
        prev_y = y;
        ++x;
        ++it;
    }
    
    return ftxui::vbox({
        ftxui::text(label) | ftxui::bold,
        ftxui::canvas(std::move(canvas))
    });
}

ftxui::Element Dashboard::renderGauge(const std::string& label, double percent,
                                       const std::string& suffix) {
    ftxui::Color color;
    if (percent > 80.0) {
        color = ftxui::Color::Red;
    } else if (percent > 50.0) {
        color = ftxui::Color::Yellow;
    } else {
        color = ftxui::Color::Green;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << percent << "%" << suffix;
    
    return ftxui::hbox({
        ftxui::text(label) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 12),
        ftxui::gauge(percent / 100.0) | ftxui::color(color) | ftxui::flex,
        ftxui::text(" " + oss.str()) | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 10)
    });
}

ftxui::Element Dashboard::renderCPU() {
    CPUInfo info = cpu_monitor_.getInfo();
    
    auto elements = ftxui::Elements{
        ftxui::text("CPU: " + info.model) | ftxui::bold,
        renderGauge("Usage:", info.total_usage),
        renderChart("History", cpu_monitor_.getHistory()),
        ftxui::hbox({
            ftxui::text("Cores: " + std::to_string(info.core_count)) | ftxui::flex,
            ftxui::text("Freq: " + std::to_string(static_cast<int>(info.frequency_ghz * 1000)) + " MHz")
        })
    };
    
    return ftxui::vbox(elements) | ftxui::border;
}

ftxui::Element Dashboard::renderMemory() {
    MemoryInfo info = mem_monitor_.getInfo();
    
    std::ostringstream total_str;
    total_str << std::fixed << std::setprecision(1) << (info.total_mb / 1024.0) << " GB Total";
    
    std::ostringstream ram_suffix;
    ram_suffix << " (" << std::fixed << std::setprecision(1) << (info.used_mb / 1024.0) << " GB)";
    
    std::ostringstream swap_suffix;
    swap_suffix << " (" << std::fixed << std::setprecision(1) << (info.swap_used_mb / 1024.0) << " GB)";
    
    auto elements = ftxui::Elements{
        ftxui::text("Memory: " + total_str.str()) | ftxui::bold,
        renderGauge("RAM:", info.usage_percent, ram_suffix.str()),
        renderChart("History", mem_monitor_.getHistory()),
        renderGauge("Swap:", info.swap_usage_percent, swap_suffix.str())
    };
    
    return ftxui::vbox(elements) | ftxui::border;
}

ftxui::Element Dashboard::renderGPU() {
    GPUInfo info = gpu_monitor_.getInfo();
    
    if (info.device_count == 0) {
        return ftxui::vbox({
            ftxui::text("GPU: Not Available") | ftxui::bold,
            ftxui::text("No NVIDIA GPU detected")
        }) | ftxui::border | ftxui::dim;
    }
    
    ftxui::Elements device_elements;
    
    for (const auto& device : info.devices) {
        device_elements.push_back(ftxui::text("GPU " + std::to_string(device.device_id) + ": " + device.name) | ftxui::bold);
        device_elements.push_back(renderGauge("Usage:", device.usage_percent));
        device_elements.push_back(renderGauge("VRAM:", device.memory_usage_percent));
        device_elements.push_back(renderChart("History", gpu_monitor_.getHistory(device.device_id)));
        device_elements.push_back(ftxui::text("Temp: " + std::to_string(static_cast<int>(device.temperature)) + "C"));
        
        if (device.device_id < info.device_count - 1) {
            device_elements.push_back(ftxui::separator());
        }
    }
    
    return ftxui::vbox(device_elements) | ftxui::border;
}

ftxui::Component Dashboard::render() {
    return ftxui::Renderer([&] {
        return ftxui::vbox({
            ftxui::text(" System Monitor ") | ftxui::bold | ftxui::center,
            ftxui::separator(),
            renderCPU(),
            ftxui::separator(),
            renderMemory(),
            ftxui::separator(),
            renderGPU(),
            ftxui::separator(),
            ftxui::text("Refresh: 1.0s  |  Press 'q' to quit") | ftxui::center | ftxui::dim
        }) | ftxui::border;
    });
}