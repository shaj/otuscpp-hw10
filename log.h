
#pragma once

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace my
{
    extern std::shared_ptr<spdlog::logger> my_logger;
}
