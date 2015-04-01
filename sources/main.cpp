#include "vvvstlhelper.hpp"
#include "vvvclanghelper.hpp"
#include "vvvsourcegraph.hpp"
#include <functional>
#include <boost/graph/depth_first_search.hpp>
#include <string>


class VariableDescription
{
    public:
        VariableDescription(const clang::VarDecl* decl) : 
            decl(decl), name(decl->getNameAsString()), type(decl->getType().getAsString()) {}

        const std::string& getName() const {return name;}
        const std::string& getType() const {return type;}

    private:
        const clang::VarDecl*   decl;
        std::string             name;
        std::string             type;
};

class OperatorDescriptor
{
    public:
        OperatorDescriptor(const std::string& label, const std::string& contents) : label(label), contents(contents) {}
        OperatorDescriptor(const OperatorDescriptor&) = default;

        const std::string& getLabel()    const  { return label;     }
        const std::string& getContents() const  { return contents;  }
    private:
        std::string label;
        std::string contents;
};

using operatorTableType = std::map<uint64_t, OperatorDescriptor>;

Graph createFlowChart(const clang::FunctionDecl* fdecl);
class operatorVisitor : public boost::default_dfs_visitor
{
    public:
        operatorVisitor(operatorTableType& operatorTable) : operatorTable(operatorTable) {}
        void discover_vertex(vertex_t v, const Graph& g) const
        {
            const auto &currentVertex = g[v];
            auto id = currentVertex->getID();
            auto contents = currentVertex->text;
            std::string label; // TODO: generate label
            operatorTable.insert( std::make_pair(id, OperatorDescriptor(label, contents)) );
        }
    private:
        operatorTableType& operatorTable;
};

class FunctionDescription
{
    public:
        FunctionDescription(const clang::FunctionDecl* fdecl) 
            : name(fdecl->getNameAsString()), returnType(fdecl->getResultType().getAsString()),
              flowChart(createFlowChart(fdecl)) 
    {
        for(const auto& p : getFunctionParams(fdecl) ) 
            parameters.push_back( p );

        // TODO: construct operator table
        boost::depth_first_search( flowChart, boost::visitor(operatorVisitor(operatorTable)) );
    }

    private:
        std::string name;
        std::vector<VariableDescription> parameters;
        std::string returnType;
        Graph flowChart;                                     // Граф блок-схемы
        operatorTableType operatorTable; // Таблица операторов
};


Graph createFlowChart(const clang::FunctionDecl* fdecl)
{
    Graph g;
    const auto functionName = fdecl->getNameAsString();
    auto start = addBeginVertex(g, functionName + "\nНачало" );
    auto end   = addEndVertex(  g, functionName + "\nКонец" );
    auto body  = getSemanticVertexFromStmt( fdecl->getBody(), g, fdecl->getASTContext() );
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

            boost::write_graphviz( std::cout, bigGraph, myLabler( bigGraph ), myEdgeLabler(bigGraph), myGraphPropertyWriter() );
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

