#include <autordf/ontology/Ontology.h>
#include "ObjectProperty.h"

#include "Klass.h"
#include "Utils.h"

namespace autordf {
namespace codegen {

void ObjectProperty::generateDeclaration(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);

    ofs << std::endl;
    indent(ofs, 1) << "/**" << std::endl;
    indent(ofs, 1) << " * Full iri for " <<  _decorated.rdfname().prettyName() << " object property." << std::endl;
    indent(ofs, 1) << " */" << std::endl;
    indent(ofs, 1) << "static const autordf::Uri " << _decorated.prettyIRIName() << "ObjectPropertyIri;" << std::endl;
    ofs << std::endl;

    if ( _decorated.maxCardinality(onClass.decorated()) <= 1 ) {
        if(genStaticModel()) {
    	generateStorage(ofs, onClass, false);
        }
        if ( _decorated.minCardinality(onClass.decorated()) > 0 ) {
            std::string methodName = _decorated.prettyIRIName();
            generatePropertyComment(ofs, onClass, methodName,  1,
                "@return the mandatory instance.\n"
                "@throw PropertyNotFound if object reference is not set", &propertyClass);
            indent(ofs, 1) << propertyClass.genCppNameWithNamespace() << " " << methodName << "() const;" << std::endl;
        } else {
            std::string methodName = _decorated.prettyIRIName() + "Optional";
            generatePropertyComment(ofs, onClass, methodName,  1,
                            "@return the object instance if it is set, or nullptr if it is not set.", &propertyClass);
            indent(ofs, 1) << "std::shared_ptr<" << propertyClass.genCppNameWithNamespace()  << "> " << methodName << "() const;" << std::endl;
        }
    }
    if ( _decorated.maxCardinality(onClass.decorated()) > 1 ) {
        if(genStaticModel()) {
    	generateStorage(ofs, onClass, true);
        }

        std::string methodName = _decorated.prettyIRIName() + "List";
        generatePropertyComment(ofs, onClass, methodName,  1,
                        "@return the list typed objects.  List can be empty if not values are set in database", &propertyClass);
        indent(ofs, 1) << "std::vector<" << propertyClass.genCppNameWithNamespace()  << "> " << methodName << "() const;" << std::endl;
        ofs << std::endl;
        if (!genStaticModel()) {
        generateDeclarationSetterForMany(ofs, onClass);
        }
    }
    ofs << std::endl;
    if (!genStaticModel()) {
    generateDeclarationSetterForOne(ofs, onClass);
    if ( _decorated.minCardinality(onClass.decorated()) != _decorated.maxCardinality(onClass.decorated()) ) {
        ofs << std::endl;
        generateRemoverDeclaration(ofs, onClass);
    }
    }
    ofs << std::endl;
}

void ObjectProperty::generateStorage(std::ostream& ofs, const Klass& onClass, bool forMany) const {
	indent(ofs, 1) << "static void initS_" << _decorated.prettyIRIName() << "(uint64_t* klassIdStorage, uint64_t* identityStorage";
	if (forMany) {
		ofs << ", uint64_t* arrStorage";
	}
	ofs << ", size_t len);\n" << std::endl;

	ofs << "protected:" << std::endl;
	indent(ofs, 1) << "static uint64_t* SK_" << _decorated.prettyIRIName() << ";" << std::endl;
	indent(ofs, 1) << "static uint64_t* SI_" << _decorated.prettyIRIName() << ";" << std::endl;
	indent(ofs, 1) << "static size_t SL_" << _decorated.prettyIRIName() << ";" << std::endl;
	indent(ofs, 1) << "uint64_t i_" << _decorated.prettyIRIName() << ";" << std::endl;
	if (forMany) {
    	indent(ofs, 1) << "static uint64_t* SARR_" << _decorated.prettyIRIName() << ";" << std::endl;
		indent(ofs, 1) << "uint64_t c_" << _decorated.prettyIRIName() << ";" << std::endl;
	}
	ofs << std::endl;
	ofs << "public:" << std::endl;
}

void ObjectProperty::generateKeyDeclaration(std::ostream& ofs, const Klass& onClass, size_t propIndex, size_t instanceSize) const {
    auto propertyClass = effectiveClass(onClass);
    std::string currentClassName = onClass.decorated().prettyIRIName();

    indent(ofs, 1) << "/**" << std::endl;
    indent(ofs, 1) << " * @brief This method " << genCppNameSpaceFullyQualified() << "::" << currentClassName << "::findBy" << _decorated.prettyIRIName(true) << " returns the only instance of " + onClass.decorated().rdfname() + " with property " + _decorated.rdfname() +  " set to given object." << std::endl;
    indent(ofs, 1) << " * " << std::endl;
    indent(ofs, 1) << " * @param key value that uniquely identifies the expected object" << std::endl;
    indent(ofs, 1) << " * " << std::endl;
    indent(ofs, 1) << " * @throw DuplicateObject if more than one object have the same property value" << std::endl;
    indent(ofs, 1) << " * @throw ObjectNotFound if no object has given property with value" << std::endl;
    indent(ofs, 1) << " */" << std::endl;

    indent(ofs, 1) << "static " << currentClassName << " findBy" << _decorated.prettyIRIName(true) << "( const " << propertyClass.genCppNameWithNamespace() << "& key ) {" << std::endl;
    if (!genStaticModel()) {
    indent(ofs, 2) <<     "return Object::findByKey(\"" << _decorated.rdfname() << "\", reinterpret_cast<const ::autordf::Object&>(key)).as<" << currentClassName << ">();" << std::endl;
    } else {
	indent(ofs, 2) << "uint64_t klassId = key.klass();" << std::endl;
	indent(ofs, 2) << "uint64_t valueIndex = key.identity();" << std::endl;
	indent(ofs, 2) << "if (valueIndex > 0) {" << std::endl;
	indent(ofs, 3) << "for (size_t i=" << 1 + propIndex << " ; i < INSTANCES_LENGTH; i+=" << instanceSize <<  ") {" << std::endl;
	indent(ofs, 4) << "if (INSTANCES[i] == klassId && INSTANCES[i+1] == valueIndex) {" << std::endl;
	indent(ofs, 5) << "return " << currentClassName << "(klassId, i - " << propIndex << ");" << std::endl;
	indent(ofs, 4) << "}" << std::endl;
	indent(ofs, 3) << "}" << std::endl;
	indent(ofs, 3) << "std::stringstream ss;" << std::endl;
	indent(ofs, 3) << "ss << \"No value for " << _decorated.prettyIRIName() << "=\" << key << \" found\";" << std::endl;
	indent(ofs, 3) << "throw autordf::ObjectNotFound(ss.str());" << std::endl;
	indent(ofs, 2) << "} else {" << std::endl;
	indent(ofs, 3) << "std::stringstream ss;" << std::endl;
	indent(ofs, 3) << "ss << \"No object with key " << _decorated.prettyIRIName() << "=\" << key << \" found\";" << std::endl;
	indent(ofs, 3) << "throw autordf::ObjectNotFound(ss.str());" << std::endl;
	indent(ofs, 2) << "}" << std::endl;
    }
    indent(ofs, 1) << "}" << std::endl;
	ofs << std::endl;
}

void ObjectProperty::generateDefinition(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);
    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;

