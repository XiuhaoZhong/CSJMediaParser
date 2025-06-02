#include "CSJLogger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

CSJLogger *CSJLogger::getLoggerInst() {
    static CSJLogger logger;
    return &logger;
}

CSJLogger::CSJLogger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    //console_sink->set_pattern("[%^%l%$] %v");

    const spdlog::sinks_init_list sink_list = {console_sink};

    spdlog::init_thread_pool(8192, 1);

    m_pLogger = std::make_shared<spdlog::async_logger>("CSJLogger",
                                                       sink_list.begin(),
                                                       sink_list.end(),
                                                       spdlog::thread_pool(),
                                                       spdlog::async_overflow_policy::block);
    
    m_pLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(m_pLogger);
}

CSJLogger::~CSJLogger() {
    m_pLogger->flush();
    spdlog::drop_all();
}
