#ifndef COLLECTFUNCTIONS_H
#define COLLECTFUNCTIONS_H
#include <boost/property_tree/ptree.hpp>
#include <clang/Analysis/AnalysisContext.h>
#include "vvvclanghelper.hpp"

void printFunctionDecls(clang::ASTContext& Context,
                        boost::property_tree::ptree& tree);

class ExtractFunctionDataConsumer : public clang::ASTConsumer
{
    public:
        explicit ExtractFunctionDataConsumer(clang::ASTContext* Context,
                                           boost::property_tree::ptree& tree)
            :tree(tree)
        {}

        virtual void HandleTranslationUnit( clang::ASTContext& Context)
        {
            printFunctionDecls(Context, tree);
        }

        boost::property_tree::ptree& tree;
};

class CollectFunctionsInfoAction : public clang::ASTFrontendAction
{
    public:
        CollectFunctionsInfoAction(boost::property_tree::ptree& tree)
            : tree(tree) {}    

        virtual clang::ASTConsumer* 
        CreateASTConsumer(clang::CompilerInstance& Compiler,
                          llvm::StringRef InFile)
        {
            return new ExtractFunctionDataConsumer(&Compiler.getASTContext(),
                                                   tree);
        }

    boost::property_tree::ptree& tree; 
};

#endif