    ofs << "const autordf::Uri " << currentClassName << "::" << _decorated.prettyIRIName() << "ObjectPropertyIri = \"" << _decorated.rdfname() << "\";" << std::endl;
    ofs << std::endl;
    if (genStaticModel()) {
	if (forMany) {
    	ofs << "uint64_t* " << currentClassName << "::SARR_" << _decorated.prettyIRIName() << " = 0;" << std::endl;
	}

	ofs << "uint64_t* " << currentClassName << "::SK_" << _decorated.prettyIRIName() << " = nullptr;" << std::endl;
	ofs << "uint64_t* " << currentClassName << "::SI_" << _decorated.prettyIRIName() << " = nullptr;" << std::endl;
	ofs << "uint64_t " << currentClassName << "::SL_" << _decorated.prettyIRIName() << " = 0;" << std::endl;

	ofs << "void " << currentClassName << "::initS_" << _decorated.prettyIRIName() << "(uint64_t* klassIdStorage, uint64_t* identityStorage";
	if (forMany) {
		ofs << ", uint64_t* arrStorage";
	}
	ofs << ", size_t len) {" << std::endl;

	indent(ofs, 1) << "SK_" << _decorated.prettyIRIName() << " = klassIdStorage;" << std::endl;
	indent(ofs, 1) << "SI_" << _decorated.prettyIRIName() << " = identityStorage;" << std::endl;
	if (forMany) {
		indent(ofs, 1) << "SARR_" << _decorated.prettyIRIName() << " = arrStorage;" << std::endl;
	}
	indent(ofs, 1) << "SL_" << _decorated.prettyIRIName() << " = len;" << std::endl;
	ofs << "}" << std::endl;
	ofs << std::endl;
    }

