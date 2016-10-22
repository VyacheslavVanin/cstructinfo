#ifndef COLLECTSTRUCTS_H
#define COLLECTSTRUCTS_H
#include <boost/property_tree/ptree.hpp>
#include <clang/Analysis/AnalysisContext.h>
#include "vvvclanghelper.hpp"

using printStructsParam = std::vector<std::string>;

void printStructDecls(clang::ASTContext& Context,
                      boost::property_tree::ptree& tree,
                      const printStructsParam& params);

class ExtractStructDataConsumer : public clang::ASTConsumer
{
    public:
        explicit ExtractStructDataConsumer(clang::ASTContext* Context,
                                           boost::property_tree::ptree& tree,
                                           const printStructsParam& params)
            :tree(tree), params(params)
        {}

        virtual void HandleTranslationUnit( clang::ASTContext& Context)
        {
            printStructDecls(Context, tree, params);
        }

        boost::property_tree::ptree& tree;
        const printStructsParam& params;
};

class CollectStructsInfoAction : public clang::ASTFrontendAction
{
    public:
        CollectStructsInfoAction(boost::property_tree::ptree& tree,
                                 const printStructsParam& params)
                : tree(tree), params(params)
        {}    

        virtual std::unique_ptr<clang::ASTConsumer>
            CreateASTConsumer(clang::CompilerInstance& Compiler,
                              llvm::StringRef InFile)
        {
            return std::unique_ptr<clang::ASTConsumer>(
                       new ExtractStructDataConsumer(&Compiler.getASTContext(),
                                                     tree, params));
        }

        boost::property_tree::ptree& tree; 
        const printStructsParam& params;
};

#endif

