#pragma once
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include "vvvclanghelper.hpp"

void printStructDecls(const std::vector<const clang::Decl*>& decls,
                      boost::property_tree::ptree& tree,
                      const std::vector<std::string>& params);
