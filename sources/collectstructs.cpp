#include "collectstructs.h"
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
    const auto& t = decl->getType();
    if(!t->isConstantArrayType())
        return;

    ConstantArrayType* ca = (ConstantArrayType*)t->getAsArrayTypeUnsafe();
    const auto elemCount = ca->getSize();
    const auto& elementTypeName = ca->getElementType().getAsString();
    
    using boost::property_tree::ptree;
    ptree arrayInfo;
    arrayInfo.put("elemType", elementTypeName);
    arrayInfo.put("elemCount", elemCount.toString(10, false));
    field.push_back(std::make_pair("array", arrayInfo));
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

