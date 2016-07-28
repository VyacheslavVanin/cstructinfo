#include <boost/algorithm/string.hpp>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include "collectfunctions.h"
using namespace clang;

std::string
getDoxyBrief(const ASTContext& ctx, const RawComment* comment)
{
    return comment ? comment->getBriefText(ctx)
                   : "";
}


/**
 * Return strings of doxygen comment without comment opening and closing.
 * From eahch line also removed decoration * from begining if exist */
std::vector<std::string> removeDecorations(const std::string& str)
{
    using namespace boost::algorithm;
    std::vector<std::string> ret;
    auto docstring = erase_tail_copy(erase_head_copy(str,3), 2);
    split(ret, docstring, is_any_of("\n"), token_compress_on);
    std::transform( ret.begin(), ret.end(), ret.begin(),
                    [](const std::string& str) 
                    { return boost::algorithm::trim_copy(str);} );
    std::transform( ret.begin(), ret.end(), ret.begin(),
                    [](const std::string& str)
                    { return str[0]=='*' ? erase_head_copy(str,1) 
                                         : str; } ); 
    return ret;
}

std::vector<std::string> splitToTrimmedWords(const std::string& str)
{
    using namespace boost::algorithm;
    std::vector<std::string> splited;

    split(splited, str, is_any_of(" \t"), token_compress_on );
    splited = filter(splited, 
                     [](const std::string& str) {return !str.empty();});

    return splited;
}

/**
 * @param c container
 * @param n number of head elements to exclude
 * @param sep separator to insert
 */
template<class T, typename S>
std::string joinTail(const T& c, size_t n, const S& sep) 
{
    const auto& tail = std::vector<std::string>(c.begin() + n, c.end());
    const auto& comment = boost::algorithm::join(tail, sep);
    return comment;
}

std::map<std::string, std::string>
getDoxyParams(const ASTContext& ctx, const RawComment* rawcomment)
{
    using namespace boost::algorithm;
    std::map<std::string, std::string> ret;
    if(rawcomment == nullptr) return ret;
    
    const SourceManager& sm = ctx.getSourceManager();
    const auto& text = rawcomment->getRawText(sm).str();
    const auto& lines = removeDecorations(text);

    for(const auto& l: lines)
    {
        const auto words = splitToTrimmedWords(l);
        const size_t splitedsize = words.size();
        if(splitedsize < 2) continue;

        const auto& Tag = words[0];
        if(Tag == "@param" && splitedsize>2)
        {
            const auto& paramName = words[1];
            const auto& comment = joinTail(words, 2, " ");
            ret[paramName] = comment;
        }
        else if(Tag == "@return")
        {
            const auto& comment = joinTail(words, 1, " ");
            ret["return"] = comment;
        }
    }

    return ret;
}

void printFunctionDecls(clang::ASTContext& Context, boost::property_tree::ptree& tree)
{
    const auto declsInMain = getNonSystemDeclarations(Context);
    const auto functionsDecls = filterFunctions(declsInMain);
    using boost::property_tree::ptree;

    for(const auto& d: functionsDecls)
    {
        using namespace std;
        const auto& fs = getFunctionParams( d );
        ptree functiondesc;
        ptree params;
        const RawComment* rc = Context.getRawCommentForDeclNoCache(d);
        const auto& brief    = getDoxyBrief(Context, rc);
        auto paramsComments  = getDoxyParams(Context, rc);
        for(const auto& f: fs) {
            const auto& name = f->getNameAsString();
            const auto& t    = f->getType().getAsString();
            //const auto& comment = getComment((Decl*)f);
            ptree param;
                param.put("param", name);
                param.put("type", t);
                const std::string& comment = paramsComments.count(name) ? paramsComments[name] : "";
                param.put("comment", comment); 
            params.push_back( std::make_pair("", param));
        }
        auto functionName = d->getName().str();

        functiondesc.put( "name", functionName);
        functiondesc.put( "retval", d->getReturnType().getAsString() );
        functiondesc.put( "retcomment", paramsComments["return"]);
        functiondesc.put( "comment", brief);
        functiondesc.push_back( std::make_pair("params", params));
        if(!fs.empty()) tree.push_back( std::make_pair("",functiondesc));
    }
}

