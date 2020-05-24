#ifndef AUTORDF_CODEGEN_OBJECTPROPERTY_H
#define AUTORDF_CODEGEN_OBJECTPROPERTY_H

#include <memory>
#include <string>
#include <map>
#include <iosfwd>

#include <autordf/ontology/ObjectProperty.h>
#include "RdfsEntity.h"

namespace autordf {

namespace codegen {

class Klass;

class ObjectProperty: public RdfsEntity {
    const ontology::ObjectProperty& _decorated;
public:
    ObjectProperty(const ontology::ObjectProperty& decorated) : RdfsEntity(decorated), _decorated(decorated) {}

    void generateDeclaration(std::ostream& ofs, const Klass& onClass) const;

    void generateDefinition(std::ostream& ofs, const Klass& onClass) const;

    void generateKeyDeclaration(std::ostream& ofs, const Klass& onClass, size_t propIndex, size_t propCount) const;

    int generateSetIndices(std::ostream& ofs, const Klass& onClass, int i) const;

    std::string name() const {
    	return _decorated.prettyIRIName();
    }

    uint64_t storageSize(const Klass&) const {
    	// either (klassId, identity) or (index, count)
    	return 2;
    }

    void generateSaverValuesDecl(std::ostream& ofs, const Klass& onClass) const;

    void generateSaverValuesSet(std::ostream& ofs, const Klass& onClass) const;

    int generateSaverInstanceSave(std::ostream& ofs, const Klass& onClass, const Klass& storageClass, int propOffset) const;

	void generateSaverGenLoaderData(std::ostream& ofs, const Klass& onClass) const;

	void generateSaverGenLoaderLoad(std::ostream& ofs, const Klass& onClass) const;

private:
    /**
     * Gets the class for this property, when instanciated as class "onClass" attribute
     */
    Klass effectiveClass(const Klass& onClass) const;

    void generateDeclarationSetterForOne(std::ostream& ofs, const Klass& onClass) const;

    void generateDeclarationSetterForMany(std::ostream& ofs, const Klass& onClass) const;

    void generateRemoverDeclaration(std::ostream& ofs, const Klass& onClass) const;

    void generateDefinitionSetterForOne(std::ostream& ofs, const Klass& onClass) const;

    void generateDefinitionSetterForMany(std::ostream& ofs, const Klass& onClass) const;

    void generateRemoverDefinition(std::ostream& ofs, const Klass& onClass) const;

    void generateStorage(std::ostream& ofs, const Klass& onClass, bool forMany) const;

    std::string orderedBoolValue() const;
};
}
}
#endif //AUTORDF_CODEGEN_OBJECTPROPERTY_H
