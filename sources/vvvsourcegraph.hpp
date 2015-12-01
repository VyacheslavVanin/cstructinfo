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

#define RUK_FUNCTION_BEGIN      "Начало"
#define RUK_FUNCTION_END        "Конец"
#define RUK_CONDITION_PREFIX    "ЛО."
#define RUK_OPERTAOR_PREFIX     "ВП."
#define RUK_LOOP_PREFIX         "Ц."
#define RUK_SUBPROGRAM_PREFIX   "ПП."
#define RUK_TRUE_BRANCH_TEXT    "да"
#define RUK_FALSE_BRANCH_TEXT   "нет" 

enum class LABEL_TYPE : int
{
    OPERATOR,
    CONDITION,
    LOOP,
    SUBPROGRAM
};

enum class VERTEX_TYPE : int
{
    BEGIN,
    END,
    OPERATOR,
    IF,
    SWITCH,
    CALL,
    LOOP_OPEN,
    LOOP_CLOSE
};

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
    LOOP_CLOSE,
    SWITCH,
    CASE,
    DEFAULT
};

class GraphData;
class VertexData;
struct EdgeData;

using Graph     = boost::adjacency_list<boost::listS,boost::vecS,boost::directedS, std::shared_ptr<VertexData>, EdgeData, GraphData>;
using vertex_t  = boost::graph_traits<Graph>::vertex_descriptor; 
using edge_t    = boost::graph_traits<Graph>::edge_descriptor; 


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
        OperatorDescriptor() = default;
        OperatorDescriptor(LABEL_TYPE type, const std::string& label, const std::string& contents) : type(type), label(label), contents(contents) {}
        OperatorDescriptor(LABEL_TYPE type, const std::string& contents) : type(type), contents(contents) {}
        OperatorDescriptor(const OperatorDescriptor&) = default;

        void setLabel(const std::string& str)   { label = str; } 
        LABEL_TYPE getType() const {return type;}

        const std::string& getLabel()    const  { return label;     }
        const std::string& getContents() const  { return contents;  }
    private:
        LABEL_TYPE type;
        std::string label;
        std::string contents;
};

using operatorTableType = std::map<vertex_t, OperatorDescriptor>;

class GraphData
{
    public:
        operatorTableType      operatorTable;
    private:
        uint32_t mutable    conditionNumerator  = 0;
        uint32_t mutable    operatorNumerator   = 0;
        uint32_t mutable    loopNumerator       = 0;
        uint32_t mutable    subprogramNumerator = 0;

    public: 
        std::string getConditionLabel() const { return std::string(RUK_CONDITION_PREFIX) + std::to_string(++conditionNumerator); }
        std::string getOperatorLabel()  const { return std::string(RUK_OPERTAOR_PREFIX)  + std::to_string(++operatorNumerator); }
        std::string getLoopLabel()      const { return std::string(RUK_LOOP_PREFIX)      + std::to_string(++loopNumerator); }
        std::string getSubprogramLabel()const { return std::string(RUK_SUBPROGRAM_PREFIX)+ std::to_string(++subprogramNumerator); }
};

class VertexData{
    public:
        VertexData() : label() {}
        virtual ~VertexData(){};
        virtual std::string getShape() const { return "Circle";}
        virtual vertex_t    getOpenOperator() const { return boost::graph_traits<Graph>::null_vertex(); }
        virtual VERTEX_TYPE getType()const = 0;
        std::string         label;
        int depth = 0;
        int branchesnes = 0;
};

struct EdgeData{
    std::string text;
    std::shared_ptr<int> color = std::shared_ptr<int>(new int);
};




Graph createFlowChart(const clang::FunctionDecl* fdecl);
class LabelVisitor : public boost::default_dfs_visitor
{
    public:
        LabelVisitor(operatorTableType& operatorTable) : operatorTable(operatorTable) {}
        void discover_vertex(vertex_t v, const Graph& g) const
        {
            auto& graphProp = g.m_property;
            auto& currentVertex = g[v];
            auto  openVertex = currentVertex->getOpenOperator();
            if( openVertex == boost::graph_traits<Graph>::null_vertex() )
            {
                if( operatorTable.count(v) == 0 ) return;
                auto& currentDesc = operatorTable.at(v);
                std::string label;
                switch( currentDesc.getType() ) {
                    case LABEL_TYPE::OPERATOR:  label = graphProp->getOperatorLabel();  break;
                    case LABEL_TYPE::CONDITION: label = graphProp->getConditionLabel(); break;
                    case LABEL_TYPE::LOOP:      label = graphProp->getLoopLabel();      break;
                    case LABEL_TYPE::SUBPROGRAM:label = graphProp->getSubprogramLabel(); break; }
                currentDesc.setLabel( label );
                currentVertex->label = label;
            }
            else
            {
                currentVertex->label = operatorTable[ openVertex ].getLabel();
            }
        }

        void tree_edge(edge_t e, const Graph& g) const
        {
            *g[e].color.get() = 1;            
        }

    private:
        operatorTableType& operatorTable;
};



class FunctionDescription
{
    public:
        FunctionDescription(const clang::FunctionDecl* fdecl) 
            : name(fdecl->getNameAsString()), returnType(fdecl->getReturnType().getAsString()),
              flowChart(createFlowChart(fdecl)) 
    {
        for(const auto& p : getFunctionParams(fdecl) ) 
            parameters.push_back( p );

        auto& opTable = flowChart.m_property->operatorTable;
        boost::depth_first_search( flowChart, boost::visitor( LabelVisitor(opTable)) );     
    }

