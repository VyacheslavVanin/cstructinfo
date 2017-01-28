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
                                                       PARAM_NAME_NO_FUNCS,
                                                       PARAM_NAME_NO_STRUCTS,
                                                       PARAM_NAME_NO_SIZES};
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

    const auto needStructs   = !contain(myParams, PARAM_NAME_NO_STRUCTS);
    const auto needFunctions = !contain(myParams, PARAM_NAME_NO_FUNCS);

    for( const auto& name: filenames) {
        using namespace clang::tooling;
        const auto code = getSourceFromFile( name.c_str() );
        if(needStructs)
            runToolOnCodeWithArgs( 
                    new CollectStructsInfoAction(structdescs, myParams),
                    code.c_str(), cxxparams, name.c_str() );
        if(needFunctions)
            runToolOnCodeWithArgs(
                    new CollectFunctionsInfoAction(functiondescs, myParams),
                    code.c_str(), cxxparams, name.c_str() );
    }

    if(structdescs.size())
        root.push_back(std::make_pair("structs",structdescs));
    if(functiondescs.size())
        root.push_back(std::make_pair("functions",functiondescs));
    boost::property_tree::write_json(std::cout, root);
    return 0;
}

