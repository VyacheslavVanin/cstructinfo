#include "arghelper.hpp"
#include "stdhelper/containerhelper.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <set>
#include "collectdecls.h"
#include "myparamnames.hpp"

using namespace vvv::helpers;

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

int StructAndFuncInfoCollector(int argc, const char** argv)
{
    if (argc < 2) {
        std::cerr << "fatal error: no input files" << std::endl;
    }

    static const std::vector<std::string> myParameters = {
        PARAM_NAME_MAIN_ONLY, PARAM_NAME_NO_FUNCS, PARAM_NAME_NO_STRUCTS,
        PARAM_NAME_NO_SIZES,  PARAM_NAME_HELP,     PARAM_NAME_WITH_SOURCE};

    const auto params = CxxToolArgs(argc, argv, myParameters);

    printHelpIfNeeded(params.getArgs());

    boost::property_tree::ptree root;
    for (const auto& name : params.getFilenames()) {
        using namespace clang::tooling;
        const auto code = getSourceFromFile(name.c_str());
        const auto& flags = params.getFlagsForSource(name);
        const auto& tool_flags = params.getCustomFlags();
        runToolOnCodeWithArgs(new CollectDeclsAction(root, tool_flags),
                              code.c_str(), flags, name.c_str());
    }

    boost::property_tree::write_json(std::cout, root);
    return 0;
}
