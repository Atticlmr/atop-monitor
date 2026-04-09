#include "dashboard.h"
#include <iomanip>
#include <sstream>
#include <functional>
#include <algorithm>

Dashboard::Dashboard(CPUMonitor& cpu_monitor, MemoryMonitor& mem_monitor, GPUMonitor& gpu_monitor)
    : cpu_monitor_(cpu_monitor), mem_monitor_(mem_monitor), gpu_monitor_(gpu_monitor) {}

ftxui::Element Dashboard::renderChart(const std::string& label, const std::deque<double>& history) {
    if (history.empty()) {
        return ftxui::text("") | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 6);
    }
    
    auto graph_func = [history](int width, int height) -> std::vector<int> {
        std::vector<int> output(width);
        if (history.empty() || width <= 0 || height <= 0) {
            return output;
        }
        
        size_t history_size = history.size();
        for (int i = 0; i < width; ++i) {
            float idx = (static_cast<float>(i) * history_size) / width;
            size_t idx_low = static_cast<size_t>(idx);
            size_t idx_high = std::min(idx_low + 1, history_size - 1);
            
            float frac = idx - idx_low;
            float value = history[idx_low] * (1 - frac) + history[idx_high] * frac;
            
            // Scale value (0-100) to height
            output[i] = static_cast<int>((value / 100.0) * height);
        }
        return output;
    };
    
    auto graph_element = ftxui::graph(graph_func) | ftxui::color(ftxui::Color::Green);
    
    if (label.empty()) {
        return graph_element | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 6);
    }
    
    return ftxui::vbox({
        ftxui::text(label) | ftxui::bold,
        graph_element
    }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 6);
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
        ftxui::text(label) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8),
        ftxui::gauge(percent / 100.0) | ftxui::color(color) | ftxui::flex,
        ftxui::text(oss.str()) | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 7)
    });
}

ftxui::Element Dashboard::renderCPU() {
    CPUInfo info = cpu_monitor_.getInfo();
    std::string model = info.model.substr(0, 30);
    
    auto elements = ftxui::Elements{
        ftxui::text("CPU: " + model) | ftxui::bold,
        renderGauge("Load", info.total_usage),
        renderChart("", cpu_monitor_.getHistory()),
        ftxui::hbox({
            ftxui::text("Cores: " + std::to_string(info.core_count)),
            ftxui::text("Freq: " + std::to_string(static_cast<int>(info.frequency_ghz * 1000)) + " MHz") | ftxui::flex
        })
    };
    
    return ftxui::vbox(elements) | ftxui::border | ftxui::flex;
}

ftxui::Element Dashboard::renderMemory() {
    MemoryInfo info = mem_monitor_.getInfo();
    
    auto elements = ftxui::Elements{
        ftxui::text("RAM: " + std::to_string(static_cast<int>(info.total_mb / 1024)) + " GB Total") | ftxui::bold,
        renderGauge("Used", info.usage_percent),
        renderChart("", mem_monitor_.getHistory()),
        renderGauge("Swap", info.swap_usage_percent)
    };
    
    return ftxui::vbox(elements) | ftxui::border | ftxui::flex;
}

ftxui::Element Dashboard::renderGPU(int device_id) {
    GPUInfo info = gpu_monitor_.getInfo();
    
    if (device_id >= info.device_count) {
        return ftxui::text("") | ftxui::flex;
    }
    
    const auto& device = info.devices[device_id];
    std::string short_name = device.name;
    if (short_name.length() > 25) {
        short_name = short_name.substr(0, 25);
    }
    
    auto elements = ftxui::Elements{
        ftxui::text("GPU " + std::to_string(device.device_id) + ": " + short_name) | ftxui::bold,
        renderGauge("GPU", device.usage_percent),
        renderGauge("VRAM", device.memory_usage_percent),
        renderChart("", gpu_monitor_.getHistory(device.device_id)),
        ftxui::text("Temp: " + std::to_string(static_cast<int>(device.temperature)) + "°C")
    };
    
    return ftxui::vbox(elements) | ftxui::border | ftxui::flex;
}

ftxui::Component Dashboard::render() {
    return ftxui::Renderer([&] {
        GPUInfo gpu_info = gpu_monitor_.getInfo();
        
        ftxui::Elements row1;
        row1.push_back(renderCPU());
        row1.push_back(ftxui::separator() | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1));
        row1.push_back(renderMemory());
        
        ftxui::Elements row2;
        for (int i = 0; i < gpu_info.device_count; ++i) {
            if (i > 0) {
                row2.push_back(ftxui::separator() | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1));
            }
            row2.push_back(renderGPU(i));
        }
        
        ftxui::Elements main_content;
        main_content.push_back(ftxui::hbox(row1) | ftxui::flex);
        
        if (gpu_info.device_count > 0) {
            main_content.push_back(ftxui::separator());
            main_content.push_back(ftxui::hbox(row2) | ftxui::flex);
        }
        
        return ftxui::vbox({
            ftxui::text(" System Monitor ") | ftxui::bold | ftxui::center,
            ftxui::separator(),
            ftxui::vbox(main_content) | ftxui::flex,
            ftxui::separator(),
            ftxui::text("Refresh: 1.0s | Press 'q' to quit") | ftxui::center | ftxui::dim
        }) | ftxui::border;
    });
}