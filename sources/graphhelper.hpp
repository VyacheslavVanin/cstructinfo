#ifndef GRAPHHELPER_H
#define GRAPHHELPER_H
#include <string>
#include <algorithm>
#include <functional>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/graphviz.hpp>
#include "vvvstlhelper.hpp"


template<class GRAPH>
inline std::vector<typename boost::graph_traits<GRAPH>::vertex_descriptor>
        getAdjacentVertices(const GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor index)
{
    std::vector<typename boost::graph_traits<GRAPH>::vertex_descriptor> ret;
    const auto ir = boost::adjacent_vertices( index, g);
    std::copy( ir.first, ir.second, std::back_inserter(ret) );
    return ret;
}

template<class GRAPH>
inline std::vector<typename boost::graph_traits<GRAPH>::vertex_descriptor>
        getPointingVertices(const GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor index)
{
    using edge_t = typename boost::graph_traits<GRAPH>::edge_descriptor;
    using vertex_t = typename boost::graph_traits<GRAPH>::vertex_descriptor;

    std::vector<vertex_t> ret;
    const auto es = boost::edges(g);
    std::for_each( es.first, es.second, [&ret,&index](edge_t e)
                   { if(e.m_target == index) ret.push_back(e.m_source); });

    return ret;
}

template<class GRAPH>
inline bool isVertexTerminal(const GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor v)
{
    const auto a = boost::adjacent_vertices( v, g ); 
    return (std::distance(a.first, a.second) == 0) ? true:
                                                     false;
}

template<class GRAPH>
inline bool isVertexHasPointer(const GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor v)
{
    using edge_t = typename boost::graph_traits<GRAPH>::edge_descriptor;
    const auto es = boost::edges(g);
    return std::any_of( es.first, es.second, 
                        [&g,&v](edge_t e){ return e.m_target == v; } );
}

template<class GRAPH>
inline std::vector<typename boost::graph_traits<GRAPH>::vertex_descriptor> 
getStartingVertices(const GRAPH& g)
{
    using vertex_t = typename boost::graph_traits<GRAPH>::vertex_descriptor;
    std::vector<vertex_t> ret;
    const auto vlist = boost::vertices( g );
    std::copy_if( vlist.first, vlist.second,  //from
                  std::back_inserter(ret),    //to
                  [&g](vertex_t v){ return !isVertexHasPointer(g,v);}); //condition

    return ret;
}

template<class GRAPH>
inline std::vector<typename boost::graph_traits<GRAPH>::vertex_descriptor> 
getTerminalVertices(const GRAPH& g)
{
    using vertex_t = typename boost::graph_traits<GRAPH>::vertex_descriptor;
    std::vector<vertex_t> ret;
    const auto vlist = boost::vertices( g );
    std::copy_if( vlist.first, vlist.second, // from
                  std::back_inserter( ret ), // to
                  [&g](vertex_t v){ return isVertexTerminal(g,v);}); // condition
    return ret;
}

template<class GRAPH>
inline typename boost::graph_traits<GRAPH>::vertex_descriptor 
insertBetween( GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor first, 
                         typename boost::graph_traits<GRAPH>::vertex_descriptor second, 
                         typename boost::graph_traits<GRAPH>::vertex_descriptor newVertex ) 
{
    const typename boost::graph_traits<GRAPH>::vertex_descriptor ret = newVertex;
    boost::remove_edge( first, second, g);
    boost::add_edge( first, ret,  g);
    boost::add_edge( ret, second, g);
    return ret;
}

template<class GRAPH>
inline typename boost::graph_traits<GRAPH>::vertex_descriptor 
insertBetween( GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor first, 
                         typename boost::graph_traits<GRAPH>::vertex_descriptor second ) 
{
    return insertBetween( g, first, second, add_vertex(g) );
}

template<class GRAPH>
inline void connectMto1(GRAPH& g, 
                        const std::vector<typename boost::graph_traits<GRAPH>::vertex_descriptor>& src,
                        typename boost::graph_traits<GRAPH>::vertex_descriptor dst )
{
    for(auto v: src)
        boost::add_edge(v, dst, g);
}


template<class GRAPH>
inline void insertGraphInsteadOf( GRAPH& dst, const GRAPH& src, 
                                  typename boost::graph_traits<GRAPH>::vertex_descriptor index)
{
    const auto dstSends     = getPointingVertices( dst, index );
    const auto dstReturns   = getAdjacentVertices( dst, index );
    const auto dstVertices  = boost::vertices(dst);
    const auto srcToDstOffset = 
                 std::distance( dstVertices.first, dstVertices.second );

    auto srcInputVertices  = getStartingVertices( src );
    auto srcOutputVertices = getTerminalVertices( src );
   
    boost::copy_graph(src, dst);

    for(auto& v: srcInputVertices )  // map indeces from src to dst 
        v += srcToDstOffset;

    for(auto& v: srcOutputVertices ) // map indeces from src to dst 
        v += srcToDstOffset;

    for( auto t: srcInputVertices )   // connect each Send to each Input
        connectMto1( dst, dstSends, t);

    for( auto t: dstReturns )   // connect each Output to each Return
        connectMto1( dst, srcOutputVertices, t );

    for( auto t: dstSends)
        boost::remove_edge( t, index, dst );

    boost::remove_vertex( index, dst ); 
}

template<class GRAPH>
inline void attachGraph( GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor to,
                         GRAPH& newGraph)
{
    auto srcInputVertices     = getStartingVertices( newGraph );
    const auto srcToDstOffset = boost::num_vertices( g );
    
    boost::copy_graph( newGraph, g ); 

    for(auto& v: srcInputVertices )  // map indeces from src to dst 
        v += srcToDstOffset;

    for(auto t: srcInputVertices )
        boost::add_edge(to, t, g);
}

template<class GRAPH>
inline void insertBetween( GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor first, 
                         typename boost::graph_traits<GRAPH>::vertex_descriptor second, 
                         const GRAPH& newGraph ) 
{
    auto srcInputVertices  = getStartingVertices( newGraph );
    auto srcOutputVertices = getTerminalVertices( newGraph );
    const auto dstVertices  = boost::vertices(g);
    const auto srcToDstOffset = std::distance( dstVertices.first, dstVertices.second );
   
    boost::remove_edge( first, second, g );
    boost::copy_graph( newGraph, g );


    for(auto& v: srcInputVertices )  // map indeces from src to dst 
        v += srcToDstOffset;

    for(auto& v: srcOutputVertices ) // map indeces from src to dst 
        v += srcToDstOffset;
   
    for(auto t: srcInputVertices )
        boost::add_edge( first, t, g);

    for(auto t: srcOutputVertices )
        boost::add_edge( t, second, g);
}

template<class GRAPH>
inline void insertAfter( GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor first,
                         const GRAPH& newGraph )
{
    const auto nextVertices = getAdjacentVertices( g, first );
    switch(nextVertices.size()) {
        case  0: attachGraph( g, first, newGraph );                     break;
        case  1: insertBetween( g, first, nextVertices[0], newGraph );  break;
        default: throw std::logic_error("inserAfter can be performed on vertices with only adjacent vertex\n" 
                                        "                         or on vertices without adjacent vertices"); }
}

template<class GRAPH>
inline void reverseOutEdgesOrder( GRAPH& g, typename boost::graph_traits<GRAPH>::vertex_descriptor v )
{
    using vertex_t      = typename boost::graph_traits<GRAPH>::vertex_descriptor;
    using edge_t        = typename boost::graph_traits<GRAPH>::edge_descriptor;
    using edgeprop_t    = typename GRAPH::edge_property_type;
    using edgearray_type= std::vector< std::tuple<vertex_t, vertex_t, edgeprop_t>>;
    const auto edges = boost::out_edges( v, g );
    const auto begin = edges.first;
    const auto end   = edges.second;

    edgearray_type edgearray; 
    std::for_each( begin, end, [&edgearray, &g](const edge_t e)
                               { edgearray.push_back( std::make_tuple( e.m_source, e.m_target, g[e] ) ); }  );

    for( const auto& e : edgearray )
        boost::remove_edge( std::get<0>(e), std::get<1>(e), g); 

    std::for_each( edgearray.rbegin(), edgearray.rend(), [&g, &edgearray](const typename edgearray_type::value_type& e ) {
                                                         auto newEdge = boost::add_edge( std::get<0>(e), std::get<1>(e), g).first;
                                                         g[ newEdge ] = std::get<2>(e); } );
}



#endif
