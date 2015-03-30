#include "vvvstlhelper.hpp"
#include "vvvclanghelper.hpp"
#include "vvvsourcegraph.hpp"
#include <functional>
#include <string>


Graph createFlowChart(const clang::FunctionDecl* fdecl)
{
    Graph g;
    const auto functionName = fdecl->getNameAsString();
    
    auto start = addBeginVertex(g, functionName + "\nНачало" );
    auto end   = addEndVertex(  g, functionName + "\nКонец" );

    auto body = getSemanticVertexFromStmt( fdecl->getBody(), g, fdecl->getASTContext() );
    body->expand( start, end, end, end, end );

    return g;
}



class FindNamedClassConsumer : public ASTConsumer
{
    public:
        explicit FindNamedClassConsumer( ASTContext* Context ) /*: Visitor(Context)*/ {}

        virtual void HandleTranslationUnit( ASTContext& Context)
        {
            const auto decls = getDeclarations( Context );
            const auto declsInMain         = filter( decls, [](const Decl* d)
                                                           { return d->getASTContext().getSourceManager().isInMainFile(d->getLocStart()); });
            const auto functionDeclsInMain = filterFunctions( declsInMain );
            Graph bigGraph;
            for( const auto& d : functionDeclsInMain ) {
                auto g = createFlowChart( d );
                boost::copy_graph(g, bigGraph);
                }

            boost::write_graphviz( std::cout, bigGraph, myLabler( bigGraph ), myEdgeLabler(bigGraph) );
            /*const auto recordDeclsInMain = filterStructs( declsInMain );
            for( const auto& d : recordDeclsInMain )
                printStructure( d );*/
        }
};

class FindNamedClassAction : public clang::ASTFrontendAction
{
    public:
        virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile)
        {
            return new FindNamedClassConsumer( &Compiler.getASTContext() );
        }
};


int main(int argc, char** argv)
{
    FindNamedClassAction f;
    if(argc>1) {
        const char* code = getSourceFromFile( argv[1] );
        clang::tooling::runToolOnCode( new FindNamedClassAction, code );}
}

