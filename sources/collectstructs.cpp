#include "collectstructs.h"
using namespace clang;

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
            const auto& name = f->getNameAsString();
            const auto& t    = f->getType().getAsString();
            const auto& comment = getComment((Decl*)f);
            ptree field;
                field.put("field", name);
                field.put("type", t);
                field.put("comment", comment); 
            fields.push_back( std::make_pair("", field));
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

