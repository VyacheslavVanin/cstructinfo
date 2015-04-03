#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <atomic>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include "graphhelper.hpp"
#include "vvvclanghelper.hpp"
#include <clang/AST/AST.h>

#define RUK_CONDITION_PREFIX    "ЛО."
#define RUK_OPERTAOR_PREFIX     "ВП."
#define RUK_LOOP_PREFIX         "Ц."
#define RUK_SUBPROGRAM_PREFIX   "ПП."
#define RUK_TRUE_BRANCH_TEXT    "да"
#define RUK_FALSE_BRANCH_TEXT   "нет" 

enum class SEMANTIC_BLOCK_TYPE : int
{
    SIMPLE,
    IF,
    CALL,
    RETURN,
    BREAK,
    CONTINUE,
    COMPOUND,
    SIMPLECOMPOUND,
    FOR,
    WHILE,
    DOWHILE,
    LOOPCLOSE,
    SWITCH,
    CASE,
    DEFAULT
};

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

class GraphData
{
    public:
        operatorTableType   operatorTable;
    private:
        uint32_t mutable    conditionNumerator = 0;
        uint32_t mutable    operatorNumerator = 0;
        uint32_t mutable    loopNumerator = 0;
        uint32_t mutable    subprogramNumerator = 0;

    public: 
        std::string getConditionLabel() const { return std::string(RUK_CONDITION_PREFIX) + std::to_string(++conditionNumerator); }
        std::string getOperatorLabel()  const { return std::string(RUK_OPERTAOR_PREFIX)  + std::to_string(++operatorNumerator); }
        std::string getLoopLabel()      const { return std::string(RUK_LOOP_PREFIX)      + std::to_string(++loopNumerator); }
        std::string getSubprogramLabel()const { return std::string(RUK_SUBPROGRAM_PREFIX)+ std::to_string(++subprogramNumerator); }
};

class VertexData{
    public:
        VertexData() : label(), id( ++counter ) {}
        uint64_t getID() const {return id;}
        virtual ~VertexData(){};
        virtual std::string getShape(){ return "Circle";}

        std::string         label;
        //SEMANTIC_BLOCK_TYPE type;
    private:
        static std::atomic_uint_least64_t counter;
        uint64_t id;
};

struct EdgeData{
    std::string text;
};

using Graph     = boost::adjacency_list<boost::listS,boost::vecS,boost::directedS, std::shared_ptr<VertexData>, EdgeData, GraphData>;
using vertex_t  = boost::graph_traits<Graph>::vertex_descriptor; 
using edge_t    = boost::graph_traits<Graph>::edge_descriptor; 



Graph createFlowChart(const clang::FunctionDecl* fdecl);
class operatorVisitor : public boost::default_dfs_visitor
{
    public:
        operatorVisitor(operatorTableType& operatorTable) : operatorTable(operatorTable) {}
        void discover_vertex(vertex_t v, const Graph& g) const
        {
            /*const auto &currentVertex = g[v];
            auto id = currentVertex->getID();
            auto contents = currentVertex->contents;
            std::string label; // TODO: generate label
            operatorTable.insert( std::make_pair(id, OperatorDescriptor(label, contents)) );
            */
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


class BlockBegin : public VertexData {
    public: virtual  std::string getShape()override { return "ellipse";} };

class BlockEnd : public VertexData {
    public: virtual  std::string getShape()override { return "ellipse";} };

class BlockProcess: public VertexData {
    public: virtual  std::string getShape()override { return "rectangle";} };

class BlockCondition: public VertexData {
    public: virtual  std::string getShape()override { return "diamond";} };

class BlockLoopOpen : public VertexData {
    public: virtual  std::string getShape()override { return "trapezium";} };

class BlockLoopClose : public VertexData {
    public: virtual  std::string getShape()override { return "invtrapezium";} };



template<class C>
inline 
std::pair<vertex_t, C*> addFlowchartVertex( Graph& g, const std::string& text = "")
{
    vertex_t ret = add_vertex( g );
    auto p = new C();
    g[ret].reset( p );
    g[ret]->label = text;
    return std::make_pair(ret, p);
}

inline
vertex_t addProcessVertex(Graph& g, const std::string& text = "")
{
    vertex_t ret = addFlowchartVertex<BlockProcess>(g, text).first;
    return ret;
}

inline
vertex_t addConditionVertex(Graph& g, const std::string& text = "")
{
    vertex_t ret = add_vertex(g);
    g[ret].reset( new BlockCondition() );
    g[ret]->label = text;
    return ret;
}

inline
vertex_t addBeginVertex(Graph& g, const std::string& text="Begin")
{
    vertex_t ret = add_vertex(g);
    g[ret].reset( new BlockBegin() );
    g[ret]->label = text;
    return ret;
}

inline
vertex_t addEndVertex(Graph& g, const std::string& text="End")
{
    vertex_t ret = add_vertex(g);
    g[ret].reset( new BlockEnd() );
    g[ret]->label = text;
    return ret;
}

inline
vertex_t addLoopOpenVertex(Graph& g, const std::string& text="Loop name\ncondition")
{
    vertex_t ret = add_vertex(g);
    g[ret].reset( new BlockLoopOpen() );
    g[ret]->label = text;
    return ret;
}

inline
vertex_t addLoopCloseVertex(Graph& g, const std::string& text="Loop name")
{
    vertex_t ret = add_vertex(g);
    g[ret].reset( new BlockLoopClose() );
    g[ret]->label = text;
    return ret;
}



class SemanticVertex
{
    public: 
        explicit SemanticVertex( Graph& g ) :graph(g), vertex(-1) {}
        virtual ~SemanticVertex(){}
        
        virtual vertex_t expand(vertex_t begin, vertex_t end, 
                            vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
                         { return end; }

        vertex_t getVertex()    const {return vertex;}
        virtual SEMANTIC_BLOCK_TYPE getType()  const = 0;
        virtual std::string         toString() const = 0;

    protected:
        Graph& graph;
        vertex_t vertex;
};

class BlockIf : public SemanticVertex
{
    public:
        BlockIf( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::IfStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::IF;   }
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::IfStmt* stmt;
        const clang::ASTContext& context;
};

 class BlockCall: public SemanticVertex
{
    public:
        BlockCall( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::CallExpr*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::CALL;}
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::CallExpr* stmt;
        const clang::ASTContext& context;
};

class BlockReturn: public SemanticVertex
{
    public:
        BlockReturn( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::ReturnStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::RETURN;}
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::ReturnStmt* stmt;
        const clang::ASTContext& context;
};

class BlockBreak: public SemanticVertex
{
    public:
        BlockBreak(Graph& g) :SemanticVertex(g) {};
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::BREAK;}
        std::string         toString() const { return "break"; }
};

class BlockContinue: public SemanticVertex
{
    public:
        BlockContinue(Graph& g) :SemanticVertex(g) {};
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::CONTINUE;}
        std::string         toString() const { return "continue";}
};

class BlockSimple: public SemanticVertex
{
    public:
        BlockSimple( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(stmt), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::SIMPLE;}
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::Stmt* stmt;
        const clang::ASTContext& context;
};

class BlockCompound: public SemanticVertex
{
    public:
        BlockCompound(Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::CompoundStmt*>(stmt)), context(context) {}

        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()   const { return SEMANTIC_BLOCK_TYPE::COMPOUND;}
        std::string         toString()  const { return decl2str( stmt, context ); }

