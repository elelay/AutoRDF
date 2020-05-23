#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <map>
#include <memory>

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>

#include <autordf/Factory.h>
#include <autordf/Object.h>
#include <autordf/ontology/Ontology.h>

#include "codegen/DataProperty.h"
#include "codegen/ObjectProperty.h"
#include "codegen/Klass.h"
#include "codegen/Utils.h"

namespace autordf {
namespace codegen {

bool verbose = false;
bool generateAllInOne = false;

// class KlassNode {
// public:
// 	KlassNode(autordf::Uri k): klass(k) {}

// public:
// 	autordf::Uri k;
// 	std::list<KlassNode> descendants;
// }

// void makeKlassTree(KlassNode& n, const std::map<uint64_t, std::set<uint64_t>>& klassDescendants, const std::map<uint64_t, autordf::Uri>& indexKlasses) {
	
// }

void run(Factory *f) {
    ontology::Ontology ontology(f, verbose);

    // Starting code Generation
    std::set<std::string> cppNameSpaces;

    std::map<autordf::Uri, uint64_t> klassIndices;
    std::map<uint64_t, autordf::Uri> indexKlasses;

    uint64_t iKlass = 1;
    for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
        klassIndices[(*klassMapItem.second).rdfname()] = iKlass;
        indexKlasses[iKlass] = (*klassMapItem.second).rdfname();
        iKlass++;
    }
    // std::list<KlassNode> rootKlasses;
    std::list<Uri> rootKlasses;
	std::map<uint64_t, std::set<uint64_t>> klassDescendants;
    for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
    	auto const& ancestors = klassMapItem.second->getAllAncestors();
		for(auto const& ancestor: ancestors) {
			klassDescendants[klassIndices[ancestor->rdfname()]].insert(klassIndices[(*klassMapItem.second).rdfname()]);
		}
		if (ancestors.empty()) {
			rootKlasses.push_back((*klassMapItem.second).rdfname());
		}
	}
	// std::map<uint64_t, std::vector<uint64_t>> klassSortedDescendants;
	// for (auto const& k: klassDescendants) {
	// 	std::vector<uint64_t> desc(k.second.begin(), k.second.end());
	// 	std::sort(desc.begin(), desc.end(), [&klassDescendants](uint64_t a, uint64_t b) {
	// 		if (klassDescendants.find(b) == klassDescendants.end()) {
	// 			return true;
	// 		} else {
	// 			auto bdesc = klassDescendants[b];
	// 			if (bdesc.find(a) == bdesc.end()) {
	// 				return false;
	// 			}
	// 		}
	// 		if (klassDescendants.find(a) == klassDescendants.end()) {
	// 			return false;
	// 		} else {
	// 			auto adesc = klassDescendants[a];
	// 			if (adesc.find(b) == adesc.end()) {
	// 				return false;
	// 			}
	// 	    }
	// 		return false;
	// 	});
	// 	klassSortedDescendants[k.first] = desc;
	// }
	// for (auto const& kd: klassSortedDescendants) {
	// 	std::cout << indexKlasses[kd.first];
	// 	for (auto const& d: kd.second) {
	// 		std::cout << " -> " << indexKlasses[d];
	// 	}
	// 	std::cout << std::endl;
	// }

    for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
        iKlass = klassIndices[(*klassMapItem.second).rdfname()];
        // created directory if needed
        createDirectory(Klass(*klassMapItem.second).genCppNameSpaceInclusionPath());
        cppNameSpaces.insert(Klass(*klassMapItem.second).genCppNameSpace());

