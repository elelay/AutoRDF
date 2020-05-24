#include "DataProperty.h"

#include <autordf/cvt/Cvt.h>

#include "Klass.h"
#include "Utils.h"

namespace autordf {
namespace codegen {

std::map<cvt::RdfTypeEnum, std::string> rdf2CppTypeMapping(
        {
//FIXME: handle PlainLiteral, XMLLiteral, Literal, real, rational
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_string,             "std::string"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_boolean,            "bool"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_decimal,            "double"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_float,              "float"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_double,             "double"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_dateTime,           "boost::posix_time::ptime"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_integer,            "long long int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_dateTimeStamp,      "boost::posix_time::ptime"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_nonNegativeInteger, "long long unsigned int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_positiveInteger,    "long long unsigned int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_nonPositiveInteger, "long long int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_negativeInteger,    "long long int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_long,               "long long int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_unsignedLong,       "unsigned long long int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_int,                "long int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_unsignedInt,        "long unsigned int"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_short,              "short"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_unsignedShort,      "unsigned short"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_byte,               "char"),
        std::pair<cvt::RdfTypeEnum, std::string>(cvt::RdfTypeEnum::xsd_unsignedByte,       "unsigned char")
    }
);

void DataProperty::generateDeclaration(std::ostream& ofs, const Klass& onClass) const {
    indent(ofs, 1) << "/**" << std::endl;
    indent(ofs, 1) << " * Full iri for " <<  _decorated.rdfname().prettyName() << " data property." << std::endl;
    indent(ofs, 1) << " */" << std::endl;
    indent(ofs, 1) << "static const autordf::Uri " << _decorated.prettyIRIName() << "DataPropertyIri;" << std::endl;
    ofs << std::endl;

    if ( _decorated.maxCardinality(onClass.decorated()) <= 1 ) {
        if ( _decorated.minCardinality(onClass.decorated()) > 0 ) {
        	generateStorage(ofs, onClass, false, false);
            generateGetterForOneMandatory(ofs, onClass);
        } else {
        	generateStorage(ofs, onClass, true, false);
            generateGetterForOneOptional(ofs, onClass);
        }
        ofs << std::endl;
        // generateSetterForOne(ofs, onClass);
        // ofs << std::endl;
    }
    if ( _decorated.maxCardinality(onClass.decorated()) > 1 ) {
		generateStorage(ofs, onClass, false, true);
        generateGetterForMany(ofs, onClass);
        ofs << std::endl;
        // generateSetterForOne(ofs, onClass);
        // ofs << std::endl;
        // generateSetterForMany(ofs, onClass);
    }
    if ( _decorated.minCardinality(onClass.decorated()) != _decorated.maxCardinality(onClass.decorated()) ) {
        ofs << std::endl;
        // generateRemover(ofs, onClass);
    }
}

uint64_t DataProperty::storageSize(const Klass& onClass) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;
    return forMany ? 2 : (rdfCppType.second.empty() ? 3 : 1);
}

void DataProperty::generateStorage(std::ostream& ofs, const Klass& onClass, bool optional, bool forMany) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    if (!rdfCppType.second.empty()) {
    	indent(ofs, 1) << "static void initS_" << _decorated.prettyIRIName() << "(" << rdfCppType.second << "* storage";
	} else {
    	indent(ofs, 1) << "static void initS_" << _decorated.prettyIRIName() << "(std::string* valueStorage, std::string* langStorage, std::string* typeStorage" << std::endl;
	}
	if (forMany) {
		ofs << ", uint64_t* arrStorage";
	}
	ofs << ", size_t len);\n" << std::endl;

	ofs << "protected:" << std::endl;
    if (!rdfCppType.second.empty()) {
    	indent(ofs, 1) << "static " << rdfCppType.second << "* S_" << _decorated.prettyIRIName() << ";" << std::endl;
	} else {
    	indent(ofs, 1) << "static std::string* SV_" << _decorated.prettyIRIName() << ";" << std::endl;
    	indent(ofs, 1) << "static std::string* SLANG_" << _decorated.prettyIRIName() << ";" << std::endl;
    	indent(ofs, 1) << "static std::string* STYPE_" << _decorated.prettyIRIName() << ";" << std::endl;
	}
	indent(ofs, 1) << "static size_t SL_" << _decorated.prettyIRIName() << ";" << std::endl;
	indent(ofs, 1) << "uint64_t i_" << _decorated.prettyIRIName() << ";" << std::endl;
	if (forMany) {
    	indent(ofs, 1) << "static uint64_t* SARR_" << _decorated.prettyIRIName() << ";" << std::endl;
		indent(ofs, 1) << "uint64_t c_" << _decorated.prettyIRIName() << ";" << std::endl;
	}
	ofs << std::endl;
	ofs << "public:" << std::endl;
}