    if ( _decorated.maxCardinality(onClass.decorated()) <= 1 ) {
        if ( _decorated.minCardinality(onClass.decorated()) > 0 ) {
            ofs << propertyClass.genCppNameWithNamespace() << " " << currentClassName << "::" << _decorated.prettyIRIName() << "() const {" << std::endl;
            if (!genStaticModel()) {
            indent(ofs, 1) << "return object().getObject(\"" << _decorated.rdfname() << "\").as<" << propertyClass.genCppNameWithNamespace() << ">();" << std::endl;
            } else {
            indent(ofs, 1) << "if (i_" << _decorated.prettyIRIName() << " > 0) {" << std::endl;
            indent(ofs, 2) << "return " << propertyClass.genCppNameWithNamespace() << "("
            	<< "SK_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
            	<< ", SI_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]);" << std::endl;
            indent(ofs, 1) << "} else {" << std::endl;
            indent(ofs, 2) << "throw autordf::PropertyNotFound(\"Property " << _decorated.prettyIRIName() << " not found in TODO\");" << std::endl;
            indent(ofs, 1) << "}" << std::endl;
            }
            ofs << "}" << std::endl;
        } else {
            ofs << "std::shared_ptr<" << propertyClass.genCppNameWithNamespace() << "> " << currentClassName << "::" <<
                    _decorated.prettyIRIName() << "Optional() const {" << std::endl;
            if (!genStaticModel()) {
            indent(ofs, 1) << "auto result = object().getOptionalObject(\"" << _decorated.rdfname() << "\");" << std::endl;
            indent(ofs, 1) << "return result ? std::make_shared<" << propertyClass.genCppNameWithNamespace() << ">(*result) : nullptr;" << std::endl;
            } else {
            indent(ofs, 1) << "if (i_" << _decorated.prettyIRIName() << " > 0) {" << std::endl;
            indent(ofs, 2) << "return std::make_shared<" << propertyClass.genCppNameWithNamespace() << ">("
            	<< "SK_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
            	<< ", SI_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]);" << std::endl;
            indent(ofs, 1) << "} else {" << std::endl;
            indent(ofs, 2) << "return nullptr;" << std::endl;
            indent(ofs, 1) << "}" << std::endl;
            }
            ofs << "}" << std::endl;
        }
    }
    if ( _decorated.maxCardinality(onClass.decorated()) > 1 ) {
        ofs << "std::vector<" << propertyClass.genCppNameWithNamespace() << "> " << currentClassName << "::" <<
                _decorated.prettyIRIName() << "List() const {" << std::endl;
        if (!genStaticModel()) {
        indent(ofs, 1) << "return object().getObjectListImpl<" << propertyClass.genCppNameWithNamespace() << ">(\"" <<  _decorated.rdfname() << "\", " << orderedBoolValue() << ");" << std::endl;
        } else {
		indent(ofs, 1) << "std::vector<" << propertyClass.genCppNameWithNamespace() << "> ret;" << std::endl;
		indent(ofs, 1) << "ret.reserve(" << "c_" << _decorated.prettyIRIName() << ");" << std::endl;
		indent(ofs, 1) << "for (uint64_t i=0; i < c_" << _decorated.prettyIRIName() << "; i++) {" << std::endl;
		indent(ofs, 2) << "uint64_t j = SARR_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << " + i];" << std::endl;
		indent(ofs, 2) << "ret.emplace_back("
			<< "SK_" << _decorated.prettyIRIName() << "[j]"
			<< ", SI_" << _decorated.prettyIRIName()  << "[j]"
			<< ");" << std::endl;
		indent(ofs, 1) << "}" << std::endl;
		indent(ofs, 1) << "return ret;" << std::endl;
        }
        ofs << "}" << std::endl;
        ofs << std::endl;
        if (!genStaticModel()) {
        generateDefinitionSetterForMany(ofs, onClass);
        }
    }
    ofs << std::endl;
    if (!genStaticModel()) {
    generateDefinitionSetterForOne(ofs, onClass);
    if ( _decorated.minCardinality(onClass.decorated()) != _decorated.maxCardinality(onClass.decorated()) ) {
        ofs << std::endl;
        generateRemoverDefinition(ofs, onClass);
    }
    ofs << std::endl;
    }
}

