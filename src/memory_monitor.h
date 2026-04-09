#pragma once

#include <string>
#include <deque>

struct MemoryInfo {
    unsigned long long total_mb;
    unsigned long long available_mb;
    unsigned long long used_mb;
    double usage_percent;
    
    unsigned long long swap_total_mb;
    unsigned long long swap_free_mb;
    unsigned long long swap_used_mb;
    double swap_usage_percent;
    
    unsigned long long buffers_mb;
    unsigned long long cached_mb;
};

class MemoryMonitor {
public:
    static constexpr int HISTORY_SIZE = 50;
    
    MemoryMonitor();
    ~MemoryMonitor();
    
    void update();
    MemoryInfo getInfo() const;
    const std::deque<double>& getHistory() const { return history_; }
    
private:
    MemoryInfo info_;
    std::deque<double> history_;
    
    unsigned long long parseMemLine(const std::string& line);
};
