#include "flowchartbuilder.h"

void createSingleFlowChart(const clang::FunctionDecl& f)
{
    using namespace clang;
    const auto& Context = f.getASTContext();
    LangOptions lo;
    AnalysisDeclContextManager m;
    AnalysisDeclContext*  ac = m.getContext(&f);
    CFG* cfg = ac->getUnoptimizedCFG();
    
    
    std::cout << f.getName().str() << std::endl;
    ac->dumpCFG(true); 
    
#if 0 
    for(auto i = cfg->nodes_begin(); i != cfg->nodes_end(); ++i)
    {
        auto& currentNode = (*i);
        auto tc  = currentNode.getTerminatorCondition(false);
        auto t  = currentNode.getTerminator().getStmt();
        auto l  = currentNode.getLabel();

        if(t)
            std::cout << "Terminator class = " << t->getStmtClassName() << std::endl;
        if(l)
        {
            const auto& labelName = decl2str(static_cast<const CaseStmt*>(l)->getLHS(), Context);
            std::cout << "Label = " << labelName << std::endl;
        }
        if(tc)
            std::cout << "Terminator condition = " << decl2str(tc, Context) << std::endl;

        for(auto j: currentNode )
            std::cout << decl2str((j).castAs<clang::CFGStmt>().getStmt(), Context) << std::endl;
    }
#endif

    //cfg->dump(lo, false);

    std::cout << "------------------------------------------" << std::endl;
}

void createFlowCharts(clang::ASTContext& Context)
{
    const auto& declsInMain = getNonSystemDeclarations(Context);
    const auto& functionsDecls = filterFunctions(declsInMain);
    for(const auto& f: functionsDecls)
        createSingleFlowChart(*f);
}

