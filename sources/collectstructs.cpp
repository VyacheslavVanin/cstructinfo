#include "collectstructs.h"
#include "vvvptreehelper.hpp"
#include "myparamnames.hpp"
#include "collectfunctions.h"

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
    if (!t->isConstantArrayType())
        return;

    std::vector<uint64_t> arraySizeByDimensions;

    while (t->isConstantArrayType()) {
        ConstantArrayType* ca = (ConstantArrayType*)t->getAsArrayTypeUnsafe();
        const auto elemCount = *ca->getSize().getRawData();
        arraySizeByDimensions.push_back(elemCount);
        t = ca->getElementType();
    }

    const auto& elementTypeName = t.getAsString();
    
    using boost::property_tree::ptree;
    ptree arraySize;
    for (const auto& s: arraySizeByDimensions)
        ptree_array_add_values(arraySize, s);

    ptree arrayInfo;
    ptree_add_value(arrayInfo, "elemType", elementTypeName);
    ptree_add_subnode(arrayInfo, "size", arraySize);
    ptree_add_subnode(field, "array", arrayInfo);
}

void addBitfieldDecl(boost::property_tree::ptree& field,
                     const clang::FieldDecl* decl)
{
    if (!decl->isBitField())
        return;
    const auto& astctx = decl->getASTContext();
    const auto bitfieldWidth = decl->getBitWidthValue(astctx);
    using boost::property_tree::ptree;
    ptree_add_value(field, "bitfieldWidth", bitfieldWidth);
}

void addSizeIfBasic(boost::property_tree::ptree& field,
                    const clang::FieldDecl* decl)
{
    auto type = decl->getType();
    if (!type->isBuiltinType())
        return;

    using boost::property_tree::ptree;
    const auto& astctx = decl->getASTContext();
    ptree_add_value(field, "builtin", astctx.getTypeSize(type));
}

boost::property_tree::ptree
makeStructDescriptionNode(const clang::RecordDecl* d, bool needSizes)
{
    using namespace std;
    using boost::property_tree::ptree;

    ptree fields;
    for (const auto& f: getStructFields(d)) {
        ptree field;
        addCommonFieldDecl(field, f);
        addArrayFieldDecl(field, f);
        addBitfieldDecl(field, f);
        if (needSizes)
            addSizeIfBasic(field, f);
        ptree_array_add_node(fields, field);
    }

    ptree methods;
    for (const auto& m: getStructMethods(d)) {
        ptree method = makeFunctionDescriptionNode(m);
        ptree modifiers;
        if (m->isStatic())
            ptree_array_add_values(modifiers, "static");
        if (m->isConst())
            ptree_array_add_values(modifiers, "const");
        if (m->isVirtual())
            ptree_array_add_values(modifiers, "virtual");
        if (m->isPure())
            ptree_array_add_values(modifiers, "pure");

        ptree_add_subnode(method, "modifiers", modifiers);

        ptree_array_add_node(methods, method);
    }

    ptree structdesc;
    ptree_add_value(structdesc, "name", getDeclName(d));
    ptree_add_value(structdesc, "comment", getComment((Decl*)d));
    ptree_add_subnode(structdesc, "fields", fields);
    ptree_add_subnode(structdesc, "methods", methods);
    return structdesc;
}

void printStructDecls(clang::ASTContext& Context,
                      boost::property_tree::ptree& tree,
                      const printStructsParam& params)
{
    using namespace std;
    using boost::property_tree::ptree;

    const auto declsInMain  = contain(params, PARAM_NAME_MAIN_ONLY)
                                      ? getMainFileDeclarations(Context)
                                      : getNonSystemDeclarations(Context);
    const auto needSizes = !contain(params, PARAM_NAME_NO_SIZES);

    for (const auto& d: filterStructs(declsInMain)) {
        const ptree structdesc = makeStructDescriptionNode(d, needSizes);
        ptree_array_add_node(tree, structdesc);
    }
}

