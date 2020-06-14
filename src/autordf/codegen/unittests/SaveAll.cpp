#include <autordf/Factory.h>
#include <autordf/Object.h>
#include <foaftest/Saver.h>

#include <boost/filesystem.hpp>

#include <iostream>

int main(int argc, char** argv) {
    autordf::Factory f;
    autordf::Object::setFactory(&f);
    if (argc != 2) {
    	std::cerr << "Usage: " << argv[0] << "<INPUT DATA TTL>" << std::endl;
    	return 1;
    }
    f.loadFromFile(argv[1]);

    Saver::saveAll(".");
}