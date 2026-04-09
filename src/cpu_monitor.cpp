#include "cpu_monitor.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <unistd.h>

CPUMonitor::CPUMonitor() {
    readCPUModel();
    readCoreCount();
    info_.cores.resize(info_.core_count);
    readUsage();
}

CPUMonitor::~CPUMonitor() = default;

void CPUMonitor::readCPUModel() {
    std::ifstream file("/proc/cpuinfo");
    std::string line;
    info_.model = "Unknown CPU";
    
    while (std::getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                info_.model = line.substr(pos + 2);
                size_t at_pos = info_.model.find("@");
                if (at_pos != std::string::npos) {
                    info_.model = info_.model.substr(0, at_pos - 1);
                }
                break;
            }
        }
    }
}

void CPUMonitor::readCoreCount() {
    info_.core_count = sysconf(_SC_NPROCESSORS_ONLN);
}

void CPUMonitor::readFrequency() {
    double total_freq = 0.0;
    int valid_cores = 0;
    
    for (int i = 0; i < info_.core_count; ++i) {
        std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(i) + 
                          "/cpufreq/scaling_cur_freq";
        std::ifstream file(path);
        if (file) {
            unsigned long long freq_hz;
            file >> freq_hz;
            total_freq += freq_hz / 1000000.0;
            valid_cores++;
        }
    }
    
    info_.frequency_ghz = valid_cores > 0 ? total_freq / valid_cores : 0.0;
}

void CPUMonitor::readUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::vector<unsigned long long> curr_idle;
    std::vector<unsigned long long> curr_total;
    
    while (std::getline(file, line)) {
        if (line.find("cpu") != 0) continue;
        
        std::istringstream iss(line);
        std::string cpu_label;
        iss >> cpu_label;
        
        if (cpu_label == "cpu") {
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
            iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
            
            unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
            curr_total.push_back(total);
            curr_idle.push_back(idle + iowait);
        } else if (cpu_label.substr(0, 3) == "cpu") {
            int core_id = std::stoi(cpu_label.substr(3));
            if (core_id < info_.core_count) {
                unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
                iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
                
                unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
                curr_total.push_back(total);
                curr_idle.push_back(idle + iowait);
            }
        }
    }
    
    if (prev_idle_.empty()) {
        prev_idle_ = curr_idle;
        prev_total_ = curr_total;
        info_.total_usage = 0.0;
        for (int i = 0; i < info_.core_count; ++i) {
            info_.cores[i].core_id = i;
            info_.cores[i].usage_percent = 0.0;
        }
        return;
    }
    
    if (curr_total.size() > 0 && prev_total_.size() > 0) {
        unsigned long long total_diff = curr_total[0] - prev_total_[0];
        unsigned long long idle_diff = curr_idle[0] - prev_idle_[0];
        
        if (total_diff > 0) {
            info_.total_usage = 100.0 * (1.0 - static_cast<double>(idle_diff) / total_diff);
        }
    }
    
    for (size_t i = 1; i < curr_total.size() && i <= static_cast<size_t>(info_.core_count); ++i) {
        if (i - 1 < prev_total_.size()) {
            unsigned long long total_diff = curr_total[i] - prev_total_[i];
            unsigned long long idle_diff = curr_idle[i] - prev_idle_[i];
            
            if (total_diff > 0) {
                info_.cores[i - 1].core_id = i - 1;
                info_.cores[i - 1].usage_percent = 100.0 * (1.0 - static_cast<double>(idle_diff) / total_diff);
            }
        }
    }
    
    prev_idle_ = curr_idle;
    prev_total_ = curr_total;
}

void CPUMonitor::update() {
    readUsage();
    readFrequency();
    
    history_.push_back(info_.total_usage);
    if (history_.size() > HISTORY_SIZE) {
        history_.pop_front();
    }
}

CPUInfo CPUMonitor::getInfo() const {
    return info_;
}