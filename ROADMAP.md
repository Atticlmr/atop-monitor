# FTXUI 系统监控应用 - 技术路线图

## 项目概述

开发一个基于FTXUI的终端系统监控工具，实时显示**CPU**、**内存**和**NVIDIA GPU**的占用率及相关详细信息。

### 核心特性
- **混合UI显示**：数字百分比 + 进度条仪表盘
- **实时刷新**：1000ms间隔
- **CPU详情**：型号、频率、核心数、占用率
- **内存详情**：总内存、已用内存、占用率、Swap使用情况
- **GPU信息**：NVIDIA GPU占用率（如可用）
- **极简交互**：纯展示，q键退出

---

## 技术栈

| 组件 | 技术 | 用途 |
|------|------|------|
| UI框架 | FTXUI (v6.1.9+) | 终端UI渲染、组件、布局 |
| 构建系统 | CMake | 跨平台构建 |
| CPU信息 | `/proc/stat` (Linux) | 读取CPU统计信息 |
| 内存信息 | `/proc/meminfo` (Linux) | 读取内存统计信息 |
| GPU信息 | NVIDIA Management Library (NVML) | 查询NVIDIA GPU状态 |
| 语言 | C++17/C++20 | 主要开发语言 |

---

## 系统架构

```
┌─────────────────────────────────────────────────────────────────┐
│                      Application (main)                          │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐            │
│  │ CPU Monitor  │ │ MemoryMonitor│ │ GPU Monitor  │            │
│  │  (Data)      │ │  (Data)      │ │  (Data)      │            │
│  └──────┬───────┘ └──────┬───────┘ └──────┬───────┘            │
│         │                │                │                     │
│         ▼                ▼                ▼                     │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐            │
│  │ /proc/stat   │ │ /proc/meminfo│ │ NVML API     │            │
│  └──────────────┘ └──────────────┘ └──────────────┘            │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                   UI Renderer (FTXUI)                     │  │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │  │
│  │  │  CPU区   │  │ 内存区   │  │  GPU区   │  │ 状态栏   │  │  │
│  │  │(进度条)  │  │(进度条)  │  │(进度条)  │  │(提示信息)│  │  │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 模块设计

### 1. 数据收集层 (Data Layer)

#### CPU监控模块 (`src/cpu_monitor.h`, `src/cpu_monitor.cpp`)
```cpp
struct CPUInfo {
    std::string model;                // CPU型号
    int core_count;                   // 核心数
    double frequency_ghz;             // 当前频率(GHz)
    std::vector<double> core_usage;   // 每个核心的占用率
    double total_usage;               // 总体占用率
};

class CPUMonitor {
public:
    CPUInfo getInfo();                // 获取当前CPU信息
    void update();                    // 更新数据
private:
    std::vector<unsigned long long> prev_idle;
    std::vector<unsigned long long> prev_total;
};
```

**实现要点：**
- 读取 `/proc/cpuinfo` 获取CPU型号和核心数
- 读取 `/proc/stat` 计算CPU占用率（两次采样计算差值）
- 读取 `/sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq` 获取频率

#### 内存监控模块 (`src/memory_monitor.h`, `src/memory_monitor.cpp`)
```cpp
struct MemoryInfo {
    // 内存信息 (单位: MB)
    unsigned long long total_mb;      // 总内存
    unsigned long long available_mb;  // 可用内存
    unsigned long long used_mb;       // 已用内存
    double usage_percent;             // 占用率(%)
    
    // Swap信息
    unsigned long long swap_total_mb; // Swap总量
    unsigned long long swap_free_mb;  // Swap空闲
    unsigned long long swap_used_mb;  // Swap已用
    double swap_usage_percent;        // Swap占用率(%)
    
    // Buffer/Cache
    unsigned long long buffers_mb;    // 缓冲区
    unsigned long long cached_mb;     // 缓存
};

