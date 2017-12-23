## C++ programming style

The goal of the programming style used in SRE is to provide a simple API for novice and intermediate C++ programmers.

Rules:

* No exceptions
* Object oriented API
* Memory management mainly using shared pointers or using plain old data (POD) for simple data types
* Builder patterns instead of constructor parameters for complex objects
* Using STL containers
* Limited use of move semantic