        const Graph& getFlowChart() const {return flowChart;}

    private:
        std::string name;
        std::vector<VariableDescription> parameters;
        std::string returnType;
        Graph flowChart;                        
};


class VertexBegin : public VertexData {
    public: virtual  std::string getShape() const override { return "ellipse";} 
            virtual  VERTEX_TYPE getType()const override {return VERTEX_TYPE::BEGIN;} };

class VertexEnd : public VertexData {
    public: virtual  std::string getShape() const override { return "ellipse";}
            virtual  VERTEX_TYPE getType()const override { return VERTEX_TYPE::END;}};

class VertexProcess: public VertexData {
    public: virtual  std::string getShape() const override { return "rectangle";} 
            virtual  VERTEX_TYPE getType()const override {return VERTEX_TYPE::OPERATOR;}};

class VertexCall: public VertexData {
    public: virtual  std::string getShape() const override { return "rectangle";} 
            virtual  VERTEX_TYPE getType()const override {return VERTEX_TYPE::CALL;}};


class VertexCondition: public VertexData {
    public: virtual  std::string getShape() const override { return "diamond";} }; 

class VertexIf: public VertexCondition {
    public: 
            virtual  VERTEX_TYPE getType()const override {return VERTEX_TYPE::IF; } };

class VertexSwitch: public VertexCondition {
    public: 
            virtual  VERTEX_TYPE getType()const override {return VERTEX_TYPE::SWITCH; } };


class VertexLoopOpen : public VertexData {
    public: virtual  std::string getShape() const override { return "trapezium";} 
            virtual  VERTEX_TYPE getType()const override {return VERTEX_TYPE::LOOP_OPEN; } };

class VertexLoopClose : public VertexData {
    public: VertexLoopClose(vertex_t closeWhat) : closeWhat(closeWhat) {}
            virtual  std::string getShape() const override { return "invtrapezium";} 
            virtual  VERTEX_TYPE getType()const override { return VERTEX_TYPE::LOOP_CLOSE;}
            virtual  vertex_t    getOpenOperator() const override {return closeWhat;}   
    private: vertex_t closeWhat; };


template<class C>
inline 
vertex_t addFlowchartVertex( Graph& g, C* p, int depth, const std::string& text = "")
{
    vertex_t ret = add_vertex( g );
    g[ret].reset( p );
    g[ret]->label = text;
    g[ret]->depth = depth;
    return ret;
}


class SemanticVertex
{
    public: 
        explicit SemanticVertex( Graph& g ) :graph(g), vertex(-1) {}
        virtual ~SemanticVertex(){}
        
        virtual vertex_t expand(vertex_t begin, vertex_t end, 
                                vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth)
                         { return end; }

        vertex_t getVertex()    const {return vertex;}

        void addToTable( LABEL_TYPE t, const std::string& contents ) 
        { 
            graph.m_property->operatorTable[vertex] =  OperatorDescriptor( t, contents); 
        }  

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::BREAK;}
        std::string         toString() const { return "break"; }
};

class BlockContinue: public SemanticVertex
{
    public:
        BlockContinue(Graph& g) :SemanticVertex(g) {};
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::CONTINUE;}
        std::string         toString() const { return "continue";}
};

class BlockSimple: public SemanticVertex
{
    public:
        BlockSimple( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(stmt), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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

        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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

        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

        SEMANTIC_BLOCK_TYPE getType()  const { return SEMANTIC_BLOCK_TYPE::CASE;}
        std::string         toString() const { return decl2str( stmt, context ); }

        const std::pair<std::vector<std::string>, const clang::Stmt*>& getConditions() const;

    protected:
        const clang::CaseStmt* stmt;
        const clang::ASTContext& context;
        mutable std::pair<std::vector<std::string>, const clang::Stmt*> conditions;
};

class BlockDefault: public SemanticVertex 
{
    public:
        BlockDefault( Graph& g, const clang::Stmt* stmt, const clang::ASTContext& context) 
            : SemanticVertex(g), stmt(static_cast<const clang::DefaultStmt*>(stmt)), context(context) {}
        
        vertex_t expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue, int depth);

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
                        out << "[label=\"" << escapeQuates( g[v]->label )
                                   << "\nd: " << g[v]->depth 
                                   << "\nb: " << g[v]->branchesnes
                                   << "\""
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
                        out << "[label=\"" << g[e].text << "\"]";
                        if( *g[e].color.get()==1 ) out << "[color=red]";}
    private:
        const Graph& g;
};

class myGraphPropertyWriter
{
    public:
        void operator()(std::ostream& out) const {
            out << "graph [fontname = \"monospace\"]" <<std::endl;
            out << "node [fontname = \"monospace\"]" <<std::endl;
            out << "edge [fontname = \"monospace\"]" <<std::endl; }
};

std::shared_ptr<SemanticVertex> 
getSemanticVertexFromStmt(const clang::Stmt* stmt, Graph& graph, const clang::ASTContext& context);

/** @brief Calculate branchesnes of each vertex in graph
 *  @return map<Key:Vertex, Value:Branchesnes>   */
std::map<vertex_t, int> calculateBranchesnes( Graph& g );


