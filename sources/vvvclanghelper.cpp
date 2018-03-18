#include "vvvclanghelper.hpp"
#include "stdhelper/containerhelper.hpp"
#include <sstream>

using namespace clang;
using namespace vvv::helpers;

std::string getComment(const Decl* d)
{
    const ASTContext& ctx      = d->getASTContext();
    const RawComment* rc = ctx.getRawCommentForDeclNoCache(d);
    if (rc)
        return rc->getBriefText(ctx);
    return "";
}

std::string decl2str(const clang::Decl* d)
{
    using namespace clang;
    const auto& context = d->getASTContext();
    const auto& sm = context.getSourceManager();
    const SourceLocation b(d->getLocStart()), _e(d->getLocEnd());
    const SourceLocation e(
        Lexer::getLocForEndOfToken(_e, 0, sm, context.getLangOpts()));
    return std::string(sm.getCharacterData(b),
                       sm.getCharacterData(e) - sm.getCharacterData(b));
}

std::string decl2str(const clang::Stmt* d, const clang::ASTContext& context)
{
    using namespace clang;
    const auto& sm = context.getSourceManager();
    const SourceLocation b(d->getLocStart()), _e(d->getLocEnd());
    const SourceLocation e(
        Lexer::getLocForEndOfToken(_e, 0, sm, context.getLangOpts()));
    return std::string(sm.getCharacterData(b),
                       sm.getCharacterData(e) - sm.getCharacterData(b));
}

bool isSystemDecl(const Decl* d)
{
    return d->getASTContext().getSourceManager().isInSystemHeader(
        d->getLocStart());
}

std::vector<const Decl*> getDeclarations(const ASTContext& context)
{
    std::vector<const Decl*> ret;
    const auto tu    = context.getTranslationUnitDecl();
    const auto begin = tu->decls_begin();
    const auto end   = tu->decls_end();
    for (auto i = begin; i != end; ++i)
        ret.push_back(*i);
    return ret;
}

std::vector<const Decl*> getNonSystemDeclarations(const ASTContext& context)
{
    const auto decls = getDeclarations(context);
    return filter(decls, [](const Decl* d) {
        return !d->getASTContext().getSourceManager().isInSystemHeader(
            d->getLocStart());
    });
}

std::vector<const Decl*> getMainFileDeclarations(const ASTContext& context)
{
    const auto decls = getDeclarations(context);
    return filter(decls, [](const Decl* d) {
        return d->getASTContext().getSourceManager().isInMainFile(
            d->getLocStart());
    });
}

std::vector<const FunctionDecl*>
filterFunctions(const std::vector<const Decl*>& decls)
{
    std::vector<const FunctionDecl*> ret;
    std::for_each(decls.begin(), decls.end(), [&ret](const Decl* d) {
        if (d->isFunctionOrFunctionTemplate() && !d->isTemplateDecl()) {
            const auto funcdecl = static_cast<const FunctionDecl*>(d);
            ret.push_back(funcdecl);
        }
    });
    return ret;
}

bool isRecord(const Decl* decl)
{
    const static auto recordKinds = {clang::Decl::Kind::CXXRecord,
                                     clang::Decl::Kind::Record};
    return contain(recordKinds, decl->getKind());
}

std::vector<const RecordDecl*>
filterStructs(const std::vector<const Decl*>& decls)
{
    std::vector<const RecordDecl*> ret;
    const auto fd = filter(decls, isRecord);
    for (const auto& d : fd)
        ret.push_back(static_cast<const RecordDecl*>(d));
    return ret;
}

std::vector<const FieldDecl*> getStructFields(const RecordDecl* r)
{
    return std::vector<const FieldDecl*>(r->field_begin(), r->field_end());
}

std::vector<const CXXMethodDecl*> getStructMethods(const RecordDecl* r)
{
    std::vector<const CXXMethodDecl*> ret;
    for (auto& i : r->decls())
        if (i->getKind() == Decl::Kind::CXXMethod)
            ret.push_back((const CXXMethodDecl*)i);
    return ret;
}

std::vector<const ParmVarDecl*> getFunctionParams(const FunctionDecl* d)
{
    std::vector<const ParmVarDecl*> ret;
    const auto numParams = d->param_size();
    for (unsigned int i = 0; i < numParams; ++i)
        ret.push_back(d->getParamDecl(i));
    return ret;
}

void printFunction(const FunctionDecl* d)
{
    using namespace std;
    cout << "function name: " << d->getNameAsString() << endl
         << "  result type: " << d->getReturnType().getAsString() << endl;
    const auto params = getFunctionParams(d);
    for (auto& p : params)
        cout << "  parameter " << p->getNameAsString() << " of type "
             << p->getType().getAsString() << endl;

    if (d->hasBody())
        cout << decl2str(d->getBody(), d->getASTContext()) << endl << endl;
}

void printStructure(const CXXRecordDecl* d)
{
    using namespace std;
    cout << d->getNameAsString() << endl;
    const auto fs = getStructFields(d);
    for (const auto& f : fs) {
        const auto name = f->getNameAsString();
        const auto t    = f->getType().getAsString();
        cout << "field: " << name << " of type " << t << ";" << endl;
    }
}

std::vector<const Stmt*> getCompoundStmtChildren(const Stmt* s)
{
    return std::vector<const Stmt*>(s->child_begin(), s->child_end());
}

std::string getSourceFromFile(const char* filename)
{
    using namespace std;
    ifstream f(filename, ios::binary);
    stringstream stream;
    stream << f.rdbuf();
    return stream.str();
}

std::string getDeclName(const clang::NamedDecl* d)
{
    auto name = d->getNameAsString();
    if (name.empty()) {
        const auto& Context = d->getASTContext();
        const auto& sm      = Context.getSourceManager();
        name = "unnamed from " + sm.getFilename(d->getLocation()).str();
    }
    return name;
}