        Klass(*klassMapItem.second).generateInterfaceDeclaration();
        Klass(*klassMapItem.second).generateInterfaceDefinition();
        Klass(*klassMapItem.second).generateDeclaration(iKlass, klassDescendants[iKlass]);
        Klass(*klassMapItem.second).generateDefinition(iKlass);
    }


    // Generate all inclusions files
    for ( const std::string& cppNameSpace : cppNameSpaces ) {
        std::ofstream ofs;
        createFile(Klass::outdir + "/" + cppNameSpace + "/" + cppNameSpace + ".h", &ofs);

        generateCodeProtectorBegin(ofs, cppNameSpace, cppNameSpace);
        for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
            if ( Klass(*klassMapItem.second).genCppNameSpace() == cppNameSpace ) {
                const Klass& cls = *klassMapItem.second;
                ofs << "#include <" << cls.genCppNameSpaceInclusionPath() << "/" << klassMapItem.second->prettyIRIName() << ".h" << ">" << std::endl;
            }
        }
        ofs << std::endl;
        generateCodeProtectorEnd(ofs, cppNameSpace, cppNameSpace);
    }

    // Generate all in one cpp file
    if ( generateAllInOne ) {
        std::ofstream ofs;
        createFile(RdfsEntity::outdir + "/AllInOne.cpp", &ofs);

        addBoilerPlate(ofs);
        ofs << std::endl;
        for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
            const Klass& cls = *klassMapItem.second;
            ofs << "#include \"" << cls.genCppNameSpaceInclusionPath() << "/I" << klassMapItem.second->prettyIRIName() << ".cpp" << "\"" << std::endl;
            ofs << "#include \"" << cls.genCppNameSpaceInclusionPath() << "/" << klassMapItem.second->prettyIRIName() << ".cpp" << "\"" << std::endl;
        }
		ofs << "#include \"All.cpp\"" << std::endl;
        ofs << std::endl;
    }

    // Generate the World view
	size_t klassCount = ontology.classUri2Ptr().size();
    {
		std::ofstream ofs;
		createFile(RdfsEntity::outdir + "/All.h", &ofs);

		// ofs << "#include <map>" << std::endl;
		// ofs << std::endl;
        generateCodeProtectorBegin(ofs, "ALL", "ALL");
        ofs << std::endl;
        ofs << "#include <cstdint>" << std::endl;
        ofs << std::endl;
		ofs << "class All {" << std::endl;
		ofs << "public:" << std::endl;
		indent(ofs, 1) << "static uint64_t* INSTANCES[" << (klassCount + 1) << "];" << std::endl; // 0 is invalid
		indent(ofs, 1) << "static uint64_t PARENT_OFFSET[" << (klassCount + 1) << "][" << (klassCount + 1) << "];" << std::endl; // 0 is invalid
		ofs << std::endl;
		ofs << "};" << std::endl;
        generateCodeProtectorEnd(ofs, "ALL", "ALL");
	}

	{
		std::ofstream ofs;
		createFile(RdfsEntity::outdir + "/All.cpp", &ofs);

		ofs << "#include \"All.h\"" << std::endl;
		ofs << std::endl;

		ofs << "uint64_t* All::INSTANCES[" << (klassCount + 1) << "] = {0};" << std::endl; // 0 is invalid

		ofs << "uint64_t All::PARENT_OFFSET[" << (klassCount + 1) << "][" << (klassCount + 1) << "] = {" << std::endl; // 0 is invalid
		indent(ofs, 1) << "{ 0";
		for (uint64_t i=1; i < klassCount + 1; i++) {
			ofs << ", 0";
		}
		ofs << "}";
		for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
			const Klass& cls = *klassMapItem.second;
			ofs << "," << std::endl;;
			indent(ofs, 1) << "// " << cls.decorated().prettyIRIName() << std::endl;
			indent(ofs, 1) << "{ 0";
			auto const& ancestors = cls.decorated().getAllAncestors();
			std::vector<uint64_t> ancestorOffsets(klassCount + 1, 0);
			uint64_t offset = 0;
			for (auto const& ancestor: ancestors) {
				ancestorOffsets[klassIndices[ancestor->rdfname()]] = offset;
				for ( const std::shared_ptr<ontology::DataProperty>& prop : ancestor->dataProperties()) {
					offset += DataProperty(*prop.get()).storageSize(Klass(*ancestor.get()));
				}
				for ( const std::shared_ptr<ontology::ObjectProperty>& prop : ancestor->objectProperties()) {
					offset += ObjectProperty(*prop.get()).storageSize(Klass(*ancestor.get()));
				}
			}
			for (uint64_t i=1; i < ancestorOffsets.size(); i++) {
				ofs << ", " << ancestorOffsets[i];
			}
			ofs << "}";
		}
		ofs << "\n};" << std::endl;

		ofs << std::endl;
    }

    // Generate Saver
    {
		std::ofstream ofs;
		createFile(RdfsEntity::outdir + "/Saver.h", &ofs);

        generateCodeProtectorBegin(ofs, "ALL", "Saver");
        ofs << std::endl;
        ofs << "#include <cstdint>" << std::endl;
		ofs << "#include <vector>" << std::endl;
        ofs << std::endl;
		ofs << "class Saver {" << std::endl;
		ofs << "public:" << std::endl;
		indent(ofs, 1) << "static std::vector<uint8_t> saveAll();" << std::endl;
		ofs << "};" << std::endl;
        generateCodeProtectorEnd(ofs, "ALL", "Saver");
    }

	{
		std::ofstream ofs;
		createFile(RdfsEntity::outdir + "/Saver.cpp", &ofs);

		ofs << "#include \"Saver.h\"" << std::endl;
		ofs << std::endl;
		ofs << "#include <map>" << std::endl;
		ofs << "#include <set>" << std::endl;
		ofs << std::endl;

		ofs << "#include <autordf/PropertyValue.h>" << std::endl;
		ofs << std::endl;


		for ( const std::string& cppNameSpace : cppNameSpaces ) {
			ofs << "#include \"" << cppNameSpace << "/" << cppNameSpace + ".h\"" << std::endl;
		}
		ofs << std::endl;

		ofs << "std::vector<uint8_t> Saver::saveAll() {" << std::endl;

		indent(ofs, 1) << "/* Generate property storage */" << std::endl;
		for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
			const Klass& cls = *klassMapItem.second;
			if ((cls.decorated().dataProperties().empty() && cls.decorated().objectProperties().empty())) {
				indent(ofs, 1) << "// No property for " << cls.decorated().prettyIRIName() << std::endl;
			} else {
				indent(ofs, 1) << "// Storage for " << cls.decorated().prettyIRIName() << std::endl;
				for ( const std::shared_ptr<ontology::DataProperty>& prop : cls.decorated().dataProperties()) {
					DataProperty(*prop.get()).generateSaverValuesDecl(ofs, cls);
				}
				for ( const std::shared_ptr<ontology::ObjectProperty>& prop : cls.decorated().objectProperties()) {
					ObjectProperty(*prop.get()).generateSaverValuesDecl(ofs, cls);
				}
			}
		}

		for ( auto const& klassMapItem: ontology.classUri2Ptr()) {
			const Klass& cls = *klassMapItem.second;
			if ((cls.decorated().dataProperties().empty() && cls.decorated().objectProperties().empty())) {
				indent(ofs, 1) << "// No property for " << cls.decorated().prettyIRIName() << std::endl;
			} else {
				indent(ofs, 1) << "// Storage for " << cls.decorated().prettyIRIName() << std::endl;
				for ( const std::shared_ptr<ontology::DataProperty>& prop : cls.decorated().dataProperties()) {
					DataProperty(*prop.get()).generateSaverValuesDecl(ofs, cls);
				}
				for ( const std::shared_ptr<ontology::ObjectProperty>& prop : cls.decorated().objectProperties()) {
					ObjectProperty(*prop.get()).generateSaverValuesDecl(ofs, cls);
				}
				indent(ofs, 1) << "for(auto const& obj: " << cls.genCppNameWithNamespace(false) << "::find()) {" << std::endl;
				for ( const std::shared_ptr<ontology::DataProperty>& prop : cls.decorated().dataProperties()) {
					DataProperty(*prop.get()).generateSaverValuesSet(ofs, cls);
				}
				for ( const std::shared_ptr<ontology::ObjectProperty>& prop : cls.decorated().objectProperties()) {
					ObjectProperty(*prop.get()).generateSaverValuesSet(ofs, cls);
				}
				indent(ofs, 1) << "}" << std::endl;
			}
		}
		
		// indent(ofs, 1) << "// Generate instances" << std::endl;
		// indent(ofs, 1) << "std::map<autordf::Object, uint64_t> allObjectIdentities;" << std::endl;
		// indent(ofs, 1) << "std::map<autordf::Object, autordf::Uri> allObjectUris;" << std::endl;
		// indent(ofs, 1) << "for (const auto& obj: autordf::Object::findAll()) {" << std::endl;
		// for (auto const& root: rootKlasses) {
		// 	indent(ofs, 2) << "if (obj.isA(Uri(\"" << root << "\"))) {" << std::endl;
		// 	bool first = true;
		// 	for (auto it = klassSortedDescendants[klassIndices[root]].rbegin(); it != klassSortedDescendants[klassIndices[root]].rend(); it++) {
		// 		indent(ofs, 3);
		// 		if (first) {
		// 			first = false;
		// 		} else {
		// 			ofs << "} else ";
		// 		}
		// 		ofs << "if (obj.isA(Uri(\"" << indexKlasses[*it] << "\"))) {" << std::endl;
		// 		indent(ofs, 4) << "// TODO: add to instances" << std::endl;
		// 	}
		// 	indent(ofs, 3) << "} else {" << std::endl;
		// 	indent(ofs, 4) << "// TODO: add to root instances" << std::endl;
		// 	indent(ofs, 3) << "}" << std::endl;
		// 	indent(ofs, 2) << "}" << std::endl;
		// }
		// indent(ofs, 1) << "}" << std::endl;

		ofs << "}" << std::endl;

		ofs << std::endl;
    }


}

}
}

