#include "vvvstlhelper.hpp"
#include "vvvclanghelper.hpp"
#include "vvvsourcegraph.hpp"
#include "structfuncinfocollector.hpp"
#include <functional>
#include <string>
#include <clang/Analysis/AnalysisContext.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Lex/Preprocessor.h>
#include <boost/algorithm/string.hpp>
#include "arghelper.hpp"
#include "flowchartbuilder.h"


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

int printFlowChart(int argc, char** argv)
{
    const auto& args = argstoarray(argc, argv);
    const auto& params = filterParams(args);
    const auto& filenames = filterNotParams(args);

    for(const auto& name: filenames){
        using namespace clang::tooling;
        const char* code = getSourceFromFile(name.c_str());
        runToolOnCodeWithArgs(new FlowChartAction(), code, params, name.c_str());
    }
    return 0;
}


int main(int argc, char** argv)
{
   // return printFlowChart(argc,argv);
    return StructAndFuncInfoCollector(argc, argv);
}

