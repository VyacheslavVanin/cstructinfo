#ifndef VVVSTLHELPER_H
#define VVVSTLHELPER_H
#include <algorithm>
    
template<class C, class P>
inline C& filter( C&& v, P p )
{
    const auto n = std::copy_if( v.begin(), v.end(), v.begin(), p );
    v.resize( std::distance(v.begin(), n) );
    return v;
}

template<class C, class P>
inline C filter( const C& v, P p)
{
    C ret;
    std::copy_if( v.begin(), v.end(), std::back_inserter( ret ), p );
    return ret;
}

#endif
