#pragma once

#include <string>
#include <vector>
#include <deque>

struct GPUDeviceInfo {
    int device_id;
    std::string name;
    double usage_percent;
    double memory_usage_percent;
    double temperature;
};

struct GPUInfo {
    int device_count;
    std::vector<GPUDeviceInfo> devices;
};

class GPUMonitor {
public:
    static constexpr int HISTORY_SIZE = 50;
    
    GPUMonitor();
    ~GPUMonitor();
    
    bool initialize();
    bool isAvailable() const;
    void shutdown();
    
    GPUInfo getInfo();
    const std::deque<double>& getHistory(int device_id) const;
    
private:
    bool available_;
    void* nvml_handle_;
    std::vector<void*> device_handles_;
    std::vector<std::deque<double>> device_histories_;
    
    typedef int (*nvmlInit_t)(void);
    typedef int (*nvmlShutdown_t)(void);
    typedef int (*nvmlDeviceGetCount_t)(unsigned int*);
    typedef int (*nvmlDeviceGetHandleByIndex_t)(unsigned int, void**);
    typedef int (*nvmlDeviceGetName_t)(void*, char*, unsigned int);
    typedef int (*nvmlDeviceGetUtilizationRates_t)(void*, void*);
    typedef int (*nvmlDeviceGetTemperature_t)(void*, unsigned int, unsigned int*);
    
    nvmlInit_t nvmlInit_;
    nvmlShutdown_t nvmlShutdown_;
    nvmlDeviceGetCount_t nvmlDeviceGetCount_;
    nvmlDeviceGetHandleByIndex_t nvmlDeviceGetHandleByIndex_;
    nvmlDeviceGetName_t nvmlDeviceGetName_;
    nvmlDeviceGetUtilizationRates_t nvmlDeviceGetUtilizationRates_;
    nvmlDeviceGetTemperature_t nvmlDeviceGetTemperature_;
};
