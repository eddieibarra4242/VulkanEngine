#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wshadow"
#   pragma clang diagnostic ignored "-Winvalid-utf8"
#   pragma clang diagnostic ignored "-Wold-style-cast"
#   pragma clang diagnostic ignored "-Wdeprecated-volatile"
#   pragma clang diagnostic ignored "-Wshift-op-parentheses"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wshadow"
#   pragma GCC diagnostic ignored "-Winvalid-utf8"
#   pragma GCC diagnostic ignored "-Wold-style-cast"
#   pragma GCC diagnostic ignored "-Wcast-align"
#   pragma GCC diagnostic ignored "-Wunused-parameter"
#   pragma GCC diagnostic ignored "-Wconversion"
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#   pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif
