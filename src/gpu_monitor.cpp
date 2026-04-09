#include "gpu_monitor.h"
#include <dlfcn.h>
#include <cstring>

struct nvmlUtilization_st {
    unsigned int gpu;
    unsigned int memory;
};

GPUMonitor::GPUMonitor() : available_(false), nvml_handle_(nullptr) {
    nvmlInit_ = nullptr;
    nvmlShutdown_ = nullptr;
    nvmlDeviceGetCount_ = nullptr;
    nvmlDeviceGetHandleByIndex_ = nullptr;
    nvmlDeviceGetName_ = nullptr;
    nvmlDeviceGetUtilizationRates_ = nullptr;
    nvmlDeviceGetTemperature_ = nullptr;
}

GPUMonitor::~GPUMonitor() {
    shutdown();
}

bool GPUMonitor::initialize() {
    nvml_handle_ = dlopen("libnvidia-ml.so.1", RTLD_LAZY);
    if (!nvml_handle_) {
        nvml_handle_ = dlopen("libnvidia-ml.so", RTLD_LAZY);
    }
    
    if (!nvml_handle_) {
        return false;
    }
    
    nvmlInit_ = (nvmlInit_t)dlsym(nvml_handle_, "nvmlInit_v2");
    if (!nvmlInit_) nvmlInit_ = (nvmlInit_t)dlsym(nvml_handle_, "nvmlInit");
    
    nvmlShutdown_ = (nvmlShutdown_t)dlsym(nvml_handle_, "nvmlShutdown");
    nvmlDeviceGetCount_ = (nvmlDeviceGetCount_t)dlsym(nvml_handle_, "nvmlDeviceGetCount");
    nvmlDeviceGetHandleByIndex_ = (nvmlDeviceGetHandleByIndex_t)dlsym(nvml_handle_, "nvmlDeviceGetHandleByIndex_v2");
    if (!nvmlDeviceGetHandleByIndex_) {
        nvmlDeviceGetHandleByIndex_ = (nvmlDeviceGetHandleByIndex_t)dlsym(nvml_handle_, "nvmlDeviceGetHandleByIndex");
    }
    nvmlDeviceGetName_ = (nvmlDeviceGetName_t)dlsym(nvml_handle_, "nvmlDeviceGetName");
    nvmlDeviceGetUtilizationRates_ = (nvmlDeviceGetUtilizationRates_t)dlsym(nvml_handle_, "nvmlDeviceGetUtilizationRates");
    nvmlDeviceGetTemperature_ = (nvmlDeviceGetTemperature_t)dlsym(nvml_handle_, "nvmlDeviceGetTemperature");
    
    if (!nvmlInit_ || !nvmlShutdown_ || !nvmlDeviceGetCount_ || 
        !nvmlDeviceGetHandleByIndex_ || !nvmlDeviceGetName_ ||
        !nvmlDeviceGetUtilizationRates_ || !nvmlDeviceGetTemperature_) {
        dlclose(nvml_handle_);
        nvml_handle_ = nullptr;
        return false;
    }
    
    if (nvmlInit_() != 0) {
        dlclose(nvml_handle_);
        nvml_handle_ = nullptr;
        return false;
    }
    
    unsigned int device_count = 0;
    if (nvmlDeviceGetCount_(&device_count) != 0 || device_count == 0) {
        nvmlShutdown_();
        dlclose(nvml_handle_);
        nvml_handle_ = nullptr;
        return false;
    }
    
    for (unsigned int i = 0; i < device_count; ++i) {
        void* device_handle = nullptr;
        if (nvmlDeviceGetHandleByIndex_(i, &device_handle) == 0) {
            device_handles_.push_back(device_handle);
            device_histories_.emplace_back();
        }
    }
    
    available_ = !device_handles_.empty();
    return available_;
}

bool GPUMonitor::isAvailable() const {
    return available_;
}

void GPUMonitor::shutdown() {
    if (nvml_handle_) {
        if (nvmlShutdown_) {
            nvmlShutdown_();
        }
        dlclose(nvml_handle_);
        nvml_handle_ = nullptr;
    }
    device_handles_.clear();
    device_histories_.clear();
    available_ = false;
}

GPUInfo GPUMonitor::getInfo() {
    GPUInfo info;
    info.device_count = static_cast<int>(device_handles_.size());
    
    for (size_t i = 0; i < device_handles_.size(); ++i) {
        GPUDeviceInfo device_info;
        device_info.device_id = static_cast<int>(i);
        
        char name[256];
        if (nvmlDeviceGetName_(device_handles_[i], name, sizeof(name)) == 0) {
            device_info.name = name;
        } else {
            device_info.name = "Unknown GPU";
        }
        
        nvmlUtilization_st utilization;
        if (nvmlDeviceGetUtilizationRates_(device_handles_[i], &utilization) == 0) {
            device_info.usage_percent = utilization.gpu;
            device_info.memory_usage_percent = utilization.memory;
        } else {
            device_info.usage_percent = 0.0;
            device_info.memory_usage_percent = 0.0;
        }
        
        unsigned int temp;
        if (nvmlDeviceGetTemperature_(device_handles_[i], 0, &temp) == 0) {
            device_info.temperature = temp;
        } else {
            device_info.temperature = 0.0;
        }
        
        info.devices.push_back(device_info);
        
        if (i < device_histories_.size()) {
            device_histories_[i].push_back(device_info.usage_percent);
            if (device_histories_[i].size() > HISTORY_SIZE) {
                device_histories_[i].pop_front();
            }
        }
    }
    
    return info;
}

const std::deque<double>& GPUMonitor::getHistory(int device_id) const {
    if (device_id >= 0 && device_id < static_cast<int>(device_histories_.size())) {
        return device_histories_[device_id];
    }
    static std::deque<double> empty;
    return empty;
}
