#include "collectstructs.h"
#include "vvvptreehelper.hpp"
#include "myparamnames.hpp"

using namespace clang;

void addCommonFieldDecl(boost::property_tree::ptree& field,
                  const clang::FieldDecl* decl)
{
    using boost::property_tree::ptree;
    const auto& name = decl->getNameAsString();
    const auto& typestring = decl->getType().getAsString();
    const auto& comment = getComment((Decl*)decl);
    ptree_add_value(field, "field", name);
    ptree_add_value(field, "type", typestring);
    ptree_add_value(field, "comment", comment); 
}

void addArrayFieldDecl(boost::property_tree::ptree& field,
                       const clang::FieldDecl* decl)
{
    auto t = decl->getType();
    if(!t->isConstantArrayType())
        return;

    std::vector<uint64_t> arraySizeByDimensions;

    while(t->isConstantArrayType()) {
        ConstantArrayType* ca = (ConstantArrayType*)t->getAsArrayTypeUnsafe();
        const auto elemCount = *ca->getSize().getRawData();
        arraySizeByDimensions.push_back(elemCount);
        t = ca->getElementType();
    }

    const auto& elementTypeName = t.getAsString();
    
    using boost::property_tree::ptree;
    ptree arraySize;
    for(const auto& s: arraySizeByDimensions)
        ptree_array_add_values(arraySize, s);

    ptree arrayInfo;
    ptree_add_value(arrayInfo, "elemType", elementTypeName);
    ptree_add_subnode(arrayInfo, "size", arraySize);
    ptree_add_subnode(field, "array", arrayInfo);
}

void printStructDecls(clang::ASTContext& Context,
                      boost::property_tree::ptree& tree,
                      const printStructsParam& params)
{
    const auto declsInMain  = contain(params, PARAM_NAME_MAIN_ONLY)
                                         ? getMainFileDeclarations(Context)
                                         : getNonSystemDeclarations(Context);
    const auto structsDecls = filterStructs(declsInMain);
    using boost::property_tree::ptree;

    for(const auto& d: structsDecls)
    {
        using namespace std;
        const auto& fs = getStructFields( d );
        ptree structdesc;
        ptree fields;
        for(const auto& f: fs) {
            ptree field;
            addCommonFieldDecl(field, f);
            addArrayFieldDecl(field, f);
            ptree_array_add_node(fields, field);
        }
        auto structName = d->getName().str();
        if(structName.empty())
            structName = "unnamed from " +
                         Context.getSourceManager().
                                          getFilename(d->getLocation()).str();

        const auto& structComment = getComment((Decl*)d);

        ptree_add_value(structdesc, "name", structName);
        ptree_add_value(structdesc, "comment", structComment);
        ptree_add_subnode(structdesc, "fields", fields);
        if(!fs.empty())
            ptree_array_add_node(tree, structdesc);
    }
}

