#ifndef PIPELINES_H
#define PIPELINES_H

#include <sstream>
#include <vector>

constexpr bool ENABLE_PIPELINE_LOGGING = true;

#define PIPELINE_LOG(msg) \
do { if (ENABLE_PIPELINE_LOGGING) std::cout << msg << std::endl; } while (0)

template<typename T>
std::string to_string_fallback(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

inline std::string to_string_fallback(const std::vector<double>& vals) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vals.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << vals[i];
    }
    oss << "]";
    return oss.str();
}

template<typename... Ts>
std::string tuple_to_string(const std::tuple<Ts...>& tpl) {
    std::ostringstream oss;
    oss << "(";
    std::apply([&oss](const Ts&... args) {
        std::size_t i = 0;
        ((oss << (i++ ? ", " : "") << to_string_fallback(args)), ...);
    }, tpl);
    oss << ")";
    return oss.str();
}

#endif // PIPELINES_H