#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "collectstructs.h"
#include "collectfunctions.h"
#include "arghelper.hpp"


int StructAndFuncInfoCollector(int argc, char** argv)
{
    if(argc < 2) { std::cerr << "fatal error: no input files"<< std::endl;}

    boost::property_tree::ptree root;
    boost::property_tree::ptree structdescs;
    boost::property_tree::ptree functiondescs;

    const auto args = argstoarray(argc, argv);
    const auto params = filterParams(args);
    const auto filenames = filterNotParams(args);
    for( const auto& name: filenames) {
        const char* code = getSourceFromFile( name.c_str() );
        using namespace clang::tooling;
        runToolOnCodeWithArgs( new CollectStructsInfoAction(structdescs), code, params, name.c_str() );
        runToolOnCodeWithArgs( new CollectFunctionsInfoAction(functiondescs), code, params, name.c_str() );
    }
    root.push_back(std::make_pair("structs",structdescs));
    root.push_back(std::make_pair("functions",functiondescs));
    boost::property_tree::write_json(std::cout, root);
    return 0;
}

