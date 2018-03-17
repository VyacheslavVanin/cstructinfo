#pragma once
#include "vvvclanghelper.hpp"

struct FunctionComments {
    std::string brief;
    std::map<std::string, std::string> params;
    const std::string& getParam(const std::string& param_name) const;
    const std::string& getReturn() const;
};

FunctionComments getDoxyComments(const clang::FunctionDecl* d);
