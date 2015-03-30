#ifndef VVVCLANGHELPER_HPP
#define VVVCLANGHELPER_HPP
#include "vvvstlhelper.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Lex/Lexer.h>
using namespace clang;

template<class D>
inline std::string decl2str(const D* d, const ASTContext& context )
{
    const auto& sm = context.getSourceManager();
    const SourceLocation  b(d->getLocStart()),
                         _e(d->getLocEnd());
    const SourceLocation  e(Lexer::getLocForEndOfToken(_e, 0, sm, context.getLangOpts() ));
    return std::string(sm.getCharacterData(b), sm.getCharacterData(e) - sm.getCharacterData(b));
}

inline 
std::vector<const Decl*> getDeclarations(const ASTContext& context)
{
    std::vector<const Decl*> ret;
    const auto  tu = context.getTranslationUnitDecl();
    const auto  begin = tu->decls_begin();
    const auto  end = tu->decls_end();
    for(auto i = begin; i != end; ++i) 
        ret.push_back( *i );
    return ret;
}

inline 
std::vector<const FunctionDecl*> filterFunctions(const std::vector<const Decl*>& decls)
{
    std::vector<const FunctionDecl*> ret;
    std::for_each( decls.begin(), decls.end(), [&ret](const Decl* d) {
                if( d->isFunctionOrFunctionTemplate() )
                    ret.push_back( dynamic_cast<const FunctionDecl*>(d) ); });
    return ret;
}

inline 
std::vector<const CXXRecordDecl*> filterStructs(const std::vector<const Decl*>& decls)
{
    std::vector<const CXXRecordDecl*> ret;
    const auto fd  = filter( decls, [](const Decl* d) { return d->getKind() == clang::Decl::Kind::CXXRecord; });
    for( const auto& d: fd )
        ret.push_back( dynamic_cast<const CXXRecordDecl*>(d) );
    return ret;
}

inline 
std::vector<const FieldDecl*> getStructFields(const CXXRecordDecl* r)
{
    std::vector<const FieldDecl*> ret;
    const auto b = r->field_begin();
    const auto e = r->field_end();
    for(auto i = b; i != e; ++i)
        ret.push_back( *i );
    return ret;
}

inline 
std::vector<const ParmVarDecl*> getFunctionParams(const FunctionDecl* d)
{
    std::vector<const ParmVarDecl*> ret;
    const auto numParams = d->param_size();
    for(unsigned int i =0; i < numParams; ++i)
        ret.push_back( d->getParamDecl( i ) ); 
    return ret;
}

inline 
void printFunction(const FunctionDecl* d)
{
    using namespace std;
    cout     << "function name: " << d->getNameAsString() << endl
             << "  result type: " << d->getResultType().getAsString() << endl;
    const auto params = getFunctionParams( d );
    for( auto& p: params)
        cout << "  parameter " << p->getNameAsString() << " of type " << p->getType().getAsString() << endl;

    if( d->hasBody() )
        cout << decl2str(d->getBody(), d->getASTContext()) << endl << endl; 
}

inline 
void printStructure( const CXXRecordDecl* d)
{
    using namespace std;
    cout << d->getNameAsString() << endl;
    const auto fs = getStructFields( d );
    for( const auto& f: fs ) {
        const auto name = f->getNameAsString();
        const auto t    = f->getType().getAsString();
        cout << "field: " << name << " of type "<< t << ";" << endl; }
}

inline 
std::vector<const Stmt*> getCompoundStmtChildren(const Stmt* s)
{
    std::vector<const Stmt*> ret;
    const auto b = s->child_begin();
    const auto e = s->child_end();
    for( auto i = b; i != e; ++i )
        ret.push_back( *i );
    return ret;
}



inline 
char* getSourceFromFile(const char* filename)
{
    std::ifstream f(filename, std::ios::binary);
    f.seekg(0, f.end );
    const size_t fsize = f.tellg();
    f.seekg(0, f.beg );
    char* ret = new char[fsize];
    f.read( ret, fsize );
    f.close();
    return ret;
}

#endif

