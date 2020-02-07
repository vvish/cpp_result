The built-in integral types are used as a low-level result storage that should provide efficient access and manipulation via bit operations.
The storage type can be specified to achieve portability and efficiency for various platforms.

## Motivation

The framework makes an attempt to provide resource efficient ways to define error types and facilities to analyze them.
In a resource constrained environment it would be convenient to have facilities to represent errors and ability to combine several of them
without overhead of memory allocation/deallocation.

The type is supposed to be project defined and concrete result values can be declared closer to the usage. That will enable clients to use unified methods to analyze and forward
errors up to the call stack while providing some encapsulation.


## Introduction

The framework allows clients to specify result code along with a set of error categories that can be used to describe the possible error in a more precise way. The underlying type can be specified
as a template parameter.

In the example the type to store the result code is defined using 8-bit integral type.
The error type is supposed to have the following structure:

2 bits for domain, 2 bits for sub-domain. 4 bits for error code


```c++
namespace application
{
//project-wide definition of error types

//definition of some categories and their widths in bits
//(here 2 bits allowing maximum of 4 categories on each level)
MAKE_RESULT_CATEGORY(Domain, 2);
MAKE_RESULT_CATEGORY(SubDomain, 2);

// definition of the result type (contains two categories
// defined above and based on 8 bit integer
// the rest 4 bits are used for result code)
MAKE_RESULT_TYPE(TestResult, uint8_t, Domain, SubDomain);

//definition of the error domains and subdomains

// here the error can occure in the application layer
// domain having value 3 is defined
constexpr auto Application = Domain{3};

// two subdomains defined
constexpr auto Server = SubDomain{1};
constexpr auto Client = SubDomain{2};

//error codes for concrete errors
constexpr uint8_t rpcErrorCode = 1;
constexpr uint8_t backendAccessErrorCode = 2;

//local alias for the positive result
constexpr auto Ok = TestResult::success;

//error definitions consisting of categories (Domain, SubDomain) and error codes
constexpr auto rpcClientError = TestResult::make(Application, Client, rpcErrorCode);
constexpr auto backendAccessError = TestResult::make(Application, Client, backendAccessErrorCode);

} // namespace application

```

