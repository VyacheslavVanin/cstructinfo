#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "collectstructs.h"
#include "collectfunctions.h"
#include "arghelper.hpp"
#include "myparamnames.hpp"

int StructAndFuncInfoCollector(int argc, char** argv)
{
    if(argc < 2) { std::cerr << "fatal error: no input files"<< std::endl;}

    static const std::set<std::string> myParameters = {PARAM_NAME_MAIN_ONLY, 
                                                PARAM_NAME_FUNC_ONLY,
                                                PARAM_NAME_STRUCT_ONLY};
    static const auto myParamFilter = [](const auto& p)
                                {return contain(myParameters, p);};
    static const auto notMyParamsFilter = [](const auto& p)
                                {return !myParamFilter(p);};

    boost::property_tree::ptree root;
    boost::property_tree::ptree structdescs;
    boost::property_tree::ptree functiondescs;

    const auto args = argstoarray(argc, argv);
    const auto allParams    = filterParams(args);
    const auto myParams     = filter(allParams, myParamFilter);
    const auto cxxparams    = filter(allParams, notMyParamsFilter);
    const auto filenames    = filterNotParams(args);

    for( const auto& name: filenames) {
        const char* code = getSourceFromFile( name.c_str() );
        using namespace clang::tooling;
        runToolOnCodeWithArgs( 
                new CollectStructsInfoAction(structdescs, myParams),
                code, cxxparams, name.c_str() );
        runToolOnCodeWithArgs(
                new CollectFunctionsInfoAction(functiondescs, myParams),
                code, cxxparams, name.c_str() );
    }
    root.push_back(std::make_pair("structs",structdescs));
    root.push_back(std::make_pair("functions",functiondescs));
    boost::property_tree::write_json(std::cout, root);
    return 0;
}

