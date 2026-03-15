// Copyright(c) 2025 business_log_manager contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace spdlog {

// 业务类型枚举
enum class BusinessType {
    ScreenRecord,    // 录制屏幕
    DesktopOpen,     // 打开桌面
    KeyboardRecord,  // 录制键盘
    SoundRecord      // 录制声音
};

// 业务日志配置
struct BusinessLogConfig {
    std::string base_filename;     // 基础文件名
    size_t max_file_size = 30 * 1024 * 1024;  // 最大文件大小 (30MB)
    size_t max_files = 3;          // 最多保留文件数
    level::level_enum level = level::info;  // 日志级别
};

// 多业务日志管理器
// 为不同业务类型创建独立的轮转日志文件
// 每个业务最多保留3个日志文件，每个文件最大30MB
class BusinessLogManager {
public:
    // 获取单例实例
    static BusinessLogManager& instance();

    // 初始化业务日志配置
    // 参数 log_dir: 日志文件存放目录
    void initialize(const std::string& log_dir = "logs");

    // 写入日志
    // 参数 business: 业务类型
    // 参数 lvl: 日志级别
    // 参数 msg: 日志消息
    void log(BusinessType business, level::level_enum lvl, const std::string& msg);

    // 便捷方法：各业务类型的日志写入
    void screen_record_log(level::level_enum lvl, const std::string& msg);
    void desktop_open_log(level::level_enum lvl, const std::string& msg);
    void keyboard_record_log(level::level_enum lvl, const std::string& msg);
    void sound_record_log(level::level_enum lvl, const std::string& msg);

    // 设置指定业务的日志级别
    void set_level(BusinessType business, level::level_enum lvl);

    // 设置所有业务的日志级别
    void set_all_levels(level::level_enum lvl);

    // 刷新指定业务的日志
    void flush(BusinessType business);

    // 刷新所有业务的日志
    void flush_all();

    // 获取业务类型对应的字符串名称
    static std::string business_type_to_string(BusinessType business);

private:
    BusinessLogManager() = default;
    ~BusinessLogManager() = default;
    BusinessLogManager(const BusinessLogManager&) = delete;
    BusinessLogManager& operator=(const BusinessLogManager&) = delete;

    // 创建业务日志器
    void create_business_logger(BusinessType business, const BusinessLogConfig& config);

    // 获取业务对应的logger
    std::shared_ptr<logger> get_logger(BusinessType business);

    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::mutex mutex_;
    std::string log_dir_;
    bool initialized_ = false;
};

// 宏定义，方便使用
#define BUSINESS_LOG_SCREEN_RECORD(lvl, msg) \
    spdlog::BusinessLogManager::instance().screen_record_log(lvl, msg)

#define BUSINESS_LOG_DESKTOP_OPEN(lvl, msg) \
    spdlog::BusinessLogManager::instance().desktop_open_log(lvl, msg)

#define BUSINESS_LOG_KEYBOARD_RECORD(lvl, msg) \
    spdlog::BusinessLogManager::instance().keyboard_record_log(lvl, msg)

#define BUSINESS_LOG_SOUND_RECORD(lvl, msg) \
    spdlog::BusinessLogManager::instance().sound_record_log(lvl, msg)

// 模板化便捷日志方法
// 支持格式化字符串，如: business_log(spdlog::BusinessType::ScreenRecord, spdlog::level::info, "Value: {}", 42);
template <typename... Args>
inline void business_log(BusinessType business, level::level_enum lvl, format_string_t<Args...> fmt, Args &&...args) {
    auto& manager = BusinessLogManager::instance();
    memory_buf_t buf;
#ifdef SPDLOG_USE_STD_FORMAT
    fmt_lib::vformat_to(std::back_inserter(buf), fmt.get(), fmt_lib::make_format_args(args...));
#else
    fmt::vformat_to(fmt::appender(buf), fmt.get(), fmt::make_format_args(args...));
#endif
    manager.log(business, lvl, std::string(buf.data(), buf.size()));
}

}  // namespace spdlog
