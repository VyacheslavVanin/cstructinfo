#ifndef ARGHELPER_HPP
#define ARGHELPER_HPP
#include "vvvstlhelper.hpp"
#include <string>
#include <vector>

std::vector<std::string> argstoarray(int argc, const char** argv);
std::vector<std::string> filterParams(const std::vector<std::string>& in);

std::vector<std::string>
filterSourceFiles(const std::vector<std::string>& in,
                  const std::vector<std::string>& exts = {".hpp", ".cpp", ".h",
                                                          ".c", ".cxx"});

std::vector<std::string>
filterNotSourceFiles(const std::vector<std::string>& in,
                     const std::vector<std::string>& exts = {
                         ".hpp", ".cpp", ".h", ".c", ".cxx"});

class CxxToolArgs {
public:
    using string_list = std::vector<std::string>;
    CxxToolArgs(int argc, const char** argv, const string_list& tool_flags);

    string_list getFlagsForSource(const std::string& source_file_name) const;

    /** @brief Get flags present in argv and tools_flags */
    const string_list& getCustomFlags() const;

    /** @brief Get list of source files */
    const string_list& getFilenames() const;

    /** @brief Get varg list without source names and tool_flags */
    const string_list& getCompilerParams() const;

    /** @brief get original argv as string vector */
    const string_list& getArgs() const;

private:
    string_list custom_params;
    string_list filenames;
    string_list compiler_params;
    string_list args;
};

#endif