void ObjectProperty::generateDeclarationSetterForOne(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);

    std::string methodName = "set" + _decorated.prettyIRIName(true);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

    generatePropertyComment(ofs, onClass, methodName,  1,
                    "Sets the mandatory value for this property.\n"
                            "@param value value to set for this property, removing all other values", &propertyClass);
    indent(ofs, 1) << currentClassName << "& " << methodName << "( const " << propertyClass.genCppNameWithNamespace(true) << "& value);" << std::endl;
}

void ObjectProperty::generateDeclarationSetterForMany(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

    std::string methodName = "set" + _decorated.prettyIRIName(true) + "List";
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "Sets the values for this property.\n"
                            "@param values values to set for this property, removing all other values", &propertyClass);
    indent(ofs, 1) << currentClassName << "& " << methodName << "( const std::vector<" << propertyClass.genCppNameWithNamespace(false) << ">& values);" << std::endl;
    ofs << std::endl;

    methodName = "add" + _decorated.prettyIRIName(true);
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "Adds a value for this property.\n"
                            "@param value value to set for this property, removing all other values", &propertyClass);
    indent(ofs, 1) << currentClassName << "& " << methodName << "( const " << propertyClass.genCppNameWithNamespace(true) << "& value);" << std::endl;
}

void ObjectProperty::generateDefinitionSetterForOne(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);
    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();
    ofs << currentClassName << "& " << currentClassName << "::set" << _decorated.prettyIRIName(true) << "( const " << propertyClass.genCppNameWithNamespace(true) << "& value) {" << std::endl;
    indent(ofs, 1) <<     "object().setObject(\"" << _decorated.rdfname() << "\", value.object());" << std::endl;
    indent(ofs, 1) <<     "return *this;" << std::endl;
    ofs << "}" << std::endl;
}

void ObjectProperty::generateDefinitionSetterForMany(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);
    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();
    ofs << currentClassName << "& " << currentClassName << "::set" << _decorated.prettyIRIName(true) << "List( const std::vector<" << propertyClass.genCppNameWithNamespace(false) << ">& values) {" << std::endl;
    indent(ofs, 1) <<     "object().setObjectListImpl<" << propertyClass.genCppNameWithNamespace(false) << ">(\"" <<  _decorated.rdfname() << "\", values, " << orderedBoolValue() << ");" << std::endl;
    indent(ofs, 1) <<     "return *this;" << std::endl;
    ofs << "}" << std::endl;
    ofs << std::endl;
    ofs << currentClassName << "& " << currentClassName << "::add" << _decorated.prettyIRIName(true) << "( const " << propertyClass.genCppNameWithNamespace(true) << "& value) {" << std::endl;
    indent(ofs, 1) <<     "object().addObject(\"" << _decorated.rdfname() << "\", value.object(), " << orderedBoolValue() << ");" << std::endl;
    indent(ofs, 1) <<     "return *this;" << std::endl;
    ofs << "}" << std::endl;
}

Klass ObjectProperty::effectiveClass(const Klass& onClass) const {
    std::shared_ptr<ontology::Klass> kls = _decorated.findClass(&onClass.decorated());
    if ( kls ) {
        return *kls;
    } else {
        return *_decorated.ontology()->findClass(autordf::ontology::Ontology::OWL_NS + "Thing");
    }
}

void ObjectProperty::generateRemoverDeclaration(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();
    std::string methodName = "remove" + _decorated.prettyIRIName(true);

    generatePropertyComment(ofs, onClass, methodName,  1,
                    "Remove a value for this property.\n"
                            "@param obj value to remove for this property.\n"
                            "@throw PropertyNotFound if propertyIRI has not obj as value", &propertyClass);
    indent(ofs, 1) << currentClassName << "& " << methodName << "( const " << propertyClass.genCppNameWithNamespace(true) << "& obj);" << std::endl;
}

void ObjectProperty::generateRemoverDefinition(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

    ofs << currentClassName << "& " << currentClassName << "::remove" << _decorated.prettyIRIName(true) << "( const " << propertyClass.genCppNameWithNamespace(true) << "& value) {" << std::endl;
    indent(ofs, 1) << "object().removeObject(\"" << _decorated.rdfname() << "\", value.object());" << std::endl;
    indent(ofs, 1) << "return *this;" << std::endl;
    ofs  << "}" << std::endl;
}

