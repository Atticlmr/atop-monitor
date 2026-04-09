# atop-monitor

基于 [FTXUI](https://github.com/ArthurSonzogni/FTXUI) 构建的终端系统监控工具。实时监控 CPU、内存和 NVIDIA GPU 使用率，并附带趋势图表。

![许可证](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)
![平台](https://img.shields.io/badge/platform-Linux-green.svg)

## 特性

- 📊 **实时 CPU 监控**
  - CPU 使用率百分比
  - CPU 频率
  - 多核心支持
  - 5秒趋势图

- 💾 **内存追踪**
  - RAM 使用率和总量
  - Swap 使用情况
  - 缓存/缓冲区信息
  - 5秒趋势图

- 🎮 **多 GPU 支持**
  - 通过 NVML 监控 NVIDIA GPU
  - 支持多块 GPU
  - GPU 使用率和显存
  - 温度监控
  - 每块 GPU 独立的趋势图

- 🎨 **美观的终端界面**
  - 带颜色编码的进度条仪表盘
  - 实时趋势图表
  - 颜色警报（绿/黄/红）
  - 简洁的边框布局

## 截图

![screenshot](./screenshot.png)

## 一键安装

```bash
curl -sSL https://raw.githubusercontent.com/Atticlmr/atop-monitor/master/install.sh | bash
```

或者下载预编译的 release：

| 架构 | 下载链接 |
|------|---------|
| x86_64 (amd64) | [atop-monitor-linux-amd64](https://github.com/Atticlmr/atop-monitor/releases/latest) |
| aarch64 (arm64) | [atop-monitor-linux-arm64](https://github.com/Atticlmr/atop-monitor/releases/latest) |

下载后：
```bash
chmod +x atop-monitor-linux-amd64
sudo mv atop-monitor-linux-amd64 /usr/local/bin/atop-monitor
```

## 从源码构建

### 前置要求

- Linux 操作系统
- C++17 兼容编译器 (GCC 7+ 或 Clang 5+)
- CMake 3.14+
- NVIDIA 显卡驱动（可选，用于 GPU 监控）

### 构建步骤

1. 克隆仓库：
```bash
git clone https://github.com/Atticlmr/atop-monitor.git
cd atop-monitor
```

2. 构建项目：
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

3. 安装到系统：
```bash
sudo make install
```

### 使用 Makefile（更简便）

```bash
make        # 构建
sudo make install   # 安装
make run    # 构建并运行
make clean  # 清理
```

## 使用方法

安装后，在终端直接运行：
```bash
atop-monitor
```

按 `q` 退出。

## 快捷键

| 按键 | 功能 |
|------|------|
| `q` | 退出应用 |

## 项目结构

```
atop-monitor/
├── CMakeLists.txt          # CMake 配置
├── Makefile               # Make 包装器
├── README.md              # 英文说明
├── README_zh.md          # 中文说明
├── install.sh             # 一键安装脚本
├── src/
│   ├── main.cpp          # 程序入口
│   ├── cpu_monitor.cpp    # CPU 监控
│   ├── memory_monitor.cpp # 内存监控
│   ├── gpu_monitor.cpp   # GPU 监控 (NVML)
│   └── ui/
│       └── dashboard.cpp  # FTXUI 仪表盘
└── .github/workflows/
    └── build.yml         # 自动构建流程
```

## 工作原理

### CPU 监控
- 从 `/proc/stat` 读取 CPU 统计信息
- 通过空闲/总时间的差值计算使用率
- 每 100ms 采样一次，每 1s 更新显示
- 维护 50 个采样点的历史记录（5 秒）

### 内存监控
- 从 `/proc/meminfo` 读取内存统计信息
- 计算实际使用量：`MemTotal - MemAvailable`
- 分别追踪 RAM 和 Swap
- 5 秒趋势历史

### GPU 监控
- 使用 NVIDIA 管理库 (NVML)
- 动态加载以避免硬依赖
- 支持多块 GPU
- 每块 GPU 独立的历史追踪

## 依赖

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - 终端 UI 库
- NVIDIA 驱动（可选）- 用于 GPU 监控

## 故障排除

### 检测不到 GPU
确保已安装 NVIDIA 驱动，且 `libnvidia-ml.so` 在库路径中。

### 构建错误
确保已安装所有依赖：
```bash
sudo apt-get install build-essential cmake
```

## 未来改进

- [ ] 网络监控
- [ ] 磁盘 I/O 统计
- [ ] 进程列表
- [ ] 配置文件支持
- [ ] 可自定义刷新率
- [ ] 导出数据到 CSV

## 贡献

欢迎提交 Pull Request！

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 致谢

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - 优秀的终端 UI 库
- NVIDIA - NVML 库

---

使用 ❤️ 和 FTXUI 构建