class MemoryMonitor {
public:
    MemoryInfo getInfo();             // 获取当前内存信息
    void update();                    // 更新数据
private:
    unsigned long long parseMemLine(const std::string& line);
};
```

**实现要点：**
- 读取 `/proc/meminfo` 获取内存统计
- 解析 `MemTotal`, `MemAvailable`, `Buffers`, `Cached` 等字段
- 计算实际使用内存：`MemTotal - MemAvailable`
- 读取Swap信息：`SwapTotal`, `SwapFree`

**内存计算方式：**
```cpp
// 从/proc/meminfo读取的字段（单位：KB）
MemTotal:       16384000 kB    // 总内存
MemFree:         2048000 kB    // 完全空闲
MemAvailable:    8192000 kB    // 可用（包含可回收的缓存）
Buffers:          512000 kB    // 缓冲区
Cached:          4096000 kB    // 缓存
SwapTotal:       2097152 kB    // Swap总量
SwapFree:        1572864 kB    // Swap空闲

// 计算方式
used = MemTotal - MemAvailable  // 最准确的使用量
usage_percent = used / MemTotal * 100
```

#### GPU监控模块 (`src/gpu_monitor.h`, `src/gpu_monitor.cpp`)
```cpp
struct GPUInfo {
    std::string name;                 // GPU型号
    double usage_percent;             // GPU利用率(%)
    double memory_usage_percent;      // 显存使用率(%)
    double temperature;               // 温度(°C)
};

