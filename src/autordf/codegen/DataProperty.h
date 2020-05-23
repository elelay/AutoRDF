#ifndef AUTORDF_CODEGEN_DATAPROPERTY_H
#define AUTORDF_CODEGEN_DATAPROPERTY_H

#include <map>
#include <memory>
#include <ostream>

#include <autordf/ontology/DataProperty.h>
#include <autordf/cvt/RdfTypeEnum.h>
#include "RdfsEntity.h"

namespace autordf {

namespace codegen {

class Klass;

class DataProperty : public RdfsEntity {
    const ontology::DataProperty& _decorated;
public:
    DataProperty(const ontology::DataProperty& decorated) : RdfsEntity(decorated), _decorated(decorated) {}

    void generateDeclaration(std::ostream& ofs, const Klass& onClass) const;

    void generateDefinition(std::ostream& ofs, const Klass& onClass) const;

    void generateKeyDeclaration(std::ostream& ofs, const Klass& onClass, size_t propIndex, size_t propCount) const;

    int generateSetIndices(std::ostream& ofs, const Klass& onClass, int i) const;

    void generateSaverValuesDecl(std::ostream& ofs, const Klass& onClass) const;

    void generateSaverValuesSet(std::ostream& ofs, const Klass& onClass) const;

    std::string name() const {
    	return _decorated.prettyIRIName();
    }

    uint64_t storageSize(const Klass& onClass) const;

private:
    std::pair<cvt::RdfTypeEnum, std::string> getRdfCppTypes(const Klass& onClass) const;

    void generateGetterForOneMandatory(std::ostream& ofs, const Klass& onClass) const;

    void generateGetterForOneOptional(std::ostream& ofs, const Klass& onClass) const;

    void generateGetterForMany(std::ostream& ofs, const Klass& onClass) const;

    void generateSetterForOne(std::ostream& ofs, const Klass& onClass) const;

    void generateSetterForMany(std::ostream& ofs, const Klass& onClass) const;

    void generateRemover(std::ostream& ofs, const Klass& onClass) const;

    void generateStorage(std::ostream& ofs, const Klass& onClass, bool optional, bool forMany) const;

    std::string orderedBoolValue() const;
};

}
}

#endif //AUTORDF_CODEGEN_DATAPROPERTY_H