void DataProperty::generateKeyDeclaration(std::ostream& ofs, const Klass& onClass, size_t propIndex, size_t propCount) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    std::string currentClassName = onClass.decorated().prettyIRIName();

    // indent(ofs, 1) << "/**" << std::endl;
    // indent(ofs, 1) << " * @brief This method " << genCppNameSpaceFullyQualified() << "::" << currentClassName << "::findBy" << _decorated.prettyIRIName(true) << " returns the only instance of " + onClass.decorated().rdfname() + " with property " + _decorated.rdfname() +  " set to given value." << std::endl;
    // indent(ofs, 1) << " * " << std::endl;
    // indent(ofs, 1) << " * @param key value that uniquely identifies the expected object" << std::endl;
    // indent(ofs, 1) << " * " << std::endl;
    // indent(ofs, 1) << " * @throw DuplicateObject if more than one object have the same property value" << std::endl;
    // indent(ofs, 1) << " * @throw ObjectNotFound if no object has given property with value" << std::endl;
    // indent(ofs, 1) << " */" << std::endl;

    // indent(ofs, 1) << "static " << currentClassName << " findBy" << _decorated.prettyIRIName(true) << "( const autordf::PropertyValue& key ) {" << std::endl;
    //indent(ofs, 2) <<     "return Object::findByKey(\"" << _decorated.rdfname() << "\", key).as<" << currentClassName << ">();" << std::endl;
    // indent(ofs, 1) << "}" << std::endl;
    // ofs << std::endl;

    if (!rdfCppType.second.empty()) {
        indent(ofs, 1) << "/**" << std::endl;
        indent(ofs, 1) << " * @brief This method " << genCppNameSpaceFullyQualified() << "::" << currentClassName << "::findBy" << _decorated.prettyIRIName(true) << " returns the only instance of " + onClass.decorated().rdfname() + " with property " + _decorated.rdfname() +  " set to given value." << std::endl;
        indent(ofs, 1) << " * " << std::endl;
        indent(ofs, 1) << " * @param key value that uniquely identifies the expected object" << std::endl;
        indent(ofs, 1) << " * " << std::endl;
        indent(ofs, 1) << " * @throw DuplicateObject if more than one object have the same property value" << std::endl;
        indent(ofs, 1) << " * @throw ObjectNotFound if no object has given property with value" << std::endl;
        indent(ofs, 1) << " */" << std::endl;

        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << "static " << currentClassName << " findBy" << _decorated.prettyIRIName(true) << "( const " << cppType << "& key ) {" << std::endl;
        // indent(ofs, 2) <<     "return findBy" << _decorated.prettyIRIName(true) << "(autordf::PropertyValue().set<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfCppType.first) << ">(key));" << std::endl;
        indent(ofs, 2) << "uint64_t valueIndex = 0;" << std::endl;
		indent(ofs, 2) << "for (size_t i=1; i < SL_" << _decorated.prettyIRIName() << "; i++) {" << std::endl;
		indent(ofs, 3) << "if (S_" << _decorated.prettyIRIName() << "[i] == key) {" << std::endl;
		indent(ofs, 4) << "valueIndex = i;" << std::endl;
		indent(ofs, 4) << "break;" << std::endl;
		indent(ofs, 3) << "}" << std::endl;
		indent(ofs, 2) << "}" << std::endl;
		indent(ofs, 2) << "if (valueIndex > 0) {" << std::endl;
		indent(ofs, 3) << "for (size_t i=" << 1 + propIndex << " ; i < INSTANCES_LENGTH; i+=" << propCount <<  ") {" << std::endl;
		indent(ofs, 4) << "if (INSTANCES[i] == valueIndex) {" << std::endl;
		indent(ofs, 5) << "return " << currentClassName << "("
			<< currentClassName << "::KLASS_ID"
			<< ", i - " << propIndex << ");" << std::endl;
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
        indent(ofs, 1) << "}" << std::endl;
        ofs << std::endl;
    }
}

