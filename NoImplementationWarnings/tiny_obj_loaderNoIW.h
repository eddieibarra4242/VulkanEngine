#pragma once

#if defined(__clang__)
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <tiny_obj_loader.h>
#if defined(__clang__)
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif
