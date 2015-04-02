#include "vvvsourcegraph.hpp"
#include <memory>
using namespace clang;
using namespace std;

std::atomic_uint_least64_t VertexData::counter(0);


shared_ptr<SemanticVertex> 
getSemanticVertexFromStmt(const Stmt* stmt, Graph& graph, const clang::ASTContext& context)
{
    SemanticVertex* ret = 0;
    if(stmt) switch(stmt->getStmtClass()){
                case Stmt::CompoundStmtClass: ret = new BlockCompound(graph, stmt, context);    break;
                case Stmt::IfStmtClass:       ret = new BlockIf(      graph, stmt, context);    break;
                case Stmt::SwitchStmtClass:   ret = new BlockSwitch(  graph, stmt, context);    break;
                case Stmt::CaseStmtClass:     ret = new BlockCase(    graph, stmt, context);    break;
                case Stmt::DefaultStmtClass:  ret = new BlockDefault( graph, stmt, context);    break;
                case Stmt::ForStmtClass:      ret = new BlockFor(     graph, stmt, context);    break;
                case Stmt::WhileStmtClass:    ret = new BlockWhile(   graph, stmt, context);    break;
                case Stmt::DoStmtClass:       ret = new BlockDoWhile( graph, stmt, context);    break;
                case Stmt::CallExprClass:     { auto ce = static_cast<const clang::CallExpr*>(stmt); 
                                                if( isSystemDecl(ce->getCalleeDecl()) )  ret =  new BlockSimple(  graph, stmt, context);
                                                else                                     ret =  new BlockCall(    graph, stmt, context);    
                                                break; }
                case Stmt::ReturnStmtClass:   ret = new BlockReturn(  graph, stmt, context);    break;
                case Stmt::BreakStmtClass:    ret = new BlockBreak(  graph );                   break;
                case Stmt::ContinueStmtClass: ret = new BlockContinue( graph );                 break;
                default:                      ret = new BlockSimple(  graph, stmt, context);    break; }
    return shared_ptr<SemanticVertex>(ret);
}


void BlockIf::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto&       graphProp      = graph.m_property;
    const std::string label    = graphProp->getConditionLabel();
    const std::string contents = decl2str( stmt->getCond(),context );
    
    vertex = addConditionVertex( graph, label );
    boost::add_edge( begin, vertex, graph);
    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );

    auto thenStmt = getSemanticVertexFromStmt( stmt->getThen(), graph, context );
    auto elseStmt = getSemanticVertexFromStmt( stmt->getElse(), graph, context );
    
    thenStmt->expand( vertex, end, onReturn, onBreak, onContinue); 
    if( elseStmt )
        elseStmt->expand( vertex, end, onReturn, onBreak, onContinue); 
    else 
        boost::add_edge( vertex, end, graph);

    auto outedges = boost::out_edges( vertex, graph );
    auto i = outedges.first;
    edge_t trueBranch =  *(i++);
    edge_t falseBranch = *i;
    graph[ trueBranch ].text  = RUK_TRUE_BRANCH_TEXT;
    graph[ falseBranch ].text = RUK_FALSE_BRANCH_TEXT;
}

void BlockCall::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{   
    auto& graphProp = graph.m_property;
    const std::string    label = graphProp->getSubprogramLabel();
    const std::string contents = decl2str( stmt, context );

    vertex = addProcessVertex( graph, label );
    boost::add_edge( begin, vertex, graph );
    boost::add_edge( vertex, end, graph );

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}

void BlockReturn::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp = graph.m_property;
    const std::string    label = graphProp->getOperatorLabel();
    const std::string contents = std::string("return ") + decl2str( stmt->getRetValue(), context );

    vertex = addProcessVertex( graph, label  );
    boost::add_edge( begin, vertex, graph);
    boost::add_edge( vertex, onReturn, graph);

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}

void BlockSimple::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp = graph.m_property;
    const std::string    label = graphProp->getOperatorLabel();
    const std::string contents = decl2str( stmt, context );

    vertex = addProcessVertex( graph, label );
    boost::add_edge( begin, vertex, graph );
    boost::add_edge( vertex, end, graph );

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}


static std::vector<std::shared_ptr<SemanticVertex>> 
groupChildren(const std::vector<const Stmt*>& stmts,
            Graph& graph, const ASTContext& context)
{
    using namespace std;
    using uptrSemVert = shared_ptr<SemanticVertex>;

    std::vector<uptrSemVert> ret;
    std::vector<uptrSemVert> oneBlock;

    auto addOneBlock = [&oneBlock, &ret, &graph, &context]() {
        if(oneBlock.size()) {
            ret.push_back( uptrSemVert(new BlockSimpleCompound(graph, oneBlock, context) ) );
            oneBlock.clear();} };


    for( auto& stmt : stmts ) {
        auto sv = getSemanticVertexFromStmt( stmt, graph, context);
        if (sv->getType() != SEMANTIC_BLOCK_TYPE::SIMPLE) {
            addOneBlock();
            ret.push_back( std::move(sv) );}
        else oneBlock.push_back( std::move(sv) ); }
    addOneBlock();

    return ret;
}

void BlockCompound::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto children = getCompoundStmtChildren( stmt );
    
    const auto groupedChildren = groupChildren(children, graph, context);
    const auto numChildren = groupedChildren.size();
   
    switch(numChildren){
        case 0: boost::add_edge(begin,end,graph); break;
        case 1: groupedChildren[0]->expand(begin,end,onReturn, onBreak, onContinue); break;
        default:
                {
                    size_t i = numChildren-1;
                    groupedChildren[i]->expand(begin,end,onReturn,onBreak,onContinue);
                    boost::remove_edge( begin, groupedChildren[i]->getVertex(), graph);
                    do
                    {
                        --i;
                        groupedChildren[i]->expand(begin,groupedChildren[i+1]->getVertex(),onReturn,onBreak,onContinue);
                        boost::remove_edge( begin, groupedChildren[i]->getVertex(), graph);
                    }while(i!=0);
                    boost::add_edge( begin, groupedChildren[0]->getVertex(), graph);
                }
                break;};
    vertex = groupedChildren[0]->getVertex(); // TODO: if compound block empty there are BUG
}

void BlockSimpleCompound::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp = graph.m_property;
    const std::string    label = graphProp->getOperatorLabel();

    vector<string> v;
    for(const auto& stmt: stmts) v.push_back(stmt->toString());
    const std::string contents = joinStringsWith(v, "\n");
    
    vertex = addProcessVertex( graph, label );
    boost::add_edge( begin, vertex, graph);
    boost::add_edge( vertex,   end, graph);

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}

void BlockFor::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp = graph.m_property;
    const std::string    label = graphProp->getLoopLabel();
    const std::string contents = std::string("for( ") + 
                                 decl2str( stmt->getInit(), context ) + "; " +
                                 decl2str( stmt->getCond(), context ) + "; " +
                                 decl2str( stmt->getInc(), context )  + ")";
 
    // add LoopOpen and LoopClose figures to flowchart 
    //     connect begin     -> LoopOpen and
    //             LoopClose -> end
    vertex             = addLoopOpenVertex( graph, label );
    auto endLoopVertex = addLoopCloseVertex(graph, label );
    boost::add_edge( begin, vertex, graph);
    boost::add_edge( endLoopVertex, end, graph);
    
    // expand loop body
    auto body = getSemanticVertexFromStmt( stmt->getBody(), graph, context);
    body->expand( vertex, endLoopVertex, onReturn, end, endLoopVertex ); 

    // add loop data to operator table
    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}

void BlockWhile::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp = graph.m_property;
    const std::string    label = graphProp->getLoopLabel();
    const std::string contents = decl2str( stmt->getCond(), context);

    vertex              = addLoopOpenVertex( graph, label);
    auto endLoopVertex  = addLoopCloseVertex(graph, label);
    boost::add_edge( begin, vertex, graph);
    boost::add_edge( endLoopVertex, end, graph);
    
    auto body = getSemanticVertexFromStmt( stmt->getBody(), graph, context);
    body->expand( vertex, endLoopVertex, onReturn, end, endLoopVertex ); 

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}

void BlockDoWhile::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp            = graph.m_property;
    const std::string    label = graphProp->getLoopLabel();
    const std::string contents = std::string("do while: ") + decl2str( stmt->getCond(), context);

    vertex             = addLoopOpenVertex( graph, label );
    auto endLoopVertex = addLoopCloseVertex( graph, label);
    boost::add_edge( begin, vertex, graph);
    boost::add_edge( endLoopVertex, end, graph);
    
    auto body = getSemanticVertexFromStmt( stmt->getBody(), graph, context);
    body->expand( vertex, endLoopVertex, onReturn, end, endLoopVertex ); 

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );
}


void BlockSwitch::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto& graphProp            = graph.m_property;
    const std::string    label = graphProp->getConditionLabel();
    const std::string contents = std::string("switch: ") + decl2str( stmt->getCond(), context );

    vertex = addConditionVertex( graph, label ); 
    boost::add_edge( begin, vertex, graph);

    graphProp->operatorTable.insert( std::make_pair( graph[vertex]->getID(), OperatorDescriptor(label,contents)) );

    const auto children = getCompoundStmtChildren( stmt->getBody() );  
    const auto groupedChildren = groupChildren(children, graph, context);
    const auto numChildren = groupedChildren.size();
   
    switch(numChildren){
        case 0: boost::add_edge(vertex,end,graph); break;
        case 1: groupedChildren[0]->expand(vertex,end,onReturn, end, onContinue); break;
        default: {  size_t i = numChildren-1;
                    if(groupedChildren[i]->getType() != SEMANTIC_BLOCK_TYPE::BREAK)
                    {
                        groupedChildren[i]->expand(vertex,end,onReturn,end,onContinue);
                        boost::remove_edge(vertex,groupedChildren[i]->getVertex(), graph);
                    }
                    do {
                        --i;
                        const auto& currentChild  = groupedChildren[i];
                        const auto& nextChild     = groupedChildren[i+1];
                        const auto localend = (nextChild->getType() == SEMANTIC_BLOCK_TYPE::BREAK) ? end :
                                                                                                     nextChild->getVertex();
                        const auto currentChildType = currentChild->getType();
                        if(currentChildType != SEMANTIC_BLOCK_TYPE::BREAK){
                            currentChild->expand(vertex, localend, onReturn, end, onContinue);  
                            if(     currentChildType != SEMANTIC_BLOCK_TYPE::CASE 
                                &&  currentChildType != SEMANTIC_BLOCK_TYPE::DEFAULT) // Only case and default statements left connected to condition block
                                boost::remove_edge(vertex, currentChild->getVertex(), graph); }
                    }while(i!=0);
                }
                break;};
}

static std::pair<vector<string>,const Stmt*> 
getConditions(const CaseStmt* stmt, const ASTContext& context)
{
    std::pair<vector<string>, const Stmt*> ret;
    
    auto currentStmt = stmt;
    for(;;){ 
        ret.first.push_back( decl2str(currentStmt->getLHS(), context) );
        auto subStmt = currentStmt->getSubStmt();
        if( subStmt->getStmtClass() == Stmt::CaseStmtClass )
            currentStmt = static_cast<const CaseStmt*>(subStmt);
        else {
            currentStmt = nullptr;
            ret.second  = subStmt;
            break;} }

    return ret;
}


void BlockCase::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto condition = getConditions( stmt, context );

    auto statements = getSemanticVertexFromStmt( condition.second, graph, context);
         statements->expand( begin, end, onReturn, onBreak, onContinue ); 

    vertex = statements->getVertex();
    boost::remove_edge( begin, vertex, graph );
    auto beginvertex = boost::add_edge( begin, vertex, graph).first;
    graph[beginvertex].text = joinStringsWith( condition.first, ", " ); 
}

void BlockDefault::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    auto statements = getSemanticVertexFromStmt( stmt->getSubStmt(), graph, context);
         statements->expand( begin, end, onReturn, onBreak, onContinue ); 

    vertex = statements->getVertex();
    boost::remove_edge( begin, vertex, graph );
    auto beginvertex = boost::add_edge( begin, vertex, graph).first;
    graph[beginvertex].text = "default";
}


void BlockBreak::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    boost::add_edge( begin, onBreak, graph);
}

void BlockContinue::expand(vertex_t begin, vertex_t end, vertex_t onReturn, vertex_t onBreak, vertex_t onContinue)
{
    boost::add_edge( begin, onContinue, graph);
}



