#include "Klass.h"

#include "autordf/ontology/Ontology.h"

#include <fstream>
#include <iostream>
#include <iterator>

#include "Utils.h"

namespace autordf {
namespace codegen {

Klass Klass::uri2Klass(const std::string& uri) const {
    return Klass(*_decorated.ontology()->findClass(uri));
}

void Klass::generateDeclaration(uint64_t iKlass) const {
    std::string cppName = _decorated.prettyIRIName();
    std::ofstream ofs;
    createFile(genCppNameSpaceInclusionPath() + "/" + cppName + ".h", &ofs);

    generateCodeProtectorBegin(ofs, genCppNameSpaceForGuard(), cppName);

    ofs << "#include <set>" << std::endl;
    ofs << "#include <iosfwd>" << std::endl;
    ofs << "#include <autordf/Object.h>" << std::endl;
    ofs << "#include <All.h>" << std::endl;
    ofs << "#include <" << genCppNameSpaceInclusionPath() << "/I" << cppName << ".h>" << std::endl;
    for ( auto const& ancestor: _decorated.getAllAncestors() ) {
        ofs << "#include <" << uri2Klass(ancestor->rdfname()).genCppNameSpaceInclusionPath() << "/I" <<
                ancestor->prettyIRIName() << ".h>" << std::endl;
    }
    ofs << std::endl;

    enterNameSpace(ofs);
    ofs << std::endl;

    generateComment(ofs, 0);
    ofs << "class " << cppName << ": public autordf::Object";
    for ( auto const& ancestor: _decorated.getAllAncestors() ) {
        ofs << ", public " << uri2Klass(ancestor->rdfname()).genCppNameSpaceFullyQualified() << "::I" <<
                ancestor->prettyIRIName();
    }
    ofs << ", public I" << cppName << " {" << std::endl;
    ofs << "public:" << std::endl;
    ofs << std::endl;
    // indent(ofs, 1) << "/**" << std::endl;
    // indent(ofs, 1) << " * @brief Creates new object, to given iri." << std::endl;
    // indent(ofs, 1) << " * " << std::endl;
    // indent(ofs, 1) << " * If iri empty, creates an anonymous (aka blank) object" << std::endl;
    // indent(ofs, 1) << " */" << std::endl;
    // indent(ofs, 1) << "explicit " << cppName << "(const std::string& iri = \"\");" << std::endl;
    // if ( _decorated.oneOfValues().size() ) {
    //     indent(ofs, 1) << std::endl;
    //     indent(ofs, 1) << "/**" << std::endl;
    //     indent(ofs, 1) << " * @brief Load enum from RDF model, from given C++ Type enum." << std::endl;
    //     indent(ofs, 1) << " * " << std::endl;
    //     indent(ofs, 1) << " * This applies only to classes defines using the owl:oneOf paradigm" << std::endl;
    //     indent(ofs, 1) << " */" << std::endl;
    //     indent(ofs, 1) << "explicit " << cppName << "(I" << cppName << "::Enum enumVal);" << std::endl;
    // }
    // ofs << std::endl;
    // indent(ofs, 1) << "/**" << std::endl;
    // indent(ofs, 1) << " * @brief Build us using the same underlying resource as the other object" << std::endl;
    // indent(ofs, 1) << " */" << std::endl;
    // indent(ofs, 1) << "explicit " << cppName << "(const Object& other);" << std::endl;
    // ofs << std::endl;
    // indent(ofs, 1) << "/**" << std::endl;
    // indent(ofs, 1) << " * @brief Clone the object, to given iri" << std::endl;
    // indent(ofs, 1) << " * " << std::endl;
    // indent(ofs, 1) << " * Object is copied by duplicating all it properties values. " << std::endl;
    // indent(ofs, 1) << " * @param iri if empty, creates an anonymous (aka blank) object." << std::endl;
    // indent(ofs, 1) << " */" << std::endl;
    // indent(ofs, 1) << cppName << " clone(const autordf::Uri& iri = \"\") const {" << std::endl;
    // indent(ofs, 2) <<     "return Object::clone(iri).as<" << cppName << ">();" << std::endl;
    // indent(ofs, 1) << "}" << std::endl;
    // ofs << std::endl;


    indent(ofs, 1) << "explicit " << cppName << "(uint64_t klassId, uint64_t storageIndex): autordf::Object(\"\", \"\", klassId, storageIndex) {" << std::endl;
    int i = 1;
    indent(ofs, 2) << "uint64_t* base_ptr = All::INSTANCES[klassId] + (storageIndex + All::PARENT_OFFSET[klassId][" << cppName << "::KLASS_ID]);" << std::endl;
    for ( auto const& ancestor: _decorated.getAllAncestors() ) {
    	indent(ofs, 2) << "// " << ancestor->prettyIRIName() << " fields" << std::endl;
		for ( const std::shared_ptr<ontology::DataProperty>& prop : ancestor->dataProperties()) {
			i = DataProperty(*prop.get()).generateSetIndices(ofs, Klass(*ancestor.get()), i);
		}
		for ( const std::shared_ptr<ontology::ObjectProperty>& prop : ancestor->objectProperties()) {
			i = ObjectProperty(*prop.get()).generateSetIndices(ofs, Klass(*ancestor.get()), i);
		}
    }
	indent(ofs, 2) << "// " << _decorated.prettyIRIName() << " fields" << std::endl;
	for ( const std::shared_ptr<ontology::DataProperty>& prop : _decorated.dataProperties()) {
		i = DataProperty(*prop.get()).generateSetIndices(ofs, *this, i);
	}
	for ( const std::shared_ptr<ontology::ObjectProperty>& prop : _decorated.objectProperties()) {
		i = ObjectProperty(*prop.get()).generateSetIndices(ofs, *this, i);
	}
    indent(ofs, 1) << "}" << std::endl;
    ofs << std::endl;
    
    indent(ofs, 1) << "/**" << std::endl;
    indent(ofs, 1) << " * @brief This method " << genCppNameSpaceFullyQualified() << "::" << cppName << "::find returns all resources of type " << _decorated.rdfname() << std::endl;
    indent(ofs, 1) << " */" << std::endl;
    indent(ofs, 1) << "static std::vector<" << cppName << "> find();" << std::endl;
    ofs << std::endl;

	size_t sz = storageSize();

    // indent(ofs, 1) << "static " << cppName << " instance(uint64_t klassId, uint64_t storageIndex) {" << std::endl;
    // indent(ofs, 2) << "// FIXME: use klassId" << std::endl;
    // indent(ofs, 2) << "return " << cppName << "(storageIndex * " << sz << ");" << std::endl;
    // indent(ofs, 1) << "}" << std::endl;
    // ofs << std::endl;

    // indent(ofs, 1) << "static std::shared_ptr<" << cppName << "> instancePtr(uint64_t klassId, uint64_t storageIndex) {" << std::endl;
    // indent(ofs, 2) << "// FIXME: use klassId" << std::endl;
    // indent(ofs, 2) << "return std::make_shared<" << cppName << ">(storageIndex * " << sz << ");" << std::endl;
    // indent(ofs, 1) << "}" << std::endl;
    // ofs << std::endl;

    // indent(ofs, 1) << "static std::vector<" << cppName << "> instances(uint64_t* klassIdStorageBegin, uint64_t* identityStorageBegin, uint64_t storageLength) {" << std::endl;
    // indent(ofs, 2) << "std::vector<" << cppName << "> ret;" << std::endl;
    // indent(ofs, 2) << "ret.reserve(storageLength);" << std::endl;
    // indent(ofs, 2) << "for (size_t i=0; i < storageLength; i++) {" << std::endl;
    // indent(ofs, 3) << "ret.push_back(" << cppName << "::instance(*(klassIdStorageBegin + i), *(identityStorageBegin + i)));" << std::endl;
    // indent(ofs, 2) << "}" << std::endl;
    // indent(ofs, 2) << "return ret;" << std::endl;
    // indent(ofs, 1) << "}" << std::endl;
    // ofs << std::endl;

	size_t propIndex = 0;
    for ( const std::shared_ptr<ontology::DataProperty>& key : _decorated.dataKeys()) {
    	DataProperty dp(*key.get());
        dp.generateKeyDeclaration(ofs, _decorated, propIndex, sz);
        propIndex += dp.storageSize(_decorated);
    }

    for ( const std::shared_ptr<ontology::ObjectProperty>& key : _decorated.objectKeys()) {
        ObjectProperty op(*key.get());
        op.generateKeyDeclaration(ofs, _decorated, propIndex, sz);
        propIndex += op.storageSize(_decorated);
    }

	ofs << std::endl;

	indent(ofs, 1) << "static void initInstances(uint64_t* storage, size_t len);\n" << std::endl;


    ofs << "private:" << std::endl;

    // indent(ofs, 1) << "Object& object() override { return *this; }" << std::endl;
    // indent(ofs, 1) << "const Object& object() const override { return *this; }" << std::endl;

	indent(ofs, 1) << "static uint64_t KLASS_ID;" << std::endl;
	indent(ofs, 1) << "static uint64_t* INSTANCES;" << std::endl;
	indent(ofs, 1) << "static size_t INSTANCES_LENGTH;" << std::endl;

    ofs << "};" << std::endl;
    ofs << std::endl;

    ofs << std::endl;

    leaveNameSpace(ofs);

    ofs << "namespace autordf {" << std::endl;
    ofs << "template<> inline " << genCppNameSpaceFullyQualified() << "::" << cppName  << " autordf::Object::as() {" << std::endl;
    indent(ofs, 1) << "return " << genCppNameSpaceFullyQualified() << "::" << cppName << "(_klass, _identity);" << std::endl;
    ofs << "}" << std::endl;
    ofs << "template<> inline bool autordf::Object::isA<" << genCppNameSpaceFullyQualified() << "::" << cppName   << ">() const {" << std::endl;
    indent(ofs, 1) << "return _klass == " << iKlass << ";" << std::endl;
    ofs << "}" << std::endl;
    ofs << "}" << std::endl;

    generateCodeProtectorEnd(ofs, genCppNameSpaceFullyQualified(), cppName);
}

void Klass::generateDefinition(uint64_t iKlass) const {
    std::string cppName = _decorated.prettyIRIName();
    std::string cppNameSpace = genCppNameSpaceFullyQualified();
    std::ofstream ofs;
    createFile(genCppNameSpaceInclusionPath() + "/" + cppName + ".cpp", &ofs);

    startInternal(ofs);
    ofs << "#include <" << genCppNameSpaceInclusionPath() << "/" << cppName << ".h>" << std::endl;
    ofs << std::endl;
    addBoilerPlate(ofs);
    ofs << std::endl;

    enterNameSpace(ofs);
    ofs << std::endl;
    // ofs << cppName << "::" << cppName << "(const std::string& iri) : autordf::Object(iri, I" << cppName << "::TYPEIRI) {" << std::endl;
    // ofs << "}" << std::endl;
    // if ( _decorated.oneOfValues().size() ) {
    //     ofs << std::endl;
    //     ofs << cppName << "::"<< cppName << "(I" << cppName << "::Enum enumVal) : autordf::Object(enumIri(enumVal)) {}" << std::endl;
    // }
    // ofs << std::endl;
    // ofs << cppName << "::" << cppName << "(const Object& other) : autordf::Object(other) {" << std::endl;
    // ofs << "}" << std::endl;
    // ofs << std::endl;

	ofs << "uint64_t " << cppName << "::KLASS_ID = " << iKlass <<";" << std::endl;
	ofs << "uint64_t* " << cppName << "::INSTANCES = nullptr;" << std::endl;
	ofs << "size_t " << cppName << "::INSTANCES_LENGTH = 0;" << std::endl;

	ofs << "void " << cppName << "::initInstances(uint64_t* storage, size_t len) {" << std::endl;
	indent(ofs, 1) << "INSTANCES = storage;" << std::endl;
	indent(ofs, 1) << "INSTANCES_LENGTH = len;" << std::endl;
	indent(ofs, 1) << "All::INSTANCES[" << cppName << "::KLASS_ID] = storage;" << std::endl;
	
	ofs << "}" << std::endl;
	ofs << std::endl;

	size_t objSize = storageSize();

    ofs << "std::vector<" << cppName << "> " << cppName << "::find() {" << std::endl;
    // indent(ofs, 1) << "return findHelper<" << cppName << ">(I" << cppName << "::TYPEIRI);" << std::endl;
    indent(ofs, 1) << "std::vector<" << cppName << "> ret;" << std::endl;
    indent(ofs, 1) << "ret.reserve((INSTANCES_LENGTH - 1) / " << objSize << ");" << std::endl;
    indent(ofs, 1) << "for (size_t i=1; i < INSTANCES_LENGTH; i += " << objSize << ") {" << std::endl;
    indent(ofs, 2) << "ret.emplace_back(" << cppName << "::KLASS_ID, " << "i);" << std::endl;
    indent(ofs, 1) << "}" << std::endl;
    // FIXME: find should return subclasses ?
    indent(ofs, 1) << "return ret;" << std::endl;
    ofs << "}" << std::endl;
    ofs << std::endl;
    leaveNameSpace(ofs);
    ofs << std::endl;
    stopInternal(ofs);
}

void Klass::generateInterfaceDeclaration() const {
    std::string cppName = "I" + _decorated.prettyIRIName();
    std::ofstream ofs;
    createFile(genCppNameSpaceInclusionPath() + "/" + cppName + ".h", &ofs);

    generateCodeProtectorBegin(ofs, genCppNameSpaceForGuard(), cppName);

    if ( _decorated.oneOfValues().size() ) {
        ofs << "#include <array>" << std::endl;
        ofs << "#include <tuple>" << std::endl;
        ofs << "#include <ostream>" << std::endl;
        ofs << std::endl;
    }
    ofs << "#include <autordf/Object.h>" << std::endl;
    ofs << std::endl;

    //get forward declarations
    std::set<std::shared_ptr<const Klass> > cppClassDeps = getClassDependencies();
    for ( const std::shared_ptr<const Klass>& cppClassDep : cppClassDeps ) {
        cppClassDep->enterNameSpace(ofs);
        ofs << "class " << cppClassDep->decorated().prettyIRIName() << ";" << std::endl;
        ofs << "class I" << cppClassDep->decorated().prettyIRIName() << ";" << std::endl;
        cppClassDep->leaveNameSpace(ofs);
    }
    // Add forward declaration for our own class
    enterNameSpace(ofs);
    ofs << "class " << _decorated.prettyIRIName() << ";" << std::endl;
    leaveNameSpace(ofs);
    ofs << std::endl;

    enterNameSpace(ofs);
    ofs << std::endl;

    generateComment(ofs, 0);
    ofs << "class " << cppName << " ";
    ofs << " {" << std::endl;
    ofs << "public:" << std::endl;
    startInternal(ofs, 1);
    indent(ofs, 1) << "/**" << std::endl;
    indent(ofs, 1) << " * IRI for rdfs type associated with this C++ Interface class" << std::endl;
    indent(ofs, 1) << " */ " << std::endl;
    indent(ofs, 1) << "static const char* TYPEIRI;" << std::endl;
    stopInternal(ofs, 1);
    ofs << std::endl;

    if ( _decorated.oneOfValues().size() ) {
        indent(ofs, 1) << "/**" << std::endl;
        indent(ofs, 1) << " * @brief C++ Enum values mapping the owl instances restrictions for this class " << std::endl;
        indent(ofs, 1) << " */ " << std::endl;
        indent(ofs, 1) << "enum Enum {" << std::endl;
        for ( const RdfsEntity& enumVal : _decorated.oneOfValues() ) {
            enumVal.generateComment(ofs, 2);
            indent(ofs, 2) << enumVal.decorated().prettyIRIName() << "," << std::endl;
        }
        indent(ofs, 1) << "};" << std::endl;
        ofs << std::endl;

        indent(ofs, 1) << "/** " << std::endl;
        indent(ofs, 1) << " * @brief Returns the current object as an Enum " << std::endl;
        indent(ofs, 1) << " * " << std::endl;
        indent(ofs, 1) << " * This object is expected to be one the instance allowed by the owl:oneOf definition." << std::endl;
        indent(ofs, 1) << " * @return enum value" << std::endl;
        indent(ofs, 1) << " * @throw InvalidEnum if the object we try to convert is not one of the instances defined by owl:oneOf" << std::endl;
        indent(ofs, 1) << " */ " << std::endl;
        indent(ofs, 1) << "Enum asEnum() const {" << std::endl;
        indent(ofs, 2) << "if (v_enum == 0) {" << std::endl;
        indent(ofs, 3) << "throw autordf::InvalidEnum(\"Object is not a valid enum member\");" << std::endl;
        indent(ofs, 2) << "} else {" << std::endl;
        indent(ofs, 3) << "return static_cast<Enum>(v_enum - 1);" << std::endl;
        indent(ofs, 2) << "}" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
        ofs << std::endl;
        indent(ofs, 1) << "/** " << std::endl;
        indent(ofs, 1) << " * @brief Converts an enum value to a pretty string " << std::endl;
        indent(ofs, 1) << " * " << std::endl;
        indent(ofs, 1) << " * @param en enum value" << std::endl;
        indent(ofs, 1) << " * @return enum value converted as string" << std::endl;
        indent(ofs, 1) << " */ " << std::endl;
        indent(ofs, 1) << "static std::string enumString(Enum en);" << std::endl;
        ofs << std::endl;
        indent(ofs, 1) << "/** " << std::endl;
        indent(ofs, 1) << " * @brief Converts current enum value to a pretty string " << std::endl;
        indent(ofs, 1) << " * " << std::endl;
        indent(ofs, 1) << " * @return current enum value converted as string" << std::endl;
        indent(ofs, 1) << " */ " << std::endl;
        indent(ofs, 1) << "std::string enumString() const { return enumString(asEnum()); }" << std::endl;
    }
    ofs << std::endl;
    for ( const std::shared_ptr<ontology::DataProperty>& prop : _decorated.dataProperties()) {
        DataProperty(*prop.get()).generateDeclaration(ofs, _decorated);
    }

    for ( const std::shared_ptr<ontology::ObjectProperty>& prop : _decorated.objectProperties()) {
        ObjectProperty(*prop.get()).generateDeclaration(ofs, _decorated);
    }
    ofs << std::endl;

    // indent(ofs, 1) << "/**" << std::endl;
    // indent(ofs, 1) << " * @brief returns the object this interface object applies to" << std::endl;
    // indent(ofs, 1) << " **/" << std::endl;
    // indent(ofs, 1) << "virtual autordf::Object& object() = 0;" << std::endl;
    // ofs << std::endl;
    // indent(ofs, 1) << "/**" << std::endl;
    // indent(ofs, 1) << " * @brief returns the object this interface object applies to" << std::endl;
    // indent(ofs, 1) << " **/" << std::endl;
    // indent(ofs, 1) << "virtual const autordf::Object& object() const = 0;" << std::endl;
    // ofs << std::endl;
    ofs << "private:" << std::endl;
    if ( _decorated.oneOfValues().size() ) {
        indent(ofs, 1) << "typedef std::tuple<Enum, const char *, const char *> EnumArrayEntryType;" << std::endl;
        indent(ofs, 1) << "typedef std::array<EnumArrayEntryType, " << _decorated.oneOfValues().size() << "> EnumArrayType;" << std::endl;
        indent(ofs, 1) << "static const EnumArrayType ENUMARRAY;" << std::endl;
        ofs << std::endl;
        indent(ofs, 1) << "static const EnumArrayEntryType& enumVal2Entry(Enum en);" << std::endl;
        ofs << std::endl;
        ofs << "protected:" << std::endl;
        startInternal(ofs, 1);
        indent(ofs, 1) << cppName << "() {}" << std::endl;
        indent(ofs, 1) << cppName << "( const " << cppName << "&) {}" << std::endl;
        indent(ofs, 1) << "static std::string enumIri(Enum en);" << std::endl;
        indent(ofs, 1) << "uint64_t v_enum;" << std::endl;
        stopInternal(ofs, 1);
    }

    ofs << "};" << std::endl;
    leaveNameSpace(ofs);

    ofs << std::endl;
    // if ( _decorated.oneOfValues().size() ) {
    //     std::string cppNameSpace;
    //     if ( outdir != "." ) {
    //         cppNameSpace.append(outdir + "::");
    //     }
    //     cppNameSpace.append(genCppNameSpace() + "::");

    //     ofs << "/**" << std::endl;
    //     ofs << " * Dumps string representation of Enumerated type " << std::endl;
    //     ofs << " */" << std::endl;
    //     ofs << "inline std::ostream& operator<<(std::ostream& os, " << cppNameSpace << cppName << "::Enum val) {" << std::endl;
    //     indent(ofs, 1) << "os << " << cppNameSpace << cppName << "::enumString(val);" << std::endl;
    //     indent(ofs, 1) << "return os;" << std::endl;
    //     ofs << "}" << std::endl;
    // }

    generateCodeProtectorEnd(ofs, genCppNameSpaceFullyQualified(), cppName);
}

void Klass::generateInterfaceDefinition() const {
    std::string cppName = "I" + _decorated.prettyIRIName();

    std::ofstream ofs;
    createFile(genCppNameSpaceInclusionPath() + "/" + cppName + ".cpp", &ofs);

    startInternal(ofs);
    ofs << "#include <" << genCppNameSpaceInclusionPath() << "/" << cppName << ".h>" << std::endl;
    ofs << std::endl;
    addBoilerPlate(ofs);
    ofs << std::endl;
    ofs << "#include <sstream>" << std::endl;
    ofs << "#include <autordf/Exception.h>" << std::endl;
    ofs << std::endl;

    // Generate class imports
    std::set<std::shared_ptr<const Klass> > cppDeps = getClassDependencies();
    for ( const std::shared_ptr<const Klass>& cppDep : cppDeps ) {
        ofs << "#include <" << cppDep->genCppNameSpaceInclusionPath() << "/" << cppDep->decorated().prettyIRIName() << ".h>" << std::endl;
    }
    // Include our own class
    ofs << "#include <" << genCppNameSpaceInclusionPath() << "/" << _decorated.prettyIRIName() << ".h>" << std::endl;
    ofs << std::endl;

    enterNameSpace(ofs);
    ofs << std::endl;

    if ( _decorated.oneOfValues().size() ) {
        ofs << "const " << _decorated.prettyIRIName() << "::EnumArrayType I" << _decorated.prettyIRIName() << "::ENUMARRAY = {" << std::endl;
        ofs << "{";
        for ( const ontology::RdfsEntity& v : _decorated.oneOfValues() ) {
            RdfsEntity enumVal(v);
            indent(ofs, 1) << "std::make_tuple(I" << _decorated.prettyIRIName() << "::" << enumVal.decorated().prettyIRIName() << ", \"" << v.rdfname() << "\", \"" <<
                    enumVal.decorated().prettyIRIName() << "\")," << std::endl;
        }
        ofs << "}" << std::endl;
        ofs << "};" << std::endl;
        ofs << std::endl;

        ofs << "const I" << _decorated.prettyIRIName() << "::EnumArrayEntryType& I" << _decorated.prettyIRIName() << "::enumVal2Entry(Enum enumVal) {" << std::endl;
        indent(ofs, 1) << "for ( auto const& enumItem: ENUMARRAY) {" << std::endl;
        indent(ofs, 2) << "if ( enumVal == std::get<0>(enumItem) ) return enumItem;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
        indent(ofs, 1) << "std::stringstream ss;" << std::endl;
        indent(ofs, 1) << "ss << \"Enum value \" << enumVal << \" is not valid for for C++ enum " << _decorated.prettyIRIName() << "\";" << std::endl;
        indent(ofs, 1) << "throw autordf::InvalidEnum(ss.str());" << std::endl;
        ofs << "};" << std::endl;

        // ofs << std::endl;
        // ofs << "I" << _decorated.prettyIRIName() << "::Enum I" << _decorated.prettyIRIName() << "::asEnum() const {" << std::endl;
        // indent(ofs, 1) << "for ( auto const& enumItem: ENUMARRAY) {" << std::endl;
        // indent(ofs, 2) << "if ( object().iri() == std::get<1>(enumItem) ) return std::get<0>(enumItem);" << std::endl;
        // indent(ofs, 1) << "}" << std::endl;
        // indent(ofs, 1) << "throw autordf::InvalidEnum(object().iri() + \" is not a valid individual for owl:oneOf type " <<
        //         _decorated.prettyIRIName() << "\");" << std::endl;
        // ofs << "}" << std::endl;
        ofs << std::endl;

        ofs << "std::string I" << _decorated.prettyIRIName() << "::enumIri(Enum enumVal) {" << std::endl;
        indent(ofs, 1) << "return std::get<1>(enumVal2Entry(enumVal));" << std::endl;
        ofs << "}" << std::endl;
        ofs << std::endl;

        ofs << "std::string I" << _decorated.prettyIRIName() << "::enumString(Enum enumVal) {" << std::endl;
        indent(ofs, 1) << "return std::get<2>(enumVal2Entry(enumVal));" << std::endl;
        ofs << "}" << std::endl;
        ofs << std::endl;
    }
    ofs << "// This type " << genCppNameSpaceFullyQualified() << "::" << cppName << " has IRI " << _decorated.rdfname() << std::endl;
    ofs << "const char * " << cppName << "::TYPEIRI = \"" << _decorated.rdfname() << "\";" << std::endl;
    ofs << std::endl;

    for ( const std::shared_ptr<ontology::DataProperty>& prop : _decorated.dataProperties()) {
        DataProperty(*prop.get()).generateDefinition(ofs, _decorated);
    }
    for ( const std::shared_ptr<ontology::ObjectProperty>& prop : _decorated.objectProperties()) {
        ObjectProperty(*prop.get()).generateDefinition(ofs, _decorated);
    }
    leaveNameSpace(ofs);
    stopInternal(ofs);
}

std::set<std::shared_ptr<const Klass> > Klass::getClassDependencies() const {
    std::set<std::shared_ptr<const Klass> > deps;
    std::set<std::shared_ptr<const ontology::ObjectProperty> > objects;

    std::copy(_decorated.objectProperties().begin(), _decorated.objectProperties().end(), std::inserter(objects, objects.begin()));
    std::copy(_decorated.objectKeys().begin(), _decorated.objectKeys().end(), std::inserter(objects, objects.begin()));

    for ( const std::shared_ptr<const ontology::ObjectProperty> p : objects) {
        auto val = p->findClass(&_decorated);
        if ( val ) {
            if ( val->prettyIRIName() != _decorated.prettyIRIName() ) {
                deps.insert(std::shared_ptr<Klass>(new Klass(*val.get())));
            }
        } else {
            deps.insert(std::shared_ptr<Klass>(new Klass(*_decorated.ontology()->findClass(autordf::ontology::Ontology::OWL_NS + "Thing"))));
        }
    }

    return deps;
}

void Klass::enterNameSpace(std::ofstream& ofs) const {
    if ( outdir != "." ) {
        ofs << "namespace " << outdir << " {" << std::endl;
    }
    ofs << "namespace " << genCppNameSpace() << " {" << std::endl;
}

void Klass::leaveNameSpace(std::ofstream& ofs) const {
    ofs << "}" << std::endl;
    if ( outdir != "." ) {
        ofs << "}" << std::endl;
    }
}

size_t Klass::storageSize() const {
    int i = 0;
    for ( auto const& ancestor: _decorated.getAllAncestors() ) {
		for ( const std::shared_ptr<ontology::DataProperty>& prop : ancestor->dataProperties()) {
			i += DataProperty(*prop.get()).storageSize(Klass(*ancestor.get()));
		}
		for ( const std::shared_ptr<ontology::ObjectProperty>& prop : ancestor->objectProperties()) {
			i += ObjectProperty(*prop.get()).storageSize(Klass(*ancestor.get()));
		}
    }
    if (i == 0) {
    	i+= 1; // instances can't be empty, even if no property
    }
	return i;
}

void Klass::generateSaverInitKlassIdsIdentities(std::ofstream& ofs, uint64_t iKlass) const {
	indent(ofs, 1) << "{" << std::endl;
	indent(ofs, 2) << "uint64_t identity = 1;" << std::endl;
	indent(ofs, 2) << "for(auto const& obj: " << genCppNameWithNamespace(false) << "::find()) {" << std::endl;
	indent(ofs, 3) << "allObjectKlassIds[obj] = " << iKlass << ";" << std::endl;
	indent(ofs, 3) << "allObjectIdentities[obj] = identity;" << std::endl;
	indent(ofs, 3) << "if (!obj.iri().empty()) {" << std::endl;
	indent(ofs, 4) << "uriToObjects[obj.iri()] = std::make_pair(" << iKlass << ", identity);"  << std::endl;
	indent(ofs, 3) << "}" << std::endl;
	indent(ofs, 3) << "identity += " << storageSize() << ";" << std::endl;
	indent(ofs, 2) << "}" << std::endl;
	indent(ofs, 1) << "}" << std::endl;
}

void Klass::generateSaverInstanceSave(std::ofstream& ofs, uint64_t iKlass) const {
	indent(ofs, 1) << "{ // " << _decorated.prettyIRIName() << std::endl;
	indent(ofs, 1) << "uint64_t identity = 1;" << std::endl;
	indent(ofs, 1) << "for(auto const& obj: " << genCppNameWithNamespace(false) << "::find()) {" << std::endl;
	int propOffset = 0;
	for (auto const& ancestor: decorated().getAllAncestors()) {
		Klass ancestorCls(*ancestor.get());
		for (const std::shared_ptr<ontology::DataProperty>& prop : ancestorCls.decorated().dataProperties()) {
			propOffset = DataProperty(*prop.get()).generateSaverInstanceSave(ofs, *this, ancestorCls, propOffset);
		}
		for (const std::shared_ptr<ontology::ObjectProperty>& prop : ancestorCls.decorated().objectProperties()) {
			propOffset = ObjectProperty(*prop.get()).generateSaverInstanceSave(ofs, *this, ancestorCls, propOffset);
		}
	}
	for (const std::shared_ptr<ontology::DataProperty>& prop : decorated().dataProperties()) {
		propOffset = DataProperty(*prop.get()).generateSaverInstanceSave(ofs, *this, *this, propOffset);
	}
	for (const std::shared_ptr<ontology::ObjectProperty>& prop : decorated().objectProperties()) {
		propOffset = ObjectProperty(*prop.get()).generateSaverInstanceSave(ofs, *this, *this, propOffset);
	}
	indent(ofs, 2) << "identity += " << storageSize() << ";" << std::endl;
	indent(ofs, 1) << "}" << std::endl;
	indent(ofs, 1) << "} // end " << _decorated.prettyIRIName() << std::endl;
}

void Klass::generateSaverGenLoaderData(std::ostream& ofs, uint64_t iKlass) const {
	for (auto const& ancestor: decorated().getAllAncestors()) {
		Klass ancestorCls(*ancestor.get());
		for (const std::shared_ptr<ontology::DataProperty>& prop : ancestorCls.decorated().dataProperties()) {
			DataProperty(*prop.get()).generateSaverGenLoaderData(ofs, ancestorCls);
		}
		for (const std::shared_ptr<ontology::ObjectProperty>& prop : ancestorCls.decorated().objectProperties()) {
			ObjectProperty(*prop.get()).generateSaverGenLoaderData(ofs, ancestorCls);
		}
	}
	for (const std::shared_ptr<ontology::DataProperty>& prop : decorated().dataProperties()) {
		DataProperty(*prop.get()).generateSaverGenLoaderData(ofs, *this);
	}
	for (const std::shared_ptr<ontology::ObjectProperty>& prop : decorated().objectProperties()) {
		ObjectProperty(*prop.get()).generateSaverGenLoaderData(ofs, *this);
	}

	indent(ofs, 1) << "ofs << \"static uint64_t " << _decorated.prettyIRIName() << "_INSTANCES[] = {\" << std::endl;" << std::endl;
	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
	indent(ofs, 1) << "for (auto i: " << _decorated.prettyIRIName() << "_INSTANCES) {" << std::endl;
		indent(ofs, 2) << "ofs << \", \" << i;" << std::endl;
	indent(ofs, 1) << "}" << std::endl;
	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;
}

void Klass::generateSaverGenLoaderLoad(std::ostream& ofs, uint64_t iKlass) const {
	for (auto const& ancestor: decorated().getAllAncestors()) {
		Klass ancestorCls(*ancestor.get());
		for (const std::shared_ptr<ontology::DataProperty>& prop : ancestorCls.decorated().dataProperties()) {
			DataProperty(*prop.get()).generateSaverGenLoaderLoad(ofs, ancestorCls);
		}
		for (const std::shared_ptr<ontology::ObjectProperty>& prop : ancestorCls.decorated().objectProperties()) {
			ObjectProperty(*prop.get()).generateSaverGenLoaderLoad(ofs, ancestorCls);
		}
	}
	for (const std::shared_ptr<ontology::DataProperty>& prop : decorated().dataProperties()) {
		DataProperty(*prop.get()).generateSaverGenLoaderLoad(ofs, *this);
	}
	for (const std::shared_ptr<ontology::ObjectProperty>& prop : decorated().objectProperties()) {
		ObjectProperty(*prop.get()).generateSaverGenLoaderLoad(ofs, *this);
	}
	indent(ofs, 1) << "ofs << \"" << genCppNameWithNamespace(false) << "::initInstances("
		<< _decorated.prettyIRIName() << "_INSTANCES, \" << " << _decorated.prettyIRIName() << "_INSTANCES.size()"
		<< " << \");\" << std::endl;" << std::endl;
}


}
}


