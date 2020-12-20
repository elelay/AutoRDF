# idocdb static runtime model

## Mission statement

This model is useful when working with an immutable autordf model, ie. never adding tuples.

Its design goal is to be as fast as possible to load and work with and with a smaller memory footprint
while retaining API compatibility with the classic model.

## Compatibility

Compatibility with code targeting the classic autordf API.

Supported are:
 - all data types
 - `GenClass::findBy<Property>()`
 - autordf::Object identity (see Objects, but still to implement)
 
Unsupported are:
 - getProperty as string when the property is a native type (int, ...);
 - in general directly manipulating the semantic web tuples;
 - creating, modifying or deleting objects.

See Objects for modifications to the autordf::Object type.

## Object model

Data is stored in static arrays of different types and "mounted" at runtime initialization in classes.
See Loader.cpp's loadAll() function.

Global state (all classes, instances and object URIs) is stored in the All class.

### Data Values

Property values are stored deduplicated in static arrays.

TODO: sorted for dichotomic lookup by value.

Native types are stored as is (bool, long, ...), not as strings to be parsed

Strings are stored as std::string arrays.
TODO: char array + string_view for 0 allocations at startup?


### Classes

Each class CLASSNAME in the model has an index CLASSNAME::KLASS_ID, determined at code generation.
The index is used to easily identify the concrete type of an object in an ObjectProperty.
A class has also a list of all its instances: see Objects.

All instances of the class CLASSNAME are mounted in All::INSTANCES[CLASSNAME::KLASS_ID] and also in CLASSNAME::INSTANCES.

All::PARENT_OFFSET[CHILD_CLASS::KLASS_ID][PARENT_CLASS::KLASS_ID] indicates the offset of the first property of the class PARENT_CLASS in
its subclass CHILD_CLASS.


### Objects

All instances of a class are stored in an uint64_t array CLASSNAME_INSTANCES.

An instance occupies a given consecutive number of slots in the array.
The slot count depends on properties, as seen below.

Inherited properties occupy the first slot(s), according to inheritance order.

autordf::Object is extended to add 2 int64_t fields:
 - `_klass` -- the class index
 - `_identity` -- the object start offset in CLASSNAME_INSTANCES


`autordf::Object::isA<CLASSNAME>()` implementation is provided by checking that the object's
`_klass` field is CLASSNAME's index.

URI to object mapping is provided via the `std::map<std::string, std::pair<int64_t,int64_t>>` All::URI_TO_OBJECT.
It maps each object's URI to the static model's CLASSNAME's index and object index in CLASSNAME_INSTANCES.

TODO: actually use it...

### Data Properties

A data property is a property with a non object type: integer, float, boolean, date, string, enum, etc.
TODO: list all types.


### Single value

A single valued property occupies a single slot in the INSTANCES array.
Its value is the index in the CLASSNAME_SV_PROPNAME.

Optional properties can point to the 0 index, representing NO VALUE.
Required properties shall never have the 0 index.

### Multiple values

For multiple valued properties, an indirection uint64_tarray named CLASSNAME_SARR_PROPNAME is required.
It stores indice sequences for the property PROPNAME of an instance of CLASSNAME.
Sorted properties have their indices in the correct order, order is unspecified otherwise.

A multiple valued property occupies therefore 2 slots in the INSTANCES array:
 - first the index in the CLASSNAME_SARR_PROPNAME array of the first value
 - second the length of the value sequence ie how many values has the instance's property.

If the length is 0, the index could be anything. It is set to 0 for clarity when debugging.


### rdfs::literal

A literal is a string with a lang and type. To account for this, additional arrays are created:
 - CLASSNAME_SLANG_PROPNAME
 - CLASSNAME_STYPE_PROPNAME

When a data property refers to a literal, the index is the same for all 3 arrays (CLASSNAME_SV_PROPNAME and the 2 others).

FIXME: how are they sorted?

## Object Properties (aka relations)

Referencing an object takes 2 pieces of information:
 - CLASSNAME_SK_PROPNAME[i] -- the actual class PROPCLASS of the object pointed to (it can be a subclass of the propertie's type),
   as an index in the global classes array;
 - CLASSNAME_SI_PROPNAME[i] -- the index in the PROPCLASS_INSTANCES array.


### Single values

An object property occupies 1 slot in the CLASSNAME_INSTANCES array:
 - the index in CLASSNAME_SK_PROPNAME and CLASSNAME_SI_PROPNAME.

An optional property shall have 0, 0 as value to denote an empty value.

### Multiple values

It's identical to a multiple valued data property.

For multiple valued properties, an indirection uint64_tarray named CLASSNAME_SARR_PROPNAME is required.
It stores indice sequences for the property PROPNAME of an instance of CLASSNAME.
Sorted properties have their indices in the correct order, order is unspecified otherwise.

A multiple valued property occupies therefore 2 slots in the INSTANCES array:
 - first the index in the CLASSNAME_SARR_PROPNAME array of the first value
 - second the length of the value sequence ie how many values has the instance's property.

If the length is 0, the index could be anything. It is set to 0 for clarity when debugging.


## Generated code

Generated code for IClasses and Classes has the same API as the classic model.

Generates a C++ module, with many static arrays for storage and a loadAll() method.

TODO: example? namespace