void DataProperty::generateDefinition(std::ostream& ofs, const Klass& onClass) const {
    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;


    ofs << "const autordf::Uri " << currentClassName << "::" << _decorated.prettyIRIName() << "DataPropertyIri = \"" << _decorated.rdfname() << "\";" << std::endl;
    ofs << std::endl;

	if (forMany) {
    	ofs << "uint64_t* " << currentClassName << "::SARR_" << _decorated.prettyIRIName() << " = 0;" << std::endl;
	}

    if (rdfCppType.second.empty()) {
    	ofs << "std::string* " << currentClassName << "::SV_" << _decorated.prettyIRIName() << " = nullptr;" << std::endl;
    	ofs << "std::string* " << currentClassName << "::SLANG_" << _decorated.prettyIRIName() << " = nullptr;" << std::endl;
    	ofs << "std::string* " << currentClassName << "::STYPE_" << _decorated.prettyIRIName() << " = nullptr;" << std::endl;
    	ofs << "uint64_t " << currentClassName << "::SL_" << _decorated.prettyIRIName() << " = 0;" << std::endl;

    	ofs << "void " << currentClassName << "::initS_" << _decorated.prettyIRIName() << "(std::string* valueStorage, std::string* langStorage, std::string* typeStorage";
    	if (forMany) {
    		ofs << ", uint64_t* arrStorage";
    	}
    	ofs << ", size_t len) {" << std::endl;

    	indent(ofs, 1) << "SV_" << _decorated.prettyIRIName() << " = valueStorage;" << std::endl;
    	indent(ofs, 1) << "SLANG_" << _decorated.prettyIRIName() << " = langStorage;" << std::endl;
    	indent(ofs, 1) << "STYPE_" << _decorated.prettyIRIName() << " = typeStorage;" << std::endl;
    	if (forMany) {
    		indent(ofs, 1) << "SARR_" << _decorated.prettyIRIName() << " = arrStorage;" << std::endl;
    	}
    	indent(ofs, 1) << "SL_" << _decorated.prettyIRIName() << " = len;" << std::endl;
    	ofs << "}" << std::endl;
    	ofs << std::endl;

        if (_decorated.maxCardinality(onClass.decorated()) <= 1) {
            if (_decorated.minCardinality(onClass.decorated()) > 0) {
                // Nothing
            } else {
                ofs << "std::shared_ptr<autordf::PropertyValue> " << currentClassName << "::" << _decorated.prettyIRIName() << "Optional() const {" << std::endl;
                indent(ofs, 1) << "if (i_" << _decorated.prettyIRIName() << " > 0) {" << std::endl;
                indent(ofs, 2) << "return std::make_shared<autordf::PropertyValue>("
                	<< "SV_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
                	<< ", SLANG_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
                	<< ", STYPE_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
                	<< ");" << std::endl;
                indent(ofs, 1) << "} else {" << std::endl;
                indent(ofs, 2) << "return nullptr;" << std::endl;
                indent(ofs, 1) << "}" << std::endl;
                ofs << "}" << std::endl;
                ofs << std::endl;
                ofs << "autordf::PropertyValue " << currentClassName << "::" << _decorated.prettyIRIName() << "(const autordf::PropertyValue& defaultval) const {" << std::endl;
                // indent(ofs, 1) << "auto ptrval = object().getOptionalPropertyValue(\"" << _decorated.rdfname() << "\");" << std::endl;
                // indent(ofs, 1) << "return (ptrval ? ptrval->get<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) << ", " << cppType << ">() : defaultval);" << std::endl;
                indent(ofs, 1) << "if (i_" << _decorated.prettyIRIName() << " > 0) {" << std::endl;
                indent(ofs, 2) << "return autordf::PropertyValue("
                	<< "SV_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
                	<< ", SLANG_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
                	<< ", STYPE_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "]"
                	<< ");" << std::endl;
                indent(ofs, 1) << "} else {" << std::endl;
                indent(ofs, 2) << "return defaultval;" << std::endl;
                indent(ofs, 1) << "}" << std::endl;
                ofs << "}" << std::endl;
            }
        }
        if (forMany) {
            ofs << "autordf::PropertyValueVector " << currentClassName << "::" << _decorated.prettyIRIName() <<
            "List() const {" << std::endl;
            // indent(ofs, 1) << "return object().getValueListImpl<autordf::cvt::RdfTypeEnum::" <<
            // cvt::rdfTypeEnumString(rdfType) << ", " << cppType << ">(\"" << _decorated.rdfname() << "\", " << orderedBoolValue() << ");" << std::endl;
            indent(ofs, 1) << "autordf::PropertyValueVector ret;" << std::endl;
            indent(ofs, 1) << "ret.reserve(" << "c_" << _decorated.prettyIRIName() << ");" << std::endl;
			indent(ofs, 1) << "for (uint64_t i=0; i < c_" << _decorated.prettyIRIName() << "; i++) {" << std::endl;
			indent(ofs, 2) << "uint64_t j = SARR_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << " + i];" << std::endl;
			indent(ofs, 2) << "ret.emplace_back("
            	<< "SV_" << _decorated.prettyIRIName() << "[j]"
            	<< ", SLANG_" << _decorated.prettyIRIName() << "[j]"
            	<< ", STYPE_" << _decorated.prettyIRIName() << "[j]"
            	<< ");" << std::endl;
			indent(ofs, 1) << "}" << std::endl;
			indent(ofs, 1) << "return ret;" << std::endl;
            ofs << "}" << std::endl;
            ofs << std::endl;
        }
    } else {
        cvt::RdfTypeEnum rdfType = rdfCppType.first;
        std::string cppType = rdfCppType.second;

    	ofs << rdfCppType.second << "* " << currentClassName << "::S_" << _decorated.prettyIRIName() << " = nullptr;" << std::endl;
    	ofs << "uint64_t " << currentClassName << "::SL_" << _decorated.prettyIRIName() << " = 0;" << std::endl;

    	ofs << "void " << currentClassName << "::initS_" << _decorated.prettyIRIName() << "(" << rdfCppType.second << "* storage";
    	if (forMany) {
    		ofs << ", uint64_t* arrStorage";
    	}
    	ofs << ", size_t len) {" << std::endl;


    	indent(ofs, 1) << "S_" << _decorated.prettyIRIName() << " = storage;" << std::endl;
    	if (forMany) {
    		indent(ofs, 1) << "SARR_" << _decorated.prettyIRIName() << " = arrStorage;" << std::endl;
    	}
    	indent(ofs, 1) << "SL_" << _decorated.prettyIRIName() << " = len;" << std::endl;
    	ofs << "}" << std::endl;
    	ofs << std::endl;

        if (_decorated.maxCardinality(onClass.decorated()) <= 1) {
            if (_decorated.minCardinality(onClass.decorated()) > 0) {
                // Nothing
            } else {
                ofs << "std::shared_ptr<" << cppType << "> " << currentClassName << "::" << _decorated.prettyIRIName() << "Optional() const {" << std::endl;
                // indent(ofs, 1) << "auto ptrval = object().getOptionalPropertyValue(\"" << _decorated.rdfname() << "\");" << std::endl;
                // indent(ofs, 1) << "return (ptrval ? std::shared_ptr<" << cppType << ">(new " << cppType <<
                // "(ptrval->get<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) << ", " << cppType << ">())) : nullptr);" << std::endl;
                indent(ofs, 1) << "if (i_" << _decorated.prettyIRIName() << " > 0) {" << std::endl;
                indent(ofs, 2) << "return std::make_shared<" << cppType << ">(S_" << _decorated.prettyIRIName() <<
                "[i_" << _decorated.prettyIRIName() << "]);" << std::endl;
                indent(ofs, 1) << "} else {" << std::endl;
                indent(ofs, 2) << "return nullptr;" << std::endl;
                indent(ofs, 1) << "}" << std::endl;
                ofs << "}" << std::endl;
                ofs << std::endl;
                ofs << cppType << " " << currentClassName << "::" << _decorated.prettyIRIName() << "(const " << cppType <<"& defaultval) const {" << std::endl;
                // indent(ofs, 1) << "auto ptrval = object().getOptionalPropertyValue(\"" << _decorated.rdfname() << "\");" << std::endl;
                // indent(ofs, 1) << "return (ptrval ? ptrval->get<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) << ", " << cppType << ">() : defaultval);" << std::endl;
                indent(ofs, 1) << "if (i_" << _decorated.prettyIRIName() << " > 0) {" << std::endl;
                indent(ofs, 2) << "return S_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << "];" << std::endl;
                indent(ofs, 1) << "} else {" << std::endl;
                indent(ofs, 2) << "return defaultval;" << std::endl;
                indent(ofs, 1) << "}" << std::endl;
                ofs << "}" << std::endl;
            }
        }
        if (_decorated.maxCardinality(onClass.decorated()) > 1) {
            ofs << "std::vector<" << cppType << "> " << currentClassName << "::" << _decorated.prettyIRIName() <<
            "List() const {" << std::endl;
            // indent(ofs, 1) << "return object().getValueListImpl<autordf::cvt::RdfTypeEnum::" <<
            // cvt::rdfTypeEnumString(rdfType) << ", " << cppType << ">(\"" << _decorated.rdfname() << "\", " << orderedBoolValue() << ");" << std::endl;

            indent(ofs, 1) << "std::vector<" << cppType << "> ret;" << std::endl;
            indent(ofs, 1) << "ret.reserve(" << "c_" << _decorated.prettyIRIName() << ");" << std::endl;
			indent(ofs, 1) << "for (uint64_t i=0; i < c_" << _decorated.prettyIRIName() << "; i++) {" << std::endl;
			indent(ofs, 2) << "uint64_t j = SARR_" << _decorated.prettyIRIName() << "[i_" << _decorated.prettyIRIName() << " + i];" << std::endl;
			indent(ofs, 2) << "ret.emplace_back(S_" << _decorated.prettyIRIName() << "[j]);" << std::endl;
			indent(ofs, 1) << "}" << std::endl;
			indent(ofs, 1) << "return ret;" << std::endl;
			ofs << "}" << std::endl;
            // ofs << currentClassName << "& " << currentClassName << "::set" << _decorated.prettyIRIName(true) << "(const std::vector<" << cppType << ">& values) {" << std::endl;
            // indent(ofs, 1) <<     "object().setValueListImpl<autordf::cvt::RdfTypeEnum::" <<
            // cvt::rdfTypeEnumString(rdfType) << ">(\"" << _decorated.rdfname() << "\", values, " << orderedBoolValue() << ");" << std::endl;
            // indent(ofs, 1) << "return *this;" << std::endl;
            // ofs << "}" << std::endl;
        }
    }
}

