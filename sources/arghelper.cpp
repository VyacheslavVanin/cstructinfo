#include "arghelper.hpp"
#include "vvvclanghelper.hpp"
#include "stdhelper/containerhelper.hpp"

using string_list = CxxToolArgs::string_list;
using namespace vvv::helpers;

std::vector<std::string> argstoarray(int argc, const char** argv)
{
    return std::vector<std::string>(&argv[1], &argv[argc]);
}

std::vector<std::string> filterParams(const std::vector<std::string>& in)
{
    auto beginWithMinus = [](const std::string& s) { return s[0] == '-'; };
    return filter(in, beginWithMinus);
}

std::vector<std::string> filterSourceFiles(const std::vector<std::string>& in,
                                           const std::vector<std::string>& exts)
{
    auto endWith = [&exts](const std::string& s) {
        return any_of(exts, [&s](const auto& ext) {
            return s.rfind(ext) != std::string::npos;
        });
    };
    return filter(in, endWith);
}

std::vector<std::string>
filterNotSourceFiles(const std::vector<std::string>& in,
                     const std::vector<std::string>& exts)
{
    auto not_source = [&exts](const std::string& s) {
        return std::all_of(exts.begin(), exts.end(), [&s](const auto& ext) {
            return s.rfind(ext) == std::string::npos;
        });
    };
    return filter(in, not_source);
}

namespace {
std::vector<std::string> autoDetectFlags(const std::string& filename)
{
    using clang::tooling::CompilationDatabase;

    std::string error;
    const auto compile_db =
        CompilationDatabase::autoDetectFromSource(filename, error);
    if (!compile_db) {
        std::cerr << error << "\n";
        return {};
    }

    const auto& ccs = compile_db->getCompileCommands(filename);
    if (ccs.empty())
        return {};

    const auto& cc = ccs.front();
    const auto& cl = cc.CommandLine;
    if (cl.size() <= 5)
        return {};

    // remove command (/usr/bin/c++) and -c ..foo.cpp -o ..foo.o
    return std::vector<std::string>(cl.begin() + 1, cl.end() - 4);
}
} // namespace

CxxToolArgs::CxxToolArgs(int argc, const char** argv,
                         const string_list& tool_flags)
{
    const auto myParamFilter = [&tool_flags](const auto& p) {
        return contain(tool_flags, p);
    };
    const auto notMyParamsFilter = [&myParamFilter](const auto& p) {
        return !myParamFilter(p);
    };

    args = argstoarray(argc, argv);

    const auto allParams = filterNotSourceFiles(args);
    custom_params = filter(allParams, myParamFilter);
    compiler_params = filter(allParams, notMyParamsFilter);
    filenames = filterSourceFiles(args);
}

string_list
CxxToolArgs::getFlagsForSource(const std::string& source_file_name) const
{
    const auto& compile_commands = autoDetectFlags(source_file_name);
    return compile_commands.size() ? compile_commands + compiler_params
                                   : compiler_params;
}

const string_list& CxxToolArgs::getCustomFlags() const { return custom_params; }

const string_list& CxxToolArgs::getFilenames() const { return filenames; }

const string_list& CxxToolArgs::getCompilerParams() const
{
    return compiler_params;
}

const string_list& CxxToolArgs::getArgs() const { return args; }
