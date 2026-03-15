// Copyright(c) 2025 business_log_manager contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/business_log_manager.h>
#include <spdlog/details/os.h>

#include <filesystem>

namespace spdlog {

BusinessLogManager& BusinessLogManager::instance() {
    static BusinessLogManager instance;
    return instance;
}

void BusinessLogManager::initialize(const std::string& log_dir) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }

    log_dir_ = log_dir;

    // 创建日志目录
    std::error_code ec;
    if (!std::filesystem::exists(log_dir_, ec)) {
        std::filesystem::create_directories(log_dir_, ec);
    }

    // 配置四个业务的日志
    // 每个业务：最多3个文件，每个文件最大30MB
    
    // 1. 录制屏幕
    BusinessLogConfig screen_config;
    screen_config.base_filename = log_dir_ + "/screen_record.log";
    screen_config.max_file_size = 30 * 1024 * 1024;  // 30MB
    screen_config.max_files = 3;
    screen_config.level = level::info;
    create_business_logger(BusinessType::ScreenRecord, screen_config);

    // 2. 打开桌面
    BusinessLogConfig desktop_config;
    desktop_config.base_filename = log_dir_ + "/desktop_open.log";
    desktop_config.max_file_size = 30 * 1024 * 1024;  // 30MB
    desktop_config.max_files = 3;
    desktop_config.level = level::info;
    create_business_logger(BusinessType::DesktopOpen, desktop_config);

    // 3. 录制键盘
    BusinessLogConfig keyboard_config;
    keyboard_config.base_filename = log_dir_ + "/keyboard_record.log";
    keyboard_config.max_file_size = 30 * 1024 * 1024;  // 30MB
    keyboard_config.max_files = 3;
    keyboard_config.level = level::info;
    create_business_logger(BusinessType::KeyboardRecord, keyboard_config);

    // 4. 录制声音
    BusinessLogConfig sound_config;
    sound_config.base_filename = log_dir_ + "/sound_record.log";
    sound_config.max_file_size = 30 * 1024 * 1024;  // 30MB
    sound_config.max_files = 3;
    sound_config.level = level::info;
    create_business_logger(BusinessType::SoundRecord, sound_config);

    initialized_ = true;
}

void BusinessLogManager::create_business_logger(BusinessType business, const BusinessLogConfig& config) {
    try {
        // 创建轮转文件sink
        auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
            config.base_filename,
            config.max_file_size,
            config.max_files,
            false  // rotate_on_open
        );

        // 创建logger
        std::string logger_name = business_type_to_string(business);
        auto logger = std::make_shared<spdlog::logger>(logger_name, sink);
        logger->set_level(config.level);
        
        // 设置日志格式: [时间] [级别] 消息
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        loggers_[business] = logger;
    } catch (const spdlog_ex& ex) {
        // 如果创建失败，记录到stderr
        fprintf(stderr, "Failed to create logger for business %s: %s\n", 
                business_type_to_string(business).c_str(), ex.what());
    }
}

std::shared_ptr<logger> BusinessLogManager::get_logger(BusinessType business) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(business);
    if (it != loggers_.end()) {
        return it->second;
    }
    return nullptr;
}

void BusinessLogManager::log(BusinessType business, level::level_enum lvl, const std::string& msg) {
    if (!initialized_) {
        initialize();
    }

    auto logger = get_logger(business);
    if (logger) {
        logger->log(lvl, msg);
    }
}

void BusinessLogManager::screen_record_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::ScreenRecord, lvl, msg);
}

void BusinessLogManager::desktop_open_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::DesktopOpen, lvl, msg);
}

void BusinessLogManager::keyboard_record_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::KeyboardRecord, lvl, msg);
}

void BusinessLogManager::sound_record_log(level::level_enum lvl, const std::string& msg) {
    log(BusinessType::SoundRecord, lvl, msg);
}

void BusinessLogManager::set_level(BusinessType business, level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = loggers_.find(business);
    if (it != loggers_.end()) {
        it->second->set_level(lvl);
    }
}

void BusinessLogManager::set_all_levels(level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [business, logger] : loggers_) {
        logger->set_level(lvl);
    }
}

void BusinessLogManager::flush(BusinessType business) {
    auto logger = get_logger(business);
    if (logger) {
        logger->flush();
    }
}

void BusinessLogManager::flush_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [business, logger] : loggers_) {
        logger->flush();
    }
}

std::string BusinessLogManager::business_type_to_string(BusinessType business) {
    switch (business) {
        case BusinessType::ScreenRecord:
            return "ScreenRecord";
        case BusinessType::DesktopOpen:
            return "DesktopOpen";
        case BusinessType::KeyboardRecord:
            return "KeyboardRecord";
        case BusinessType::SoundRecord:
            return "SoundRecord";
        default:
            return "Unknown";
    }
}

}  // namespace spdlog
