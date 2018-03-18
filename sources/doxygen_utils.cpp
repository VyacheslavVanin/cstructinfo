#include "doxygen_utils.hpp"
#include "vvvclanghelper.hpp"
#include <boost/algorithm/string.hpp>
#include "stdhelper/containerhelper.hpp"

using namespace clang;
using namespace vvv::helpers;

/**
 * Return strings of doxygen comment without comment opening and closing.
 * From eahch line also removed decoration * from begining if exist */
std::vector<std::string> removeDecorations(const std::string& str)
{
    using namespace boost::algorithm;
    std::vector<std::string> ret;
    auto docstring = erase_tail_copy(erase_head_copy(str, 3), 2);
    split(ret, docstring, is_any_of("\n"), token_compress_on);
    std::transform(ret.begin(), ret.end(), ret.begin(), [](const auto& str) {
        return boost::algorithm::trim_copy(str);
    });
    std::transform(ret.begin(), ret.end(), ret.begin(), [](const auto& str) {
        return str[0] == '*' ? erase_head_copy(str, 1) : str;
    });
    return ret;
}

std::vector<std::string> splitToTrimmedWords(const std::string& str)
{
    using namespace boost::algorithm;
    std::vector<std::string> splited;

    split(splited, str, is_any_of(" \t"), token_compress_on);
    return filter(splited, [](const auto& str) { return !str.empty(); });
}

/**
 * Function join elements from 'c' using sep omitting n first elements.
 * @param c container
 * @param n number of head elements to exclude
 * @param sep separator to insert
 */
template <class T, typename S>
auto joinTail(const T& c, size_t n, const S& sep)
{
    const auto& tail = std::vector<std::string>(c.begin() + n, c.end());
    const auto& comment = boost::algorithm::join(tail, sep);
    return comment;
}

std::map<std::string, std::string> getDoxyParams(const ASTContext& ctx,
                                                 const RawComment* rawcomment)
{
    std::map<std::string, std::string> ret;
    if (rawcomment == nullptr)
        return ret;

    const SourceManager& sm = ctx.getSourceManager();
    const auto& text = rawcomment->getRawText(sm).str();
    const auto& lines = removeDecorations(text);

    for (const auto& l : lines) {
        static const auto paramTags = {"@param", "\\param"};
        static const auto returnTags = {"@return", "\\return"};

        const auto words = splitToTrimmedWords(l);
        const size_t splitedsize = words.size();
        if (splitedsize < 2)
            continue;

        const auto& Tag = words[0];
        if (contain(paramTags, Tag) && splitedsize > 2) {
            const auto& paramName = words[1];
            const auto& comment = joinTail(words, 2, " ");
            ret[paramName] = comment;
        }
        else if (contain(returnTags, Tag)) {
            const auto& comment = joinTail(words, 1, " ");
            ret["return"] = comment;
        }
    }

    return ret;
}

FunctionComments getDoxyComments(const clang::FunctionDecl* d)
{
    const auto& Context = d->getASTContext();
    const RawComment* rc = Context.getRawCommentForDeclNoCache(d);
    FunctionComments ret;
    ret.brief = getComment(d);
    ret.params = getDoxyParams(Context, rc);
    return ret;
}

static const std::string empty = "";
const std::string&
FunctionComments::getParam(const std::string& param_name) const
{
    const auto it = params.find(param_name);
    if (it == params.end())
        return empty;
    return it->second;
}

const std::string&
FunctionComments::getReturn() const
{
    static const std::string empty = "";
    const auto it = params.find("return");
    if (it == params.end())
        return empty;
    return it->second;
}
