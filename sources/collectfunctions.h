#ifndef COLLECTFUNCTIONS_H
#define COLLECTFUNCTIONS_H
#include "vvvclanghelper.hpp"
#include <boost/property_tree/ptree.hpp>
#include <clang/Analysis/AnalysisContext.h>

using printFunctionsParam = std::vector<std::string>;

boost::property_tree::ptree
makeFunctionDescriptionNode(const clang::FunctionDecl* d);

void printFunctionDecls(clang::ASTContext& Context,
                        boost::property_tree::ptree& tree,
                        const printFunctionsParam& params);

class ExtractFunctionDataConsumer : public clang::ASTConsumer {
public:
    explicit ExtractFunctionDataConsumer(clang::ASTContext* Context,
                                         boost::property_tree::ptree& tree,
                                         const printFunctionsParam& params)
        : tree(tree), params(params)
    {
    }

    virtual void HandleTranslationUnit(clang::ASTContext& Context)
    {
        printFunctionDecls(Context, tree, params);
    }

    boost::property_tree::ptree& tree;
    const printFunctionsParam& params;
};

class CollectFunctionsInfoAction : public clang::ASTFrontendAction {
public:
    CollectFunctionsInfoAction(boost::property_tree::ptree& tree,
                               const printFunctionsParam& params)
        : tree(tree), params(params)
    {
    }

    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile)
    {
        return std::unique_ptr<clang::ASTConsumer>(
            new ExtractFunctionDataConsumer(&Compiler.getASTContext(), tree,
                                            params));
    }

    boost::property_tree::ptree& tree;
    const printFunctionsParam& params;
};

#endif
