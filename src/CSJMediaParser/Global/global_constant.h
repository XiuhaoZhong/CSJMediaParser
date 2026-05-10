#pragma once

#include <string>

#ifdef _WIN32
    static std::string g_log_path("Logs\\media_parser.log");
#else
    static std::string g_log_path("/tmp/Logs/CSJMediaParser/media_parser.log");
#endif

static const std::string g_test_mp4_file("what_song.mp4");