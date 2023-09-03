// Code example for the library usage. Demonstates the basic layout for a
// sample project having several sub-components. Apart from this purpose the
// code is meaningless.

#include "respp/result.hpp"

namespace application
{
// project-wide definitions of error types

// definition of error categories and their widths in bits
// (here 2 bits allowing maximum of 4 categories on each level)
MAKE_RESULT_CATEGORY(Category, 2);
MAKE_RESULT_CATEGORY(SubCategory, 2);

// definition of the result type (contains two categories
// defined above and based on 8 bit integer
// the rest 4 bits are used for result code)
MAKE_RESULT_TYPE(Result, uint8_t, Category, SubCategory);
// and aggregate result capable of containing 32/8 = 4 errors
MAKE_AGGREGATE_RESULT_TYPE(AggregateResult, uint32_t, Result);

// local alias for the positive result can be defined
constexpr auto Ok = Result::success;

// application error categories
constexpr auto Ui = Category{1};
constexpr auto Backend = Category{2};

// some sample submodules having various responsibilities
namespace backend_access
{
// sub-categories defined for DB, Rpc and more common errors
constexpr auto Db = SubCategory{1};
constexpr auto Rpc = SubCategory{2};
constexpr auto DataAccess = SubCategory{3};

// sample RPC client for backend access
class BackendClient {
public:
    // localy defined error code
    static constexpr Result rpcError = Result::make(Backend, Rpc, 1);
    static constexpr Result wrongQuery = Result::make(Backend, Db, 1);

    // method return single result
    Result executeRemoteQuery()
    {
        // RPC failed
        return rpcError;
    }
};

constexpr Result BackendClient::rpcError;
constexpr Result BackendClient::wrongQuery;

// shared error codes can be used
constexpr uint8_t backendAccessErrorCode = 2;
constexpr auto backendAccessError
    = Result::make(Backend, DataAccess, backendAccessErrorCode);

// method returns aggregated result
AggregateResult retrieveRemoteData()
{
    backend_access::BackendClient client;
    Result const result = client.executeRemoteQuery();
    // handling error
    if (!respp::is_success(result)) {
        if (respp::get_category<SubCategory>(result) == Rpc) {
            // in case of RPC error we return more common error and nest
            // the error from lower layer
            return AggregateResult{backendAccessError, result};
        } else {
            // ignore other types of error
            // aggregated result can be constructed from single result
            return Ok;
        }
    }

    return result;
}

}  // namespace backend_access

namespace ui
{
constexpr auto DataModel = SubCategory{1};

constexpr auto dataRetrievalError = Result::make(Ui, DataModel, 1);

AggregateResult retrieveData()
{
    auto result = backend_access::retrieveRemoteData();
    if (respp::is_success(result)) {
        // positive case
        return {};
    } else {
        // append error from upper layer
        return result << dataRetrievalError;
    }
}

}  // namespace ui
}  // namespace application

int main()
{
    auto const result = application::ui::retrieveData();
    if (respp::is_success(result)) {
        return 0;
    } else {
        // if the errors are in the public interfaces of the modules we can
        // collect them here and output readable information or just codes
        for (const auto it : result.iterate_errors()) {
            if (it == application::backend_access::BackendClient::rpcError) {
                // some logic or reporting
            }
        }
    }
}
