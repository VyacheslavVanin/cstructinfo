#ifndef ARGHELPER_HPP
#define ARGHELPER_HPP
#include <vector>
#include <string>
#include "vvvstlhelper.hpp"

inline
std::vector<std::string> argstoarray(int argc, char** argv)
{
    return std::vector<std::string>(&argv[1], &argv[argc]);
}

inline
std::vector<std::string> filterParams(const std::vector<std::string>& in)
{
    auto beginWithMinus = [](const std::string& s){ return s[0]=='-'; };
    return filter(in, beginWithMinus);
}

inline
std::vector<std::string> filterNotParams(const std::vector<std::string>& in)
{
    auto beginWithNotMinus = [](const std::string& s){ return s[0] != '-'; };
    return filter(in, beginWithNotMinus);
}

#endif

