#include "collectdecls.h"
#include "myparamnames.hpp"
#include "vvvptreehelper.hpp"
#include "doxygen_utils.hpp"
#include <string>

boost::property_tree::ptree
makeFunctionDescriptionNode(const clang::FunctionDecl* d, bool needSources)
{
    using namespace std;
    using boost::property_tree::ptree;

    ptree params;
    const auto& comments = getDoxyComments(d);
    for (const auto& f : getFunctionParams(d)) {
        const auto& name = f->getNameAsString();
        const auto& t    = f->getType().getAsString();
        const auto& comment = comments.getParam(name);
        ptree param;
        ptree_add_value(param, "name", name);
        ptree_add_value(param, "type", t);
        ptree_add_value(param, "comment", comment);
        ptree_array_add_node(params, param);
    }

    const auto& sm = d->getASTContext().getSourceManager();
    const auto& locationstring = d->getLocation().printToString(sm);

    ptree functiondesc;
    ptree_add_value(functiondesc, "location", locationstring);
    ptree_add_value(functiondesc, "name", getDeclName(d));
    ptree_add_value(functiondesc, "rettype", d->getReturnType().getAsString());
    ptree_add_value(functiondesc, "retcomment", comments.getReturn());
    ptree_add_value(functiondesc, "comment", comments.brief);
    ptree_add_subnode(functiondesc, "params", params);
    ptree_add_value(functiondesc, "source", needSources ? decl2str(d) : "");
    return functiondesc;
}

void printFunctionDecls(const std::vector<const clang::Decl*>& decls,
                        boost::property_tree::ptree& tree,
                        const ParamList& params)
{
    using namespace std;
    using boost::property_tree::ptree;

    const auto needSources = contain(params, PARAM_NAME_WITH_SOURCE);
    for (const auto& d : filterFunctions(decls)) {
        const ptree functiondesc = makeFunctionDescriptionNode(d, needSources);
        ptree_array_add_node(tree, functiondesc);
    }
}