std::pair<cvt::RdfTypeEnum, std::string> DataProperty::getRdfCppTypes(const Klass& onClass) const {
    auto rdfTypeEntry = cvt::rdfMapType.find(_decorated.range(&onClass.decorated()));

    if (rdfTypeEntry != cvt::rdfMapType.end()) {
        auto cppTypeEntry = rdf2CppTypeMapping.find(rdfTypeEntry->second);
        if (cppTypeEntry != rdf2CppTypeMapping.end()) {
            return std::make_pair(rdfTypeEntry->second, cppTypeEntry->second);
        }
    }
    return std::make_pair(cvt::RdfTypeEnum::xsd_string , "");
}

void DataProperty::generateGetterForOneMandatory(std::ostream& ofs, const Klass& onClass) const {
    std::string methodName = _decorated.prettyIRIName();
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "@return the mandatory value for this property.\n"
                            "@throw PropertyNotFound if value is not set in database");
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    if (!rdfCppType.second.empty()) {
        cvt::RdfTypeEnum rdfType = rdfCppType.first;
        std::string cppType = rdfCppType.second;
        /*indent(ofs, 1) << cppType << " " << methodName << "() const {" << std::endl;
        indent(ofs, 2) << "return object().getPropertyValue(\"" << _decorated.rdfname() << "\").get<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) << ", " << cppType<< ">();" << std::endl;
        indent(ofs, 1) << "}" << std::endl;*/
        indent(ofs, 1) << cppType << " " << methodName << "() const {" << std::endl;
        indent(ofs, 2) << "return S_" << methodName << "[i_" << methodName << "];\n";
        indent(ofs, 1) << "}" << std::endl;
    } else {
        indent(ofs, 1) << "autordf::PropertyValue " << methodName << "() const {" << std::endl;
        indent(ofs, 2) << "return autordf::PropertyValue("
        	<< "SV_" << methodName << "[i_" << methodName << "]"
        	<< "SLANG_" << methodName << "[i_" << methodName << "]"
        	<< "STYPE_" << methodName << "[i_" << methodName << "]"
        	<< ");" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    }
}