int main(int argc, char **argv) {
    autordf::Factory f;

    namespace po = boost::program_options;

    po::options_description desc("Usage: autordfcodegen [-v] [-a] [-n namespacemap] [-o outdir] owlfile1 [owlfile2...]\n"
                                 "\tProcesses an OWL file, and generates C++ classes from it in current directory\n");

    desc.add_options()
            ("help,h", "Describe arguments")
            ("verbose,v", "Turn verbose output on.")
            ("all-in-one,a", "Generate one cpp file that includes all the other called AllInOne.cpp")
            ("namespacemap,n", po::value< std::vector<std::string> >(), "Adds supplementary namespaces prefix definition, in the form 'prefix:namespace IRI'. Defaults to empty.")
            ("outdir,o", po::value< std::string >(), "Folder where to generate files in. If it does not exit it will be created. Defaults to current directory.")
            ("owlfile", po::value< std::vector<std::string> >(), "Input file (repeated)");

    po::positional_options_description p;
    p.add("owlfile", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    autordf::codegen::verbose = vm.count("verbose") > 0;

    if(vm.count("namespacemap")) {
        for(auto prefix_namespace: vm["namespacemap"].as< std::vector<std::string> >()) {
            std::stringstream ss(prefix_namespace);
            std::string prefix;
            std::getline(ss, prefix, ':');
            std::string ns;
            ss >> ns;
            if ( autordf::codegen::verbose ) {
                std::cout << "Adding  " << prefix << "-->" << ns << " map." << std::endl;
            }
            f.addNamespacePrefix(prefix, ns);
        }
    }

    if(vm.count("outdir")) {
        autordf::codegen::RdfsEntity::outdir = vm["outdir"].as<std::string>();
    }

    autordf::codegen::generateAllInOne = vm.count("all-in-one") > 0;;

    if (!(vm.count("owlfile"))) {
        std::cerr << "Expected argument after options" << std::endl;
        return 1;
    }

    try {

        autordf::codegen::createDirectory(autordf::codegen::RdfsEntity::outdir);

        // Hardcode some prefixes
        f.addNamespacePrefix("owl", autordf::ontology::Ontology::OWL_NS);
        f.addNamespacePrefix("rdfs", autordf::ontology::Ontology::RDFS_NS);

        for ( std::string owlfile: vm["owlfile"].as< std::vector<std::string> >() ) {
            if ( autordf::codegen::verbose ) {
                std::cout << "Loading " << owlfile << " into model." << std::endl;
            }
            f.loadFromFile(owlfile);
        }

        autordf::codegen::run(&f);

    } catch(const std::exception& e) {
        std::cerr << "E: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
