#include "clanghelper/arghelper.hpp"
#include "clanghelper/declprocessor.hpp"
#include "clanghelper/doxygen_utils.hpp"
#include "clanghelper/stdhelper/containerhelper.hpp"
#include <iostream>
#include <json/json.h>
#include <json/writer.h>

namespace {
const std::string PARAM_NAME_MAIN_ONLY = "--main-only";
const std::string PARAM_NAME_NO_FUNCS = "--no-functions";
const std::string PARAM_NAME_NO_STRUCTS = "--no-structs";
const std::string PARAM_NAME_NO_SIZES = "--no-sizes";
const std::string PARAM_NAME_HELP = "--help";
const std::string PARAM_NAME_WITH_SOURCE = "--with-source";

const std::vector<std::string> ALL_PARAMS = {
    PARAM_NAME_MAIN_ONLY, PARAM_NAME_NO_FUNCS, PARAM_NAME_NO_STRUCTS,
    PARAM_NAME_NO_SIZES,  PARAM_NAME_HELP,     PARAM_NAME_WITH_SOURCE};

template <typename T>
inline Json::Value to_json(const std::vector<T>& v);
inline Json::Value to_json(uint64_t v) { return v; }

template <typename T>
inline Json::Value to_json(const std::vector<T>& v)
{
    Json::Value ret(Json::arrayValue);
    for (const auto& e : v)
        ret.append(to_json(e));
    return ret;
}

} // namespace

using vvv::helpers::contain;

struct DeclCollector {
    DeclCollector(int argc, const char** argv)
        : args(argc, argv, ALL_PARAMS), functions(Json::arrayValue),
          structs(Json::arrayValue),
          main_only(contain(args.getCustomFlags(), PARAM_NAME_MAIN_ONLY)),
          with_funcs(!contain(args.getCustomFlags(), PARAM_NAME_NO_FUNCS)),
          with_structs(!contain(args.getCustomFlags(), PARAM_NAME_NO_STRUCTS)),
          with_sizes(!contain(args.getCustomFlags(), PARAM_NAME_NO_SIZES)),
          with_source(contain(args.getCustomFlags(), PARAM_NAME_WITH_SOURCE))
    {
    }

    void operator()(const clang::Decl* decl)
    {
        using namespace vvv::helpers;

        if (isSystemDecl(decl))
            return;
        if (main_only && !isInMainFile(decl))
            return;

        if (with_funcs && isNonTemplateFunction(decl)) {
            const auto* func = decl->getAsFunction();
            addFunction(func);
            return;
        }
        if (with_structs && isRecord(decl)) {
            addRecord(static_cast<const clang::RecordDecl*>(decl));
            return;
        }
    }

    Json::Value MakeFunctionParams(const clang::FunctionDecl* func,
                                   const FunctionComments& docs)
    {
        auto ret = Json::Value(Json::arrayValue);
        for (const auto& param : getFunctionParams(func)) {
            const auto& name = getDeclName(param);
            Json::Value param_node;
            param_node["name"] = name;
            param_node["type"] = param->getType().getAsString();
            param_node["comment"] = docs.getParam(name);
            ret.append(std::move(param_node));
        }
        return ret;
    }

    Json::Value to_json(const clang::FunctionDecl* func)
    {
        const auto& name = getDeclName(func);
        const auto& docs = getDoxyComments(func);
        Json::Value ret;
        ret["name"] = name;
        ret["comment"] = docs.brief;
        ret["rettype"] = func->getReturnType().getAsString();
        ret["retcomment"] = docs.getReturn();

        ret["params"] = MakeFunctionParams(func, docs);
        ret["location"] = getLocation(func);

        if (with_source)
            ret["source"] = decl2str(func);
        return ret;
    }

    void addFunction(const clang::FunctionDecl* func)
    {
        functions.append(to_json(func));
    }

    void addArrayInfo(Json::Value& out, const clang::FieldDecl* decl)
    {
        using namespace clang;
        auto t = decl->getType();
        if (!t->isConstantArrayType())
            return;

        std::vector<uint64_t> arraySizeByDimensions;

        while (t->isConstantArrayType()) {
            const auto ca = (ConstantArrayType*)t->getAsArrayTypeUnsafe();
            const auto elemCount = *ca->getSize().getRawData();
            arraySizeByDimensions.push_back(elemCount);
            t = ca->getElementType();
        }

        const auto& elementTypeName = t.getAsString();

        Json::Value arraySize = ::to_json(arraySizeByDimensions);
        Json::Value array;
        array["elem_type"] = elementTypeName;
        array["size"] = std::move(arraySize);

        out["array"] = std::move(array);
    }

    void addBitfieldInfo(Json::Value& out, const clang::FieldDecl* decl)
    {
        if (!decl->isBitField())
            return;

        const auto& astctx = decl->getASTContext();
        const auto bitfieldWidth = decl->getBitWidthValue(astctx);
        out["bitfieldWidth"] = bitfieldWidth;
    }

    void addBasicSizeInfo(Json::Value& out, const clang::FieldDecl* decl)
    {
        auto type = decl->getType();
        if (!type->isBuiltinType())
            return;

        const auto& astctx = decl->getASTContext();
        out["builtin"] = astctx.getTypeSize(type);
    }

    Json::Value makeFields(const clang::RecordDecl* decl, bool with_sizes)
    {
        Json::Value ret(Json::arrayValue);
        for (const auto& f : getStructFields(decl)) {
            Json::Value field;
            field["name"] = getDeclName(f);
            field["type"] = f->getType().getAsString();
            field["comment"] = getComment(f);
            addArrayInfo(field, f);
            addBitfieldInfo(field, f);
            if (with_sizes)
                addBasicSizeInfo(field, f);
            ret.append(std::move(field));
        }
        return ret;
    }

    Json::Value makeMethods(const clang::RecordDecl* decl) {
        Json::Value ret(Json::arrayValue);
        for (const auto& f : getStructMethods(decl)) {
            ret.append(to_json(f));
        }
        return ret;
    }

    Json::Value to_json(const clang::RecordDecl* decl)
    {
        Json::Value ret;
        ret["name"] = getDeclName(decl);
        ret["comment"] = getComment(decl);
        ret["location"] = getLocation(decl);
        ret["fields"] = makeFields(decl, with_sizes);
        ret["methods"] = makeMethods(decl);
        if (with_source)
            ret["source"] = decl2str(decl);
        return ret;
    }

    void addRecord(const clang::RecordDecl* decl)
    {
        structs.append(to_json(decl));
    }

    CxxToolArgs args;
    Json::Value functions;
    Json::Value structs;

    bool main_only = false;
    bool with_funcs = true;
    bool with_structs = true;
    bool with_sizes = true;
    bool with_source = false;
};

void printHelpIfNeeded(const std::vector<std::string>& params)
{
    using namespace vvv::helpers;
    static const char* message =
        "cstructinfo usage:\n"
        "cstructinfo [options] <input files ...> [clang compiler options]\n"
        "Options:\n"
        "--main-only - do not process any included files. Process only "
        "specified\n"
        "--no-functions - exclude functions descriptions from output\n"
        "--no-structs - exclude structs descriptions from output\n"
        "--no-sizes - do not add sizeofs of primitive types to structs "
        "descriptions\n"
        "--with-source - add source field containing 'source'"
        " of struct/function\n"
        "--help - show this help\n";
    if (params.empty() || contain(params, PARAM_NAME_HELP)) {
        std::cout << message << std::endl;
        exit(EXIT_SUCCESS);
    }
    return;
}

int main(int argc, const char** argv)
{
    printHelpIfNeeded(argstoarray(argc, argv));

    auto d = DeclCollector(argc, argv);
    visit_decls(argc, argv, [&d](const clang::Decl* decl) { d(decl); },
                ALL_PARAMS);

    Json::Value result;
    result["functions"] = std::move(d.functions);
    result["structs"] = std::move(d.structs);

    Json::StreamWriterBuilder().newStreamWriter()->write(result, &std::cout);
    std::cout << std::endl;
    return 0;
}