int ObjectProperty::generateSetIndices(std::ostream& ofs, const Klass& onClass, int i) const {
    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

	indent(ofs, 2) << currentClassName << "::i_" << name() << " = INSTANCES[storageIndex + " << i << "];" << std::endl;
	i++;
	if (_decorated.maxCardinality(onClass.decorated()) > 1) {
		indent(ofs, 2) << currentClassName << "::c_" << name() << " = INSTANCES[storageIndex + " << i << "];" << std::endl;
		i++;
	}
	return i;
}

void ObjectProperty::generateSaverValuesDecl(std::ostream& ofs, const Klass& onClass) const {
    auto propertyClass = effectiveClass(onClass);
    std::string currentClassName = onClass.decorated().prettyIRIName();
	indent(ofs, 1) << "std::set<" << propertyClass.genCppNameWithNamespace(false) << "> " << currentClassName << "_" << name() << "_values;" << std::endl;
    if (_decorated.maxCardinality(onClass.decorated()) > 1) {
        indent(ofs, 1) << "std::vector<uint64_t> " << currentClassName << "_" << name() << "_ARR;" << std::endl;
    }
}

void ObjectProperty::generateSaverValuesSet(std::ostream& ofs, const Klass& onClass) const {
    std::string currentClassName = onClass.decorated().prettyIRIName();
    auto propertyClass = effectiveClass(onClass);

	if (_decorated.maxCardinality(onClass.decorated()) <= 1) {
		if (_decorated.minCardinality(onClass.decorated()) > 0) {
			indent(ofs, 2) << currentClassName << "_" << name() << "_values.insert("
				<< "obj." << _decorated.prettyIRIName() << "());" << std::endl;
		} else {
			indent(ofs, 2) << "{" << std::endl;
			indent(ofs, 3) << "std::shared_ptr<" << propertyClass.genCppNameWithNamespace(false) << "> tmp = obj." << _decorated.prettyIRIName() << "Optional();" << std::endl;
			indent(ofs, 3) << "if (tmp) {" << std::endl;
			indent(ofs, 4) << currentClassName << "_" << name() << "_values.insert(*tmp);" << std::endl;
			indent(ofs, 3) << "}" << std::endl;
			indent(ofs, 2) << "}" << std::endl;
		}
	} else {  // maxCardinality > 1
		indent(ofs, 2) << "for(auto const& p: obj." << _decorated.prettyIRIName() << "List()) {" << std::endl;
		indent(ofs, 3) << currentClassName << "_" << name() << "_values.insert(p);" << std::endl;
		indent(ofs, 2) << "}" << std::endl;
	}
}

