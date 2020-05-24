#include <autordf/Factory.h>
#include <autordf/Object.h>
#include <foaftest/Saver.h>

#include <boost/filesystem.hpp>

#include <iostream>

int main() {
    autordf::Factory f;
    autordf::Object::setFactory(&f);

    f.loadFromFile(boost::filesystem::path(__FILE__).parent_path().string() + "/../../unittests/foafExample.ttl");

    Saver::saveAll(".");
}