class GPUMonitor {
public:
    bool initialize();                // 初始化NVML
    bool isAvailable();               // 检查NVIDIA GPU是否可用
    GPUInfo getInfo();                // 获取GPU信息
    void shutdown();                  // 清理NVML
private:
    nvmlDevice_t device;              // NVML设备句柄
    bool available = false;
};
```

---

## UI布局设计

```
┌──────────────────────────────────────────────────────┐
│  🖥️  System Monitor                           [q]    │
├──────────────────────────────────────────────────────┤
│  CPU: Intel Core i7-9700K @ 3.60GHz                 │
│  ├─ Usage: [████████░░░░░░░░░░░░░░░░] 32%           │
│  ├─ Cores: 8      Freq: 4.20 GHz                    │
│                                                      │
│  Memory: 16.0 GB Total                              │
│  ├─ RAM:   [███████░░░░░░░░░░░░░░░░░] 45% (7.2 GB) │
│  ├─ Swap:  [██░░░░░░░░░░░░░░░░░░░░░░] 12% (0.2 GB) │
│  └─ Buff/Cache: 4.5 GB                              │
│                                                      │
│  GPU: NVIDIA GeForce RTX 2080 Ti                    │
│  ├─ Usage: [████████████████░░░░░░░░] 65%           │
│  ├─ VRAM:  [██████░░░░░░░░░░░░░░░░░] 35%            │
│  └─ Temp:  62°C                                     │
├──────────────────────────────────────────────────────┤
│  Refresh: 1.0s  |  Press 'q' to quit               │
└──────────────────────────────────────────────────────┘
```

**颜色编码：**
- 🟢 低占用 (< 50%): 绿色
- 🟡 中占用 (50-80%): 黄色
- 🔴 高占用 (> 80%): 红色

---

## 项目结构

```
atop/
├── CMakeLists.txt              # CMake配置
├── src/
│   ├── main.cpp                # 应用入口
│   ├── cpu_monitor.h/.cpp      # CPU监控
│   ├── memory_monitor.h/.cpp   # 内存监控
│   ├── gpu_monitor.h/.cpp      # GPU监控
│   └── ui/
│       ├── dashboard.h/.cpp    # 仪表盘UI
│       └── components.h/.cpp   # 可复用UI组件
└── build/                      # 构建输出
```

---

## CMake配置要点

```cmake
cmake_minimum_required(VERSION 3.14)
project(sysmon-tui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# FTXUI库 (FetchContent)
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

# 源文件
add_executable(sysmon-tui
    src/main.cpp
    src/cpu_monitor.cpp
    src/memory_monitor.cpp
    src/gpu_monitor.cpp
    src/ui/dashboard.cpp
)

target_link_libraries(sysmon-tui PRIVATE
    ftxui::component
    ftxui::dom
    ftxui::screen
    pthread
    dl
)

# 可选NVML
find_library(NVML_LIBRARY nvidia-ml 
    PATHS /usr/lib/x86_64-linux-gnu)
if(NVML_LIBRARY)
    target_link_libraries(sysmon-tui PRIVATE ${NVML_LIBRARY})
    target_compile_definitions(sysmon-tui PRIVATE HAS_NVML)
endif()
```

---

## 核心实现片段

### 内存监控实现
```cpp
#include <fstream>
#include <string>

MemoryInfo MemoryMonitor::getInfo() {
    MemoryInfo info{};
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            info.total_mb = parseValue(line) / 1024;
        } else if (line.find("MemAvailable:") == 0) {
            info.available_mb = parseValue(line) / 1024;
        } else if (line.find("Buffers:") == 0) {
            info.buffers_mb = parseValue(line) / 1024;
        } else if (line.find("Cached:") == 0) {
            info.cached_mb = parseValue(line) / 1024;
        } else if (line.find("SwapTotal:") == 0) {
            info.swap_total_mb = parseValue(line) / 1024;
        } else if (line.find("SwapFree:") == 0) {
            info.swap_free_mb = parseValue(line) / 1024;
        }
    }
    
    // 计算
    info.used_mb = info.total_mb - info.available_mb;
    info.usage_percent = 100.0 * info.used_mb / info.total_mb;
    
    info.swap_used_mb = info.swap_total_mb - info.swap_free_mb;
    info.swap_usage_percent = info.swap_total_mb > 0 
        ? 100.0 * info.swap_used_mb / info.swap_total_mb 
        : 0;
    
    return info;
}
```

### FTXUI进度条组件
```cpp
ftxui::Element renderGauge(const std::string& label, 
                           double percent,
                           const std::string& suffix = "") {
    // 根据占用率选择颜色
    ftxui::Color color = percent > 80 ? ftxui::Color::Red :
                         percent > 50 ? ftxui::Color::Yellow :
                                        ftxui::Color::Green;
    
    return ftxui::hbox({
        ftxui::text(label) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 12),
        ftxui::gauge(percent / 100.0) | ftxui::color(color) | ftxui::flex,
        ftxui::text(" " + std::to_string((int)percent) + "%" + suffix) 
            | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 8)
    });
}
```

---

## 开发路线图

### 阶段1：项目搭建 (30分钟)
- [ ] 创建项目目录结构
- [ ] 配置CMake和FTXUI依赖
- [ ] 验证编译通过

### 阶段2：数据收集层 (1小时)
- [ ] 实现`CPUMonitor` - 读取CPU信息
- [ ] 实现`MemoryMonitor` - 读取内存信息
- [ ] 实现`GPUMonitor` - 读取GPU信息（可选）
- [ ] 测试各模块数据准确性

### 阶段3：UI层开发 (1小时)
- [ ] 实现`renderGauge`进度条组件
- [ ] 设计主布局（CPU/内存/GPU分区）
- [ ] 实现颜色编码（绿/黄/红）
- [ ] 添加边框和标题样式

### 阶段4：主循环集成 (30分钟)
- [ ] 实现1000ms定时刷新
- [ ] 集成所有监控模块
- [ ] 实现q键退出
- [ ] 处理窗口大小变化

### 阶段5：测试与优化 (30分钟)
- [ ] 长时间运行稳定性测试
- [ ] 处理边界情况（无GPU、内存满等）
- [ ] 性能优化（减少闪烁）
- [ ] 代码清理和文档

---

## 参考命令

```bash
# 构建项目
mkdir build && cd build
cmake ..
make -j

# 运行
./sysmon-tui

# 后台压力测试（测试CPU/内存显示）
stress --cpu 4 --vm 2 --vm-bytes 1G --timeout 60s
```

---

## 扩展功能建议

1. **进程列表**：显示占用CPU/内存最高的进程
2. **历史图表**：使用canvas绘制CPU/内存历史曲线
3. **网络监控**：显示上传/下载速度
4. **磁盘IO**：显示磁盘读写速率
5. **配置文件**：支持自定义刷新频率、显示选项

---

*生成时间: 2026-04-09*
