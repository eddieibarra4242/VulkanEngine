#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wold-style-cast"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#   pragma clang diagnostic ignored "-Wdouble-promotion"
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wold-style-cast"
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#   pragma GCC diagnostic ignored "-Wcast-align"
#   pragma GCC diagnostic ignored "-Wdouble-promotion"
#   pragma GCC diagnostic ignored "-Wduplicated-branches"
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#   pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <stb_image.h>
#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif