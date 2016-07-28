#ifndef FLOWCHARTBUILDER_H
#define FLOWCHARTBUILDER_H
#include <clang/Analysis/AnalysisContext.h>
#include "vvvclanghelper.hpp"


void createFlowCharts(clang::ASTContext& Context);


class FlowChartConsumer : public clang::ASTConsumer
{
    public:
        explicit FlowChartConsumer(clang::ASTContext* Context)
        {}

        virtual void HandleTranslationUnit(clang::ASTContext& Context)
        {
            createFlowCharts(Context);
        }
};

class FlowChartAction : public clang::ASTFrontendAction
{
    public:
        FlowChartAction() {}

        virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& Compiler,
                                                      llvm::StringRef InFile)
        {
            return new FlowChartConsumer(&Compiler.getASTContext());
        }
};

#endif

