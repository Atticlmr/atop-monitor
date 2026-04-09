#include "memory_monitor.h"
#include <fstream>
#include <sstream>
#include <iostream>

MemoryMonitor::MemoryMonitor() {
    update();
}

MemoryMonitor::~MemoryMonitor() = default;

unsigned long long MemoryMonitor::parseMemLine(const std::string& line) {
    std::istringstream iss(line);
    std::string label;
    unsigned long long value;
    std::string unit;
    
    iss >> label >> value >> unit;
    return value;
}

void MemoryMonitor::update() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    
    info_.total_mb = 0;
    info_.available_mb = 0;
    info_.buffers_mb = 0;
    info_.cached_mb = 0;
    info_.swap_total_mb = 0;
    info_.swap_free_mb = 0;
    
    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            info_.total_mb = parseMemLine(line) / 1024;
        } else if (line.find("MemAvailable:") == 0) {
            info_.available_mb = parseMemLine(line) / 1024;
        } else if (line.find("Buffers:") == 0) {
            info_.buffers_mb = parseMemLine(line) / 1024;
        } else if (line.find("Cached:") == 0) {
            info_.cached_mb = parseMemLine(line) / 1024;
        } else if (line.find("SwapTotal:") == 0) {
            info_.swap_total_mb = parseMemLine(line) / 1024;
        } else if (line.find("SwapFree:") == 0) {
            info_.swap_free_mb = parseMemLine(line) / 1024;
        }
    }
    
    info_.used_mb = info_.total_mb - info_.available_mb;
    
    if (info_.total_mb > 0) {
        info_.usage_percent = 100.0 * static_cast<double>(info_.used_mb) / info_.total_mb;
    } else {
        info_.usage_percent = 0.0;
    }
    
    info_.swap_used_mb = info_.swap_total_mb - info_.swap_free_mb;
    
    if (info_.swap_total_mb > 0) {
        info_.swap_usage_percent = 100.0 * static_cast<double>(info_.swap_used_mb) / info_.swap_total_mb;
    } else {
        info_.swap_usage_percent = 0.0;
    }
    
    history_.push_back(info_.usage_percent);
    if (history_.size() > HISTORY_SIZE) {
        history_.pop_front();
    }
}

MemoryInfo MemoryMonitor::getInfo() const {
    return info_;
}
