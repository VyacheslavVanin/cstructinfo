#pragma once
#include "myparamnames.hpp"
#include "vvvclanghelper.hpp"
#include <boost/property_tree/ptree.hpp>
#include <clang/Analysis/AnalysisContext.h>

using ParamList = std::vector<std::string>;

boost::property_tree::ptree
makeFunctionDescriptionNode(const clang::FunctionDecl* d, bool needSources);

void printFunctionDecls(const std::vector<const clang::Decl*>& decls,
                        boost::property_tree::ptree& tree,
                        const ParamList& params);
void printStructDecls(const std::vector<const clang::Decl*>& decls,
                      boost::property_tree::ptree& tree,
                      const ParamList& params);

class ExtractDataConsumer : public clang::ASTConsumer {
public:
    explicit ExtractDataConsumer(clang::ASTContext* Context,
                                 boost::property_tree::ptree& tree,
                                 const ParamList& params)
        : tree(tree), params(params)
    {
    }

    ~ExtractDataConsumer() {
        tree.push_back(std::make_pair("structs", structdescs));
        tree.push_back(std::make_pair("functions", functiondescs));
    }

    virtual void HandleTranslationUnit(clang::ASTContext& Context)
    {
        const auto needStructs = !contain(params, PARAM_NAME_NO_STRUCTS);
        const auto needFunctions = !contain(params, PARAM_NAME_NO_FUNCS);
        const auto declsInMain = contain(params, PARAM_NAME_MAIN_ONLY)
                                     ? getMainFileDeclarations(Context)
                                     : getNonSystemDeclarations(Context);
        if (needFunctions)
            printFunctionDecls(declsInMain, functiondescs, params);
        if (needStructs)
            printStructDecls(declsInMain, structdescs, params);
    }

    boost::property_tree::ptree& tree;
    const ParamList& params;
    boost::property_tree::ptree structdescs;
    boost::property_tree::ptree functiondescs;
};

class CollectDeclsAction : public clang::ASTFrontendAction {
public:
    CollectDeclsAction(boost::property_tree::ptree& tree,
                       const ParamList& params)
        : tree(tree), params(params)
    {
    }

    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile)
    {
        return std::unique_ptr<clang::ASTConsumer>(
            new ExtractDataConsumer(&Compiler.getASTContext(), tree, params));
    }

    boost::property_tree::ptree& tree;
    const ParamList& params;
};
