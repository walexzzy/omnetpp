//==========================================================================
//  MSGANALYZER.H - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __OMNETPP_NEDXML_MSGANALYZER_H
#define __OMNETPP_NEDXML_MSGANALYZER_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "errorstore.h"
#include "msgelements.h"
#include "msgtypetable.h"

namespace omnetpp {
namespace nedxml {

struct MsgCompilerOptions;

/**
 * @brief Part of the message compiler. Produces ClassInfo/EnumInfo objects from the ASTNode tree.
 * Assumes object tree has already passed all validation stages (DTD, syntax, semantic).
 *
 * @ingroup CppGenerator
 */
class NEDXML_API MsgAnalyzer
{
  public:
    typedef std::set<std::string> StringSet;
    typedef std::vector<std::string> StringVector;
    typedef MsgTypeTable::Properties Properties;
    typedef MsgTypeTable::FieldInfo FieldInfo;
    typedef MsgTypeTable::ClassInfo ClassInfo;
    typedef MsgTypeTable::EnumItem EnumItem;
    typedef MsgTypeTable::EnumInfo EnumInfo;

  public:
    static StringSet RESERVED_WORDS;

  protected:
    ErrorStore *errors;
    MsgTypeTable *typeTable;
    const MsgCompilerOptions& opts;

  protected:
    void extractFields(ClassInfo& classInfo);
    void analyzeClassOrStruct(ClassInfo& classInfo, const std::string& namespaceName);
    void analyzeFields(ClassInfo& classInfo, const std::string& namespaceName);
    void analyzeField(ClassInfo& classInfo, FieldInfo *field, const std::string& namespaceName);
    void analyzeInheritedField(ClassInfo& classInfo, FieldInfo *field);
    std::string prefixWithNamespace(const std::string& name, const std::string& namespaceName);
    Properties extractProperties(ASTNode *node);
    bool hasSuperclass(ClassInfo& classInfo, const std::string& superclassQName);
    FieldInfo *findSuperclassField(ClassInfo& classInfo, const std::string& fieldName);
    bool hasProperty(const Properties& p, const char *name)  { return (p.find(name) != p.end()); }
    bool getPropertyAsBool(const Properties& p, const char *name, bool defval);
    std::string getProperty(const Properties& p, const char *name, const std::string& defval = std::string());
    std::string makeFuncall(const std::string& var, const std::string& funcTemplate, bool withIndex=false, const std::string& value="");
    std::string lookupExistingClassName(const std::string& name, const std::string& contextNamespace,  ClassInfo *contextClass=nullptr);

  public:
    MsgAnalyzer(const MsgCompilerOptions& opts, MsgTypeTable *typeTable, ErrorStore *errors);
    ~MsgAnalyzer();
    ClassInfo extractClassInfo(ASTNode *node, const std::string& namespaceName); // accepts StructElement, ClassElement, MessageElement, PacketElement
    void ensureAnalyzed(ClassInfo& classInfo);
    void ensureFieldsAnalyzed(ClassInfo& classInfo);
    EnumInfo extractEnumDecl(EnumDeclElement *node, const std::string& namespaceName);
    EnumInfo extractEnumInfo(EnumElement *node, const std::string& namespaceName);
    ClassInfo extractClassInfoFromEnum(EnumElement *node, const std::string& namespaceName);

  private:
    static constexpr const char* ATT_NAME = "name";
    static constexpr const char* ATT_EXTENDS_NAME = "extends-name";

    static constexpr const char* PROP_ACTUALLY = "actually";
    static constexpr const char* PROP_PRIMITIVE = "primitive";
    static constexpr const char* PROP_OPAQUE = "opaque";
    static constexpr const char* PROP_BYVALUE = "byValue";
    static constexpr const char* PROP_DEFAULTVALUE = "defaultValue";
    static constexpr const char* PROP_CPPTYPE = "cppType";
    static constexpr const char* PROP_ARGTYPE = "argType";
    static constexpr const char* PROP_RETURNTYPE = "returnType";
    static constexpr const char* PROP_TOSTRING = "toString";
    static constexpr const char* PROP_FROMSTRING = "fromString";
    static constexpr const char* PROP_MAYBECSTR = "maybe_c_str";
    static constexpr const char* PROP_EXISTINGCLASS = "existingClass";
    static constexpr const char* PROP_DESCRIPTOR = "descriptor";
    static constexpr const char* PROP_OMITGETVERB = "omitGetVerb";
    static constexpr const char* PROP_FIELDNAMESUFFIX = "fieldNameSuffix";
    static constexpr const char* PROP_BEFORECHANGE = "beforeChange";
    static constexpr const char* PROP_IMPLEMENTS = "implements";
    static constexpr const char* PROP_NOPACK = "nopack";
    static constexpr const char* PROP_OWNED = "owned";
    static constexpr const char* PROP_EDITABLE = "editable";
    static constexpr const char* PROP_OVERRIDEGETTER = "overrideGetter";
    static constexpr const char* PROP_OVERRIDESETTER = "overrideSetter";
    static constexpr const char* PROP_ENUM = "enum";
    static constexpr const char* PROP_SIZETYPE = "sizeType";
    static constexpr const char* PROP_SETTER = "setter";
    static constexpr const char* PROP_GETTER = "getter";
    static constexpr const char* PROP_MUTABLEGETTER = "mutableGetter";
    static constexpr const char* PROP_SIZESETTER = "sizeSetter";
    static constexpr const char* PROP_SIZEGETTER = "sizeGetter";
    static constexpr const char* PROP_CUSTOMIZE = "customize";
};

} // namespace nedxml
}  // namespace omnetpp


#endif

