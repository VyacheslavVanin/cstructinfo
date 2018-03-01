#ifndef ARGHELPER_HPP
#define ARGHELPER_HPP
#include "vvvstlhelper.hpp"
#include <string>
#include <vector>

inline std::vector<std::string> argstoarray(int argc, char** argv)
{
    return std::vector<std::string>(&argv[1], &argv[argc]);
}

inline std::vector<std::string> filterParams(const std::vector<std::string>& in)
{
    auto beginWithMinus = [](const std::string& s) { return s[0] == '-'; };
    return filter(in, beginWithMinus);
}

inline std::vector<std::string> filterSourceFiles(
    const std::vector<std::string>& in,
    const std::vector<std::string> exts = {".hpp", ".cpp", ".h", ".c", ".cxx"})
{
    auto endWith = [&exts](const std::string& s) {
        return any_of(exts, [&s](const auto& ext) {
            return s.rfind(ext) != std::string::npos;
        });
    };
    return filter(in, endWith);
}

inline std::vector<std::string> filterNotSourceFiles(
    const std::vector<std::string>& in,
    const std::vector<std::string> exts = {".hpp", ".cpp", ".h", ".c", ".cxx"})
{
    auto not_source = [&exts](const std::string& s) {
        return std::all_of(exts.begin(), exts.end(), [&s](const auto& ext) {
            return s.rfind(ext) == std::string::npos;
        });
    };
    return filter(in, not_source);
}

#endif
