# Business Log Manager - 多业务日志管理器

基于 spdlog 的多业务日志管理功能，支持为不同业务类型创建独立的轮转日志文件。

## 功能特性

1. **多业务分离** - 四种业务类型各自独立的日志文件
2. **自动轮转** - 每个业务最多保留 3 个日志文件，每个文件最大 30MB
3. **线程安全** - 支持多线程并发写入
4. **级别控制** - 可为每个业务单独设置日志级别
5. **格式化支持** - 支持 spdlog 的格式化字符串语法

## 业务类型

| 业务类型 | 枚举值 | 日志文件名 |
|---------|--------|-----------|
| 录制屏幕 | `BusinessType::ScreenRecord` | `screen_record.log` |
| 打开桌面 | `BusinessType::DesktopOpen` | `desktop_open.log` |
| 录制键盘 | `BusinessType::KeyboardRecord` | `keyboard_record.log` |
| 录制声音 | `BusinessType::SoundRecord` | `sound_record.log` |

## 文件结构

```
business_logs/                    # 日志目录（可配置）
├── screen_record.log             # 当前屏幕录制日志
├── screen_record.1.log           # 轮转的历史日志 1
├── screen_record.2.log           # 轮转的历史日志 2
├── screen_record.3.log           # 轮转的历史日志 3
├── desktop_open.log
├── desktop_open.1.log
├── desktop_open.2.log
├── desktop_open.3.log
├── keyboard_record.log
├── keyboard_record.1.log
├── keyboard_record.2.log
├── keyboard_record.3.log
├── sound_record.log
├── sound_record.1.log
├── sound_record.2.log
└── sound_record.3.log
```

## 快速开始

### 1. 包含头文件

```cpp
#include <spdlog/business_log_manager.h>
```

### 2. 初始化管理器

```cpp
// 初始化，指定日志目录（默认为 "logs"）
spdlog::BusinessLogManager::instance().initialize("business_logs");
```

### 3. 写入日志

#### 方式一：使用宏（推荐）

```cpp
// 录制屏幕日志
BUSINESS_LOG_SCREEN_RECORD(spdlog::level::info, "开始录制屏幕");
BUSINESS_LOG_SCREEN_RECORD(spdlog::level::debug, "分辨率: 1920x1080");

// 打开桌面日志
BUSINESS_LOG_DESKTOP_OPEN(spdlog::level::info, "用户登录成功");

// 键盘记录日志
BUSINESS_LOG_KEYBOARD_RECORD(spdlog::level::debug, "捕获按键: Ctrl+C");

// 声音录制日志
BUSINESS_LOG_SOUND_RECORD(spdlog::level::error, "麦克风设备未找到");
```

#### 方式二：使用模板函数（支持格式化）

```cpp
// 支持格式化字符串
spdlog::business_log(spdlog::BusinessType::ScreenRecord, 
                     spdlog::level::info, 
                     "录制进度: {}%, 已录制 {} 秒", 50, 30);

spdlog::business_log(spdlog::BusinessType::DesktopOpen, 
                     spdlog::level::info, 
                     "打开窗口: {}, 位置: ({}, {})", "Chrome", 100, 200);
```

#### 方式三：使用枚举 API

```cpp
auto& manager = spdlog::BusinessLogManager::instance();

manager.log(spdlog::BusinessType::ScreenRecord, 
            spdlog::level::info, 
            "屏幕录制消息");

manager.log(spdlog::BusinessType::KeyboardRecord, 
            spdlog::level::debug, 
            "键盘记录消息");
```

## 高级用法

### 设置日志级别

```cpp
auto& manager = spdlog::BusinessLogManager::instance();

// 设置单个业务的日志级别
manager.set_level(spdlog::BusinessType::ScreenRecord, spdlog::level::warn);

// 设置所有业务的日志级别
manager.set_all_levels(spdlog::level::debug);
```

### 刷新日志

```cpp
// 刷新单个业务的日志
manager.flush(spdlog::BusinessType::ScreenRecord);

// 刷新所有业务的日志
manager.flush_all();
```

### 获取业务名称

```cpp
std::string name = spdlog::BusinessLogManager::business_type_to_string(
    spdlog::BusinessType::ScreenRecord
);
// 返回: "ScreenRecord"
```

## 编译示例

### 使用 CMake

```bash
cd spdlog/example
mkdir build && cd build
cmake -f ../CMakeLists_business_log.txt ..
cmake --build .
./business_log_example
```

### 直接编译（Linux/GCC）

```bash
cd spdlog
g++ -std=c++17 -I include -o business_log_example \
    example/business_log_example.cpp \
    src/business_log_manager.cpp \
    src/spdlog.cpp \
    src/stdout_sinks.cpp \
    src/color_sinks.cpp \
    src/file_sinks.cpp \
    src/async.cpp \
    src/cfg.cpp \
    src/bundled_fmtlib_format.cpp \
    -lpthread
```

## 配置参数

在 `business_log_manager.cpp` 中可以修改以下默认配置：

| 参数 | 默认值 | 说明 |
|-----|-------|------|
| `max_file_size` | 30 * 1024 * 1024 (30MB) | 单个日志文件最大大小 |
| `max_files` | 3 | 最多保留的日志文件数 |
| `level` | `level::info` | 默认日志级别 |
| `log_pattern` | `"[%Y-%m-%d %H:%M:%S.%e] [%l] %v"` | 日志格式 |

## 线程安全

- 所有日志写入方法都是线程安全的
- 支持多线程并发写入不同业务的日志
- 支持多线程并发写入同一业务的日志

## 注意事项

1. **初始化时机** - 在首次写入日志前调用 `initialize()` 方法
2. **日志目录** - 如果不存在会自动创建
3. **文件权限** - 确保程序有写入日志目录的权限
4. **磁盘空间** - 每个业务最多占用 90MB（3个文件 × 30MB）

## 日志格式示例

```
[2025-01-15 10:30:25.123] [info] 开始录制屏幕
[2025-01-15 10:30:25.456] [debug] 屏幕分辨率: 1920x1080
[2025-01-15 10:30:30.789] [warning] 帧率下降
[2025-01-15 10:35:00.000] [info] 录制完成
```

## 扩展开发

如需添加新的业务类型：

1. 在 `BusinessType` 枚举中添加新类型
2. 在 `initialize()` 方法中添加配置
3. 在 `business_type_to_string()` 中添加名称映射
4. 添加对应的便捷方法和宏

## 许可证

与 spdlog 相同，使用 MIT 许可证。
