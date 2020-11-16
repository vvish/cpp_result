# The minimalist error handling framework

[![codecov](https://codecov.io/gh/vvish/cpp_result/branch/master/graph/badge.svg)](https://codecov.io/gh/vvish/cpp_result)

## Motivation

The framework makes an attempt to provide resource efficient ways to define 
error types and facilities to analyze them. In a resource-constrained 
environment it would be convenient to have facilities to represent errors and 
ability to combine several of them without overhead of memory 
allocation/deallocation and access indirection.

The built-in integral types are used as a low-level result storage that should
provide efficient access and manipulation via bit operations. The storage type
can be specified to achieve portability and efficiency for various platforms.

The result type is supposed to be project defined and concrete result values
can be declared closer to the usage. That will enable clients to use unified
methods to analyze and forward errors up to the call stack while providing some
encapsulation.

## Build

The library requires C++17.
Makefile in the project root can be used to build and execute unit-tests.

To build and execute tests the following make target can be used:

```sh
make test
```

And to build the example:

```sh
make example
```

The executables containing unit-tests and examples can then be found in the
`bin` subfolder.

## Getting started

As the library is header-only, to be used in the project it is sufficient to add 
the `include` folder to the project include path.

The project-specific layout for result type should be defined.
It can have several layers of categories pointing at project submodules where
error can occur. In the result type declaration they are identified via
types that work as names for the layer.

In the example two levels of categories are defined each having 2 bits of size
(capable of representing 4 values each).
```c++
MAKE_RESULT_CATEGORY(Category, 2);
MAKE_RESULT_CATEGORY(SubCategory, 2);
```

The result type declaration should contain name, specification of the
underlying type and a list of categories to incorporate. Here the definition
uses 8-bit integral type as a low-level storage and has 2 bits for category, 2
bits for subcategory and 4 remaining bits for an error code.
The result type is defined with the name `Result`.

```c++
MAKE_RESULT_TYPE(Result, uint8_t, Category, SubCategory);
```
The aggregate result can be defined to combine several single results.
In the example it is capable of containing 32/8 bit = 4 result objects.

```c++
MAKE_AGGREGATE_RESULT_TYPE(AggregateResult, uint32_t, Result);
```

Objects of different categories can be created to identify errors that can
occur in different program modules. They can be placed in corresponding
namespaces.

```c++
constexpr auto Ui = Category{1};
constexpr auto DataModel = SubCategory{1};

constexpr auto Backend = Category{2};
constexpr auto Db = SubCategory{1};
constexpr auto Rpc = SubCategory{2};
```

The error definitions can be placed globally, in the corresponding modules
or nested in the class declarations.

```c++
constexpr auto dataRetrievalError = Result::make(Ui, DataModel, 1);

constexpr auto backendAccessErrorCode = 1;
constexpr auto backendAccessError
    = Result::make(Backend, DataAccess, backendAccessErrorCode);

class BackendClient {
public:
    static constexpr Result rpcError = Result::make(Backend, Rpc, 1);
    static constexpr Result wrongQuery = Result::make(Backend, Db, 1);
// ...
};
```

The client code can analyze returned result objects.

```c++
Result const result = ...;
if (respp::is_success(result)) {
} else if (respp::get_category<SubCategory>(result) == Rpc) {
} else if (respp::get_code(result) == backendAccessErrorCode) {}
```

Several errors can be combined (nested) in aggregate result:

```c++
AggregateResult result {backendAccessError, wrongQuery};
return result << dataRetrievalError;
```

The aggregated errors can be iterated to traverse 'error stack'.

```c++
AggregateResult const result = ...;
for (auto const it : result.iterate_errors()) {
    if (it == application::backend_access::BackendClient::rpcError) {
    }
}
```

For more complete examples please refer to `examples/example.cpp` 
and unit-tests `test/result_test.cpp`.