void DataProperty::generateSetterForOne(std::ostream& ofs, const Klass& onClass) const {
    std::string methodName = "set" + _decorated.prettyIRIName(true);
    /*generatePropertyComment(ofs, onClass, methodName, 1,
                    "Sets the mandatory value for this property.\n"
                            "@param value value to set for this property, removing all other values");
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

    if (!rdfCppType.second.empty()) {
        cvt::RdfTypeEnum rdfType = rdfCppType.first;
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const " << cppType << "& value) {" << std::endl;
        indent(ofs, 2) << "object().setPropertyValue(\"" << _decorated.rdfname() <<
            "\", autordf::PropertyValue().set<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) <<
            ">(value));" << std::endl;
        indent(ofs, 2) << "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    } else {
        indent(ofs, 1) << currentClassName << "& set" << _decorated.prettyIRIName(true) << "(const autordf::PropertyValue& value) {" << std::endl;
        indent(ofs, 2) << "object().setPropertyValue(\"" << _decorated.rdfname() << "\", value);" << std::endl;
        indent(ofs, 2) << "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    }
    */
    indent(ofs, 1) << "// TODO: setter " << methodName << " for one" << std::endl;
}

void DataProperty::generateGetterForOneOptional(std::ostream& ofs, const Klass& onClass) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    std::string methodName = _decorated.prettyIRIName() + "Optional";
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "@return the valueif it is set, or nullptr if it is not set.");

    if (!rdfCppType.second.empty()) {
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << "std::shared_ptr<" << cppType << "> " << methodName << "() const;" << std::endl;
    } else {
        indent(ofs, 1) << "std::shared_ptr<autordf::PropertyValue> " << methodName << "() const;" << std::endl;
    }

    ofs << std::endl;
    methodName = _decorated.prettyIRIName();
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "@return the valueif it is set, or defaultval if it is not set.");

    if (!rdfCppType.second.empty()) {
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << cppType << " " << _decorated.prettyIRIName() << "(const " << cppType << "& defaultval) const;" << std::endl;
    } else {
        indent(ofs, 1) << "autordf::PropertyValue " << _decorated.prettyIRIName() << "(const autordf::PropertyValue& defaultval) const;" << std::endl;
    }
}

void DataProperty::generateGetterForMany(std::ostream& ofs, const Klass& onClass) const {
    std::string methodName = _decorated.prettyIRIName() + "List";
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "@return the list of values.  List can be empty if not values are set in database");
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    if (!rdfCppType.second.empty()) {
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << "std::vector<" << cppType << "> " << methodName << "() const;" << std::endl;
    } else {
        indent(ofs, 1) << "autordf::PropertyValueVector " << methodName << "() const;" << std::endl;
    }
}

void DataProperty::generateSetterForMany(std::ostream& ofs, const Klass& onClass) const {
    std::string methodName = "set" + _decorated.prettyIRIName(true);
    /*generatePropertyComment(ofs, onClass, methodName, 1,
                    "Sets property to list of values \n@param values the list of values");
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

    if (!rdfCppType.second.empty()) {
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const std::vector<" << cppType << "> " << "& values);" << std::endl;
    } else {
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const autordf::PropertyValueVector& values) {" << std::endl;
        indent(ofs, 2) <<     "object().setPropertyValueList(\"" << _decorated.rdfname() << "\", values, " << orderedBoolValue() << ");" << std::endl;
        indent(ofs, 2) <<     "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    }
    ofs << std::endl;

    methodName = "add" + _decorated.prettyIRIName(true);
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "Adds a value to a property \n@param value the value to add");
    if (!rdfCppType.second.empty()) {
        cvt::RdfTypeEnum rdfType = rdfCppType.first;
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const " << cppType << "& value) {" << std::endl;
        indent(ofs, 2) << "object().addPropertyValue(\"" << _decorated.rdfname() <<
        "\", autordf::PropertyValue().set<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) <<
        ">(value), " << orderedBoolValue() << ");" << std::endl;
        indent(ofs, 2) << "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    } else {
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const autordf::PropertyValue& value) {" << std::endl;
        indent(ofs, 2) << "object().addPropertyValue(\"" << _decorated.rdfname() << "\", value, " << orderedBoolValue() << ");" << std::endl;
        indent(ofs, 2) << "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    }*/
	indent(ofs, 1) << "// TODO: " << methodName << " for many" << std::endl;
}

void DataProperty::generateRemover(std::ostream& ofs, const Klass& onClass) const {
    std::string methodName = "remove" + _decorated.prettyIRIName(true);
    generatePropertyComment(ofs, onClass, methodName, 1,
                    "Remove a value for this property.\n"
                            "@param value to remove for this property.\n"
                            "@throw PropertyNotFound if propertyIRI has not obj as value\n");
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);

    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

    if (!rdfCppType.second.empty()) {
        cvt::RdfTypeEnum rdfType = rdfCppType.first;
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const " << cppType << "& value) {" << std::endl;
        indent(ofs, 2) << "object().removePropertyValue(\"" << _decorated.rdfname() <<
        "\", autordf::PropertyValue().set<autordf::cvt::RdfTypeEnum::" << cvt::rdfTypeEnumString(rdfType) <<
        ">(value));" << std::endl;
        indent(ofs, 2) << "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    } else {
        indent(ofs, 1) << currentClassName << "& " << methodName << "(const autordf::PropertyValue& value) {" << std::endl;
        indent(ofs, 2) << "object().removePropertyValue(\"" << _decorated.rdfname() << "\", value);" << std::endl;
        indent(ofs, 2) << "return *this;" << std::endl;
        indent(ofs, 1) << "}" << std::endl;
    }
}

int DataProperty::generateSetIndices(std::ostream& ofs, const Klass& onClass, int i) const {
    std::string currentClassName = "I" + onClass.decorated().prettyIRIName();

	indent(ofs, 2) << currentClassName << "::i_" << name() << " = *(base_ptr + " << i << ");" << std::endl;
	i++;
	if (_decorated.maxCardinality(onClass.decorated()) > 1) {
		indent(ofs, 2) << currentClassName << "::c_" << name() << " = *(base_ptr + " << i + 1 << ");" << std::endl;
		i++;
	}
	return i;
}

void DataProperty::generateSaverValuesDecl(std::ostream& ofs, const Klass& onClass) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    std::string currentClassName = onClass.decorated().prettyIRIName();

    if (rdfCppType.second.empty()) {
        indent(ofs, 1) << "std::set<autordf::PropertyValue> " << currentClassName << "_" << name() << "_values;" << std::endl;
    } else {
        std::string cppType = rdfCppType.second;
        indent(ofs, 1) << "std::set<" << cppType << "> " << currentClassName << "_" << name() << "_values;" << std::endl;
    }
    if (_decorated.maxCardinality(onClass.decorated()) > 1) {
        indent(ofs, 1) << "std::vector<uint64_t> " << currentClassName << "_" << name() << "_ARR;" << std::endl;
    }
}

