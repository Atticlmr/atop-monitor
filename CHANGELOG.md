# Changelog

## [v0.2.0] - 2024-04-09

### Fixed
- Fixed multi-GPU display bug where GPU 1+ were missing history charts and temperature
- Fixed GPU rendering to properly show all detected NVIDIA GPUs with complete information

### Changed
- Improved layout: switched to horizontal two-row layout
  - Row 1: CPU | Memory (side by side)
  - Row 2: GPU 0 | GPU 1 | GPU 2 | ... (all GPUs in horizontal row)
- Enhanced chart display using FTXUI's built-in `graph()` function
  - Now renders filled sparkline-style graphs (similar to htop)
  - Uses Unicode block characters (█▉▊▋▌▍▎▏) for smooth filling
  - Added linear interpolation for smoother curve transitions
- Increased default panel widths for better readability
- Improved text labels and spacing throughout the UI

### Technical Details
- Replaced Canvas-based line drawing with `ftxui::graph()` for better performance
- Graph data now uses floating-point interpolation between history points
- Fixed history tracking for each GPU device individually

---

## [v0.1.1] - 2024-04-09

### Changed
- Use Ubuntu 20.04 for CI builds to improve glibc compatibility
- Fixed install script URLs to use `master` branch

---

## [v0.1.0] - 2024-04-09

### Added
- Initial release
- Real-time CPU monitoring with usage percentage and frequency
- Memory monitoring (RAM and Swap) with trend charts
- Multi-GPU support for NVIDIA cards via NVML
- 5-second history tracking for all metrics
- Terminal-based UI using FTXUI
- Color-coded alerts (Green/Yellow/Red)
- One-line install script
