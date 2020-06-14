#include <autordf/Factory.h>

#include <foaftest/All.h>

#include "Loader.cpp"

using namespace foaftest;

int main() {
    autordf::Factory f;
    autordf::Object::setFactory(&f);

	loadAll();

	 // Or directly
    std::vector<foaf::Person> list2 = foaf::Person::find();
    foaf::Person p2 = list2.front();
    std::cout << "Found " << list2.size() << " persons" << std::endl;
    for(auto p: list2) {
    	std::cout << " " << p.nameList().size() << " name(s)" << std::endl;
    	for (auto n: p.nameList()) {
    		std::cout << " - " << n << std::endl;
    	}
    }
    // FIXME: why is givenName Lost ?
    // std::cout << "  knows : ";
    // for ( const foaf::Person& o : p2.knowsList() ) {
    //     std::cout << o.nameList().front() << ", ";
    // }
}