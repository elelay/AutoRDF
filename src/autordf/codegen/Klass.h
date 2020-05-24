#ifndef AUTORDF_CODEGEN_KLASS_H
#define AUTORDF_CODEGEN_KLASS_H

#include <set>
#include <map>
#include <memory>
#include <string>

#include "RdfsEntity.h"
#include "DataProperty.h"
#include "ObjectProperty.h"

#include <autordf/ontology/Klass.h>
#include "RdfsEntity.h"

namespace autordf {
namespace codegen {

class Klass : public RdfsEntity {
    const ontology::Klass& _decorated;
public:
    Klass(const ontology::Klass& decorated) : RdfsEntity(decorated), _decorated(decorated) {}

    const ontology::Klass& decorated() const { return _decorated; }

    std::set <std::shared_ptr<const Klass>> getClassDependencies() const;

    void generateInterfaceDeclaration() const;

    void generateInterfaceDefinition() const;

    void generateDeclaration(uint64_t iKlass) const;

    void generateDefinition(uint64_t iKlass) const;

    void enterNameSpace(std::ofstream& ofs) const;
    void leaveNameSpace(std::ofstream& ofs) const;

    Klass uri2Klass(const std::string& uri) const;

    void generateSaverInstanceSave(std::ofstream& ofs, uint64_t iKlass) const;
    void generateSaverInitKlassIdsIdentities(std::ofstream& ofs, uint64_t iKlass) const;

	void generateSaverGenLoaderData(std::ostream& ofs, uint64_t iKlass) const;
	void generateSaverGenLoaderLoad(std::ostream& ofs, uint64_t iKlass) const;

    size_t storageSize() const;

private:
};

}
}
#endif //AUTORDF_CODEGEN_KLASS_H