int ObjectProperty::generateSaverInstanceSave(std::ostream& ofs, const Klass& onClass, const Klass& storageClass, int propOffset) const {
    std::string storageClassName = storageClass.decorated().prettyIRIName();
    std::string currentClassName = onClass.decorated().prettyIRIName();
    auto propertyClass = effectiveClass(onClass);

	indent(ofs, 2) << "{ // " << currentClassName << " " << storageClassName << "::" << _decorated.prettyIRIName() << std::endl;
	if (_decorated.maxCardinality(onClass.decorated()) <= 1) {
		if (_decorated.minCardinality(onClass.decorated()) > 0) {
			indent(ofs, 3) << "uint64_t i=1;" << std::endl;
			indent(ofs, 3) << "auto const& objV = obj." << _decorated.prettyIRIName() << "());" << std::endl;
			indent(ofs, 3) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
			indent(ofs, 4) << "if (objV == v) {" << std::endl;
			indent(ofs, 5) << currentClassName << "_INSTANCES[identity + " << propOffset << "] = i;" << std::endl;
			indent(ofs, 5) << "break;" << std::endl;
			indent(ofs, 4) << "}" << std::endl;
			indent(ofs, 4) << "i++;" << std::endl;
			indent(ofs, 3) << "}" << std::endl;
		} else {
			indent(ofs, 3) << "std::shared_ptr<" << propertyClass.genCppNameWithNamespace(false) << "> tmp = obj." << _decorated.prettyIRIName() << "Optional();" << std::endl;
			indent(ofs, 3) << currentClassName << "_INSTANCES[identity + " << propOffset << "] = 0;" << std::endl;
			indent(ofs, 3) << "if (tmp) {" << std::endl;
			indent(ofs, 4) << "uint64_t i=1;" << std::endl;
			indent(ofs, 4) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
			indent(ofs, 5) << "if (*tmp == v) {" << std::endl;
			indent(ofs, 6) << currentClassName << "_INSTANCES[identity + " << propOffset << "] = i;" << std::endl;
			indent(ofs, 6) << "break;" << std::endl;
			indent(ofs, 5) << "}" << std::endl;
			indent(ofs, 5) << "i++;" << std::endl;
			indent(ofs, 4) << "}" << std::endl;
			indent(ofs, 3) << "}" << std::endl;
		}
	} else {  // maxCardinality > 1
		indent(ofs, 3) << currentClassName << "_INSTANCES[identity + " << propOffset << "] = " << storageClassName << "_" << _decorated.prettyIRIName() << "_ARR.size();" << std::endl;
		indent(ofs, 3) << "uint64_t j=0;" << std::endl;
		indent(ofs, 3) << "for(auto const& p: obj." << _decorated.prettyIRIName() << "List()) {" << std::endl;
			indent(ofs, 4) << "uint64_t i=1;" << std::endl;
			indent(ofs, 4) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
				indent(ofs, 5) << "if (p == v) {" << std::endl;
					indent(ofs, 6) << storageClassName << "_" << _decorated.prettyIRIName() << "_ARR.push_back(i);" << std::endl;
					indent(ofs, 6) << "break;" << std::endl;
				indent(ofs, 5) << "}" << std::endl;
				indent(ofs, 5) << "i++;" << std::endl;
			indent(ofs, 4) << "}" << std::endl;
			indent(ofs, 4) << "j++;" << std::endl;
		indent(ofs, 3) << "}" << std::endl;
		indent(ofs, 3) << currentClassName << "_INSTANCES[identity + " << (propOffset + 1)  << "] = j;" << std::endl;
	}
	indent(ofs, 2) << "}" << std::endl;
	return propOffset + storageSize(onClass);
}

void ObjectProperty::generateSaverGenLoaderData(std::ostream& ofs, const Klass& onClass) const {
    std::string storageClassName = onClass.decorated().prettyIRIName();
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;

	indent(ofs, 1) << "ofs << \"static const size_t " << storageClassName << "_SL_" << _decorated.prettyIRIName() << " = \" << " << storageClassName << "_" << name() << "_values.size()" << " << \";\\n\";" << std::endl;
	indent(ofs, 1) << "ofs << \"static uint64_t " << storageClassName << "_SK_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
	indent(ofs, 2) << "ofs << \", \" << allObjectKlassIds[v] << std::endl;" << std::endl;
	indent(ofs, 1) << "}" << std::endl;
	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;

	indent(ofs, 1) << "ofs << \"static uint64_t " << storageClassName << "_SI_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
	indent(ofs, 2) << "ofs << \", \" << allObjectIdentities[v] << std::endl;" << std::endl;
	indent(ofs, 1) << "}" << std::endl;
	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;
	if (forMany) {
    	indent(ofs, 1) << "ofs << \"static uint64_t " << storageClassName << "_SARR_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_ARR) {" << std::endl;
    	indent(ofs, 2) << "ofs << \", \" << v << std::endl;" << std::endl;
    	indent(ofs, 1) << "}" << std::endl;
    	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;
	}
}

void ObjectProperty::generateSaverGenLoaderLoad(std::ostream& ofs, const Klass& onClass) const {
    std::string storageClassName = onClass.decorated().prettyIRIName();
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;
	indent(ofs, 1) << "ofs << \"" << onClass.genCppNameWithNamespace(false) << "::initS_" << _decorated.prettyIRIName() << "("
			<< storageClassName << "_SK_" << _decorated.prettyIRIName()
			<< ", " << storageClassName << "_SI_" << _decorated.prettyIRIName();
	if (forMany) {
		 ofs << ", " << storageClassName << "_SARR_" << _decorated.prettyIRIName();
	}
	ofs << ", " << storageClassName << "_SL_" << _decorated.prettyIRIName() << ");\\n\";" << std::endl;
}

std::string ObjectProperty::orderedBoolValue() const {
    return _decorated.ordered() ? "true" : "false";
}

}
}
