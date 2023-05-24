#pragma once

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wshadow"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GNU diagnostic push
#endif
#include <spdlog/spdlog.h>
#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GNU diagnostic pop
#endif