    private:
        const clang::CompoundStmt*  stmt;
        const clang::ASTContext&    context;
};

class BlockSimpleCompound: public SemanticVertex
{
    using uptrSemVert = std::shared_ptr<SemanticVertex>;
    public:
        BlockSimpleCompound( Graph& g, const std::vector<uptrSemVert>& stmts, const clang::ASTContext& context) 
            : SemanticVertex(g), stmts(stmts), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::SIMPLECOMPOUND;}
        std::string         toString() const 
        {
            std::string ret;
            for( const auto& stmt: stmts ) 
                ret += stmt->toString();
            return ret;
        }

    private:
        const std::vector<uptrSemVert> stmts;
        const clang::ASTContext& context;
};

class BlockFor: public SemanticVertex
{
    public:
        BlockFor( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::ForStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::FOR; }
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::ForStmt* stmt;
        const clang::ASTContext& context;
};

class BlockWhile: public SemanticVertex
{
    public:
        BlockWhile( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::WhileStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::WHILE;}
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::WhileStmt* stmt;
        const clang::ASTContext& context;
};

class BlockDoWhile: public SemanticVertex
{
    public:
        BlockDoWhile( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::DoStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::DOWHILE; }
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::DoStmt* stmt;
        const clang::ASTContext& context;
};


class BlockSwitch : public SemanticVertex
{
    public:
        BlockSwitch( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::SwitchStmt*>(stmt)), context(context) {}

        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::SWITCH;}
        std::string         toString() const { return decl2str( stmt, context ); }

    private:
        const clang::SwitchStmt* stmt;
        const clang::ASTContext& context;
};


class BlockCase: public SemanticVertex 
{
    public:
        BlockCase( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::CaseStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::CASE;}
        std::string         toString() const { return decl2str( stmt, context ); }

        const std::pair<std::vector<std::string>, const Stmt*>& getConditions() const;

    protected:
        const clang::CaseStmt* stmt;
        const clang::ASTContext& context;
        mutable std::pair<std::vector<std::string>, const Stmt*> conditions;
};

class BlockDefault: public SemanticVertex 
{
    public:
        BlockDefault( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::DefaultStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::DEFAULT;}
        std::string         toString() const { return decl2str( stmt, context ); }

    protected:
        const clang::DefaultStmt* stmt;
        const clang::ASTContext& context;
};



inline 
std::string escapeQuates(const std::string& str) 
{
    std::string ret;
    for(auto c: str)
        switch(c){
            case '\"': ret += "\\\""; break;
            case '\\': ret += "\\\\"; break;
            default: ret += c; break;}

    return ret;
}



class myLabler 
{
    public:
        myLabler(const Graph& g) : g(g) {}

        template<class V>
        void operator()(std::ostream& out, const V& v) const {
                        out << "[label=\"" << escapeQuates( g[v]->label ) << "\""
                            << " shape=\"" << g[v]->getShape() << "\"" 
                            << "]";}
    private:
    const Graph& g;
};

class myEdgeLabler
{
    public:
        myEdgeLabler(const Graph& g) : g(g) {}
            
        template<class E>
        void operator()(std::ostream& out, const E& e) const {
                        out << "[label=\"" << g[e].text << "\"]";}

    private:
        const Graph& g;
};

class myGraphPropertyWriter
{
    public:
        void operator()(std::ostream& out) const
        {
            out << "graph [fontname = \"monospace\"]" <<std::endl;
            out << "node [fontname = \"monospace\"]" <<std::endl;
            out << "edge [fontname = \"monospace\"]" <<std::endl;
        }
};

std::shared_ptr<SemanticVertex> 
getSemanticVertexFromStmt(const clang::Stmt* stmt, Graph& graph, const clang::ASTContext& context);