void DataProperty::generateSaverValuesSet(std::ostream& ofs, const Klass& onClass) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    std::string currentClassName = onClass.decorated().prettyIRIName();
    std::string propType = rdfCppType.second.empty() ? "autordf::PropertyValue" : rdfCppType.second;

	if (_decorated.maxCardinality(onClass.decorated()) <= 1) {
		if (_decorated.minCardinality(onClass.decorated()) > 0) {
			indent(ofs, 2) << currentClassName << "_" << name() << "_values.insert("
				<< "obj." << _decorated.prettyIRIName() << "());" << std::endl;
		} else {
			indent(ofs, 2) << "{" << std::endl;
			indent(ofs, 3) << "std::shared_ptr<" << propType << "> tmp = obj." << _decorated.prettyIRIName() << "Optional();" << std::endl;
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

int DataProperty::generateSaverInstanceSave(std::ostream& ofs, const Klass& onClass, const Klass& storageClass, int propOffset) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    std::string storageClassName = storageClass.decorated().prettyIRIName();
    std::string currentClassName = onClass.decorated().prettyIRIName();
    std::string propType = rdfCppType.second.empty() ? "autordf::PropertyValue" : rdfCppType.second;

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
			indent(ofs, 3) << "std::shared_ptr<" << propType << "> tmp = obj." << _decorated.prettyIRIName() << "Optional();" << std::endl;
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

void DataProperty::generateSaverGenLoaderData(std::ostream& ofs, const Klass& onClass) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    std::string storageClassName = onClass.decorated().prettyIRIName();
    std::string propType = rdfCppType.second.empty() ? "autordf::PropertyValue" : rdfCppType.second;
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;

	indent(ofs, 1) << "ofs << \"static const size_t " << storageClassName << "_SL_" << _decorated.prettyIRIName() << " = " << storageClassName << "_" << name() << "_values.size();\\n\";" << std::endl;
    if (!rdfCppType.second.empty()) {
    	indent(ofs, 1) << "ofs << \"static " << rdfCppType.second << " " << storageClassName << "_S_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
    	indent(ofs, 2) << "ofs << \", \" << v << std::endl;" << std::endl;
    	indent(ofs, 1) << "}" << std::endl;
    	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;
	} else {
    	indent(ofs, 1) << "ofs << \"static std::string " << storageClassName << "_SV_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
    	indent(ofs, 2) << "ofs << \", \" << v << std::endl;" << std::endl;
    	indent(ofs, 1) << "}" << std::endl;
    	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;

    	indent(ofs, 1) << "ofs << \"static std::string " << storageClassName << "_SLANG_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
    	indent(ofs, 2) << "ofs << \", \" << v.lang() << std::endl;" << std::endl;
    	indent(ofs, 1) << "}" << std::endl;
    	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;

    	indent(ofs, 1) << "ofs << \"static std::string " << storageClassName << "_STYPE_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_values) {" << std::endl;
    	indent(ofs, 2) << "ofs << \", \" << v.dataTypeIri() << std::endl;" << std::endl;
    	indent(ofs, 1) << "}" << std::endl;
    	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;
	}
	if (forMany) {
    	indent(ofs, 1) << "ofs << \"static uint64_t " << storageClassName << "_SARR_" << _decorated.prettyIRIName() << "[] = {\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "ofs << \"0\" << std::endl;" << std::endl;
    	indent(ofs, 1) << "for (auto const& v: " << storageClassName << "_" << name() << "_ARR) {" << std::endl;
    	indent(ofs, 2) << "ofs << \", \" << v << std::endl;" << std::endl;
    	indent(ofs, 1) << "}" << std::endl;
    	indent(ofs, 1) << "ofs << \"};\" << std::endl;" << std::endl;
	}
}

void DataProperty::generateSaverGenLoaderLoad(std::ostream& ofs, const Klass& onClass) const {
    std::pair<cvt::RdfTypeEnum, std::string> rdfCppType = getRdfCppTypes(onClass);
    std::string storageClassName = onClass.decorated().prettyIRIName();
    std::string propType = rdfCppType.second.empty() ? "autordf::PropertyValue" : rdfCppType.second;
    bool forMany = _decorated.maxCardinality(onClass.decorated()) > 1;
	indent(ofs, 1) << "ofs << \"" << storageClassName << "::initS_" << _decorated.prettyIRIName() << "(";
	if (rdfCppType.second.empty()) {
		ofs << storageClassName << "_SV_" << _decorated.prettyIRIName()
			<< ", " << storageClassName << "_SLANG_" << _decorated.prettyIRIName()
			<< ", " << storageClassName << "_STYPE_" << _decorated.prettyIRIName();
	} else {
		ofs << storageClassName << "_S_" << _decorated.prettyIRIName();
	}
	if (forMany) {
		 ofs << ", " << storageClassName << "_SARR_" << _decorated.prettyIRIName();
	}
	ofs << ", " << storageClassName << "_SL_" << _decorated.prettyIRIName() << ");\\n\";" << std::endl;
}

std::string DataProperty::orderedBoolValue() const {
    return _decorated.ordered() ? "true" : "false";
}

}
}
