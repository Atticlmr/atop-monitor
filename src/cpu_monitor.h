#pragma once

#include <string>
#include <vector>
#include <deque>

struct CPUCoreInfo {
    int core_id;
    double usage_percent;
};

struct CPUInfo {
    std::string model;
    int core_count;
    double frequency_ghz;
    double total_usage;
    std::vector<CPUCoreInfo> cores;
};

class CPUMonitor {
public:
    static constexpr int HISTORY_SIZE = 50;
    
    CPUMonitor();
    ~CPUMonitor();
    
    void update();
    CPUInfo getInfo() const;
    const std::deque<double>& getHistory() const { return history_; }
    
private:
    CPUInfo info_;
    std::vector<unsigned long long> prev_idle_;
    std::vector<unsigned long long> prev_total_;
    std::deque<double> history_;
    
    void readCPUModel();
    void readCoreCount();
    void readFrequency();
    void readUsage();
};
