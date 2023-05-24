#pragma once

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wshadow"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wshadow"
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#   pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#include <spdlog/spdlog.h>
#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif