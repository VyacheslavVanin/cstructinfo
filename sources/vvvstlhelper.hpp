#ifndef VVVSTLHELPER_H
#define VVVSTLHELPER_H
#include <algorithm>
#include <string>
#include <vector>
    
template<typename C, class P,
    typename = typename std::enable_if_t<std::is_rvalue_reference<C&&>::value>
    >
inline decltype(auto) filter(C&& v, P p )
{
    const auto n = std::copy_if( v.begin(), v.end(), v.begin(), p );
    v.resize( std::distance(v.begin(), n) );
    return std::move(v);
}

template<class C, class P>
inline C filter( const C& v, P p)
{
    C ret;
    std::copy_if( v.begin(), v.end(), std::back_inserter( ret ), p );
    return ret;
}

template<class C, class P>
inline bool any_of(const C& c, const P& p)
{
    return std::any_of(c.cbegin(), c.cend(), p);
}

template<class C, class V>
inline bool contain(const C& c, const V& v)
{
    return any_of(c, [&v](const auto& e){return e == v;});
}


inline std::string joinStringsWith( const std::vector<std::string>& v, const std::string& delimiter)
{
    std::string ret;
    const auto numStrings = v.size();    
    const auto lastIndex = numStrings-1;
    for(size_t i = 0; i < numStrings; ++i) {
        ret += v[i];
        if( i!= lastIndex) ret += delimiter; }

    return ret;
}


inline
void split(const std::string& input, char delimiter, std::vector<std::string>& ret)
{
    using namespace std;
    size_t last_pos = 0;
    while(true) {
        const size_t current_pos = input.find(delimiter,last_pos); 
        if(current_pos == string::npos) {
            const string substr = input.substr(last_pos, input.size() - last_pos );  
            ret.push_back(substr);
            break; }
        else {
            const string substr = input.substr(last_pos, current_pos-last_pos); 
            ret.push_back(substr);
            last_pos = current_pos+1; }}
}

inline
std::vector<std::string> split(const std::string& str, char delimiter=' ')
{
    std::vector<std::string> ret;
    split(str, delimiter, ret);
    return ret;
}

#endif

