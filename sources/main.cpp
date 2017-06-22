#include "arghelper.hpp"
#include "structfuncinfocollector.hpp"
#include "vvvclanghelper.hpp"
#include "vvvstlhelper.hpp"
#include <boost/algorithm/string.hpp>
#include <clang/Analysis/AnalysisContext.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Lex/Preprocessor.h>
#include <functional>
#include <string>

void clangFlowChart(const clang::FunctionDecl* d)
{
    clang::LangOptions lo;
    clang::AnalysisDeclContextManager m;
    auto ac = m.getContext(d);
    // auto cfg = ac->getCFG();
    auto cfg = ac->getUnoptimizedCFG();
    cfg->dump(lo, false);
    // cfg->viewCFG(lo);
    std::cerr
        << "---------------------------------------------------------------"
        << std::endl;
    // std::unique_ptr<CFG> fcfg = clang::CFG::buildCFG(d,d,Context);
}

int main(int argc, char** argv)
{
    return StructAndFuncInfoCollector(argc, argv);
}
