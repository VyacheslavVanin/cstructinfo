#include "collectstructs.h"
#include "vvvptreehelper.hpp"
using namespace clang;

void addCommonFieldDecl(boost::property_tree::ptree& field,
                  const clang::FieldDecl* decl)
{
    using boost::property_tree::ptree;
    const auto& name = decl->getNameAsString();
    const auto& typestring = decl->getType().getAsString();
    const auto& comment = getComment((Decl*)decl);
    field.put("field", name);
    field.put("type", typestring);
    field.put("comment", comment); 
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
                      boost::property_tree::ptree& tree)
{
    const auto declsInMain = getNonSystemDeclarations(Context);
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
            fields.push_back(std::make_pair("", field));
        }
        auto structName = d->getName().str();
        if(structName.empty())
            structName = "unnamed from " +
                         Context.getSourceManager().
                                          getFilename(d->getLocation()).str();

        const auto& structComment = getComment((Decl*)d);

        structdesc.put( "name", structName);
        structdesc.put( "comment", structComment);
        structdesc.push_back( std::make_pair("fields", fields));
        if(!fs.empty()) tree.push_back( std::make_pair("",structdesc));
    }
}

