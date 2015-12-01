#include "vvvstlhelper.hpp"
#include "vvvclanghelper.hpp"
#include "vvvsourcegraph.hpp"
#include "collectstructs.h"
#include "collectfunctions.h"
#include <functional>
#include <string>
#include <clang/Analysis/AnalysisContext.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Lex/Preprocessor.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>


Graph createFlowChart(const clang::FunctionDecl* fdecl)
{
    Graph g;
    const auto functionName = fdecl->getNameAsString();
    auto start = addFlowchartVertex( g, new VertexBegin(), 0, functionName + "\n" + RUK_FUNCTION_BEGIN);
    auto end   = addFlowchartVertex( g, new VertexEnd(),  0,  functionName + "\n" + RUK_FUNCTION_END  );
    auto body  = getSemanticVertexFromStmt( fdecl->getBody(), g, fdecl->getASTContext() );
    auto bodyVertex = body->expand( start, end, end, end, end, 1 );
    boost::add_edge(start, bodyVertex, g);
    return g;
}

void clangFlowChart(const clang::FunctionDecl* d)
{
    clang::LangOptions    lo;
    clang::AnalysisDeclContextManager m;
    auto ac = m.getContext(d);
    //auto cfg = ac->getCFG();
    auto cfg = ac->getUnoptimizedCFG();
    cfg->dump(lo,false);
    //cfg->viewCFG(lo);
    std::cerr << "---------------------------------------------------------------" << std::endl;
    //std::unique_ptr<CFG> fcfg = clang::CFG::buildCFG(d,d,Context);            
}

void dumpGraph(clang::ASTContext& Context)
{
    const auto declsInMain = getNonSystemDeclarations(Context);
    const auto functionDeclsInMain = filterFunctions( declsInMain );
    Graph bigGraph;
    for( const auto& d : functionDeclsInMain ) {
        auto g = FunctionDescription(d).getFlowChart(); 
        calculateBranchesnes( g ); 
        boost::copy_graph(g, bigGraph);
    }

    //       boost::write_graphviz( std::cout, bigGraph, myLabler( bigGraph ), myEdgeLabler(bigGraph), myGraphPropertyWriter() );

    /*const auto recordDeclsInMain = filterStructs( declsInMain );
      for( const auto& d : recordDeclsInMain )
      printStructure( d );*/
}



void printFunctionsDecs(clang::ASTContext& Context)
{
    const auto declsInMain = getNonSystemDeclarations(Context);
    const auto functionDeclsInMain = filterFunctions( declsInMain );
    for(const auto& d: functionDeclsInMain)
    {
        std::cout << d->getReturnType().getAsString() << " ";
        std::cout << d->getNameAsString() << std::endl;
        for(const auto p: getFunctionParams(d) )
            std::cout << p->getNameAsString() << " : " << p->getType().getAsString() << ";" << std::endl;
        std::cout << std::endl;
    }
}


std::vector<std::string> argstoarray(int argc, char** argv)
{
    std::vector<std::string> ret;
    for(int i = 1; i < argc; ++i)
        ret.push_back( argv[i] );
    return ret;
}

static std::vector<std::string> filterParams(const std::vector<std::string>& in)
{
    auto beginWithMinus = [](const std::string& s){ return (s[0]=='-') ? true :false; };
    return filter(in, beginWithMinus);
}

static std::vector<std::string> filterNotParams(const std::vector<std::string>& in)
{
    auto beginWithNotMinus = [](const std::string& s){ return (s[0]!='-') ? true :false; };
    return filter(in, beginWithNotMinus);
}

int main(int argc, char** argv)
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
}

