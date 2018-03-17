#include "arghelper.hpp"
#include "collectdecls.h"
#include "myparamnames.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <set>

void printHelpIfNeeded(const std::vector<std::string>& params)
{
    static const char* message =
        "cstructinfo usage:\n"
        "cstructinfo [options] <input files ...> [clang compiler options]\n"
        "Options:\n"
        "--main-only - do not process any included files. Process only "
        "specified\n"
        "--no-functions - exclude functions descriptions from output\n"
        "--no-structs - exclude structs descriptions from output\n"
        "--no-sizes - do not add sizeofs of primitive types to structs "
        "descriptions\n"
        "--with-source - add source field containing 'source'"
        " of struct/function\n"
        "--help - show this help\n";
    if (params.empty() || contain(params, PARAM_NAME_HELP)) {
        std::cout << message << std::endl;
        exit(EXIT_SUCCESS);
    }
    return;
}

std::vector<std::string> autoDetectFlags(const std::string& filename)
{
    using clang::tooling::CompilationDatabase;

    std::string error;
    const auto compile_db =
        CompilationDatabase::autoDetectFromSource(filename, error);
    if (!compile_db) {
        std::cout << error << "\n";
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

int StructAndFuncInfoCollector(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "fatal error: no input files" << std::endl;
    }

    static const std::set<std::string> myParameters = {
        PARAM_NAME_MAIN_ONLY, PARAM_NAME_NO_FUNCS, PARAM_NAME_NO_STRUCTS,
        PARAM_NAME_NO_SIZES,  PARAM_NAME_HELP,     PARAM_NAME_WITH_SOURCE};
    static const auto myParamFilter = [](const auto& p) {
        return contain(myParameters, p);
    };
    static const auto notMyParamsFilter = [](const auto& p) {
        return !myParamFilter(p);
    };

    boost::property_tree::ptree root;

    const auto args = argstoarray(argc, argv);
    const auto allParams = filterNotSourceFiles(args);
    const auto myParams = filter(allParams, myParamFilter);
    const auto cxxparams = filter(allParams, notMyParamsFilter);
    const auto filenames = filterSourceFiles(args);

    printHelpIfNeeded(args);

    for (const auto& name : filenames) {
        using namespace clang::tooling;
        const auto code = getSourceFromFile(name.c_str());
        const auto& compile_commands = autoDetectFlags(name);
        const auto& params =
            compile_commands.size() ? compile_commands + cxxparams : cxxparams;
        runToolOnCodeWithArgs(new CollectDeclsAction(root, myParams),
                              code.c_str(), params, name.c_str());
    }

    boost::property_tree::write_json(std::cout, root);
    return 0;
}
