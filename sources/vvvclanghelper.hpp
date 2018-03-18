#ifndef VVVCLANGHELPER_HPP
#define VVVCLANGHELPER_HPP
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Lex/Lexer.h>
#include <clang/Tooling/Tooling.h>
#include <fstream>
#include <iostream>
#include <vector>

std::string getComment(const clang::Decl* d);

std::string decl2str(const clang::Decl* d);
std::string decl2str(const clang::Stmt* d, const clang::ASTContext& context);

bool isSystemDecl(const clang::Decl* d);

/**
 * Test is decl is struct or class
 * */
bool isRecord(const clang::Decl* decl);

std::vector<const clang::Decl*>
getDeclarations(const clang::ASTContext& context);

std::vector<const clang::Decl*>
getNonSystemDeclarations(const clang::ASTContext& context);

std::vector<const clang::Decl*>
getMainFileDeclarations(const clang::ASTContext& context);

std::vector<const clang::FunctionDecl*>
filterFunctions(const std::vector<const clang::Decl*>& decls);

std::vector<const clang::RecordDecl*>
filterStructs(const std::vector<const clang::Decl*>& decls);

std::vector<const clang::FieldDecl*>
getStructFields(const clang::RecordDecl* r);
std::vector<const clang::CXXMethodDecl*>
getStructMethods(const clang::RecordDecl* r);

std::vector<const clang::ParmVarDecl*>
getFunctionParams(const clang::FunctionDecl* d);

void printFunction(const clang::FunctionDecl* d);

void printStructure(const clang::CXXRecordDecl* d);

std::vector<const clang::Stmt*> getCompoundStmtChildren(const clang::Stmt* s);

std::string getSourceFromFile(const char* filename);
std::string getDeclName(const clang::NamedDecl* d);

#endif
