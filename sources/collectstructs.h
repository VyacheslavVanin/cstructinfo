#ifndef COLLECTSTRUCTS_H
#define COLLECTSTRUCTS_H
#include <boost/property_tree/ptree.hpp>
#include <clang/Analysis/AnalysisContext.h>
#include "vvvclanghelper.hpp"

void printStructDecls(clang::ASTContext& Context, boost::property_tree::ptree& tree);

class ExtractStructDataConsumer : public clang::ASTConsumer
{
    public:
        explicit ExtractStructDataConsumer(clang::ASTContext* Context,
                                           boost::property_tree::ptree& tree)
            :tree(tree)
        {}

        virtual void HandleTranslationUnit( clang::ASTContext& Context)
        {
            printStructDecls(Context, tree);
        }

        boost::property_tree::ptree& tree;
};

class CollectStructsInfoAction : public clang::ASTFrontendAction
{
    public:
        CollectStructsInfoAction(boost::property_tree::ptree& tree): tree(tree) {}    

        virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile)
        {
            return new ExtractStructDataConsumer( &Compiler.getASTContext(), tree );
        }

    boost::property_tree::ptree& tree; 
};

#endif

