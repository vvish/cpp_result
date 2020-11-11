#include "respp/result.hpp"

#include <gtest/gtest.h>

namespace result_tests
{
TEST(Resuls, ManualDeclaration_8_BitWidth)
{
    struct TestDomain {};
    struct TestSubDomain {};

    using Domain = respp::category_t<TestDomain, 2>;
    using SubDomain = respp::category_t<TestSubDomain, 3>;

    using TestResult = respp::result_t<uint8_t, Domain, SubDomain>;

    TestResult r{0b01111101};

    EXPECT_EQ(respp::get_category<Domain>(r).value, 0b01);
    EXPECT_EQ(respp::get_category<SubDomain>(r).value, 0b111);
    EXPECT_EQ(respp::get_code(r), 0b101);
}

MAKE_RESULT_CATEGORY(Domain, 2);
MAKE_RESULT_CATEGORY(SubDomain, 2);
MAKE_RESULT_TYPE(TestResult, uint8_t, Domain, SubDomain);

namespace test_errors
{
namespace drivers
{
constexpr auto Drivers = Domain{0};
constexpr auto Ethernet = SubDomain{1};

constexpr uint8_t linkErrorCode = 3;

constexpr auto Ok = TestResult::success;
constexpr auto ethLinkError
    = TestResult::make(Drivers, Ethernet, linkErrorCode);
}  // namespace drivers

namespace networking
{
constexpr auto Networking = Domain{1};
constexpr auto TcpIpStack = SubDomain{1};

constexpr uint8_t connectionAbortedErrorCode = 3;

constexpr auto Ok = TestResult::success;
constexpr auto connectionAbortedError
    = TestResult::make(Networking, TcpIpStack, connectionAbortedErrorCode);
}  // namespace networking

namespace infrastructure
{
constexpr auto Infrastructure = Domain{2};
constexpr auto Remote = SubDomain{1};

constexpr uint8_t messageSendingErrorCode = 1;
constexpr uint8_t messageReceivingErrorCode = 2;

constexpr auto Ok = TestResult::success;

constexpr auto messageSendingError
    = TestResult::make(Infrastructure, Remote, messageSendingErrorCode);

}  // namespace infrastructure

namespace application
{
constexpr auto Application = Domain{3};
constexpr auto Server = SubDomain{1};
constexpr auto Client = SubDomain{2};

constexpr uint8_t rpcErrorCode = 1;
constexpr uint8_t backendAccessErrorCode = 2;

constexpr auto Ok = TestResult::success;

constexpr auto rpcClientError
    = TestResult::make(Application, Client, rpcErrorCode);
constexpr auto backendAccessError
    = TestResult::make(Application, Client, backendAccessErrorCode);

}  // namespace application
}  // namespace test_errors

using aggregate_result = respp::aggregate_result_t<uint32_t, TestResult>;

TEST(AggregateError_4x8bit, Intialized_with_default_value)
{
    aggregate_result e;

    EXPECT_TRUE(respp::is_success(e));
    
    aggregate_result::error_iterator_t it(e), end;
    ASSERT_EQ(it, end);
}

TEST(AggregateError_4x8bit, Intialized_with_one_error)
{
    aggregate_result e(test_errors::drivers::ethLinkError);

    EXPECT_EQ(
        respp::get_category<Domain>(e[0]), test_errors::drivers::Drivers);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[0]), test_errors::drivers::Ethernet);
    EXPECT_EQ(respp::get_code(e[0]), test_errors::drivers::linkErrorCode);

    EXPECT_EQ(respp::get_category<Domain>(e[1]), 0);
    EXPECT_EQ(respp::get_category<SubDomain>(e[1]), 0);
    EXPECT_EQ(respp::get_code(e[1]), 0);

    EXPECT_EQ(respp::get_category<Domain>(e[2]), 0);
    EXPECT_EQ(respp::get_category<SubDomain>(e[2]), 0);
    EXPECT_EQ(respp::get_code(e[2]), 0);

    EXPECT_EQ(respp::get_category<Domain>(e[3]), 0);
    EXPECT_EQ(respp::get_category<SubDomain>(e[3]), 0);
    EXPECT_EQ(respp::get_code(e[3]), 0);
}

TEST(AggregateError_4x8bit, Intialized_with_one_error_appended_with_second)
{
    aggregate_result e(test_errors::drivers::ethLinkError);
    e.append(test_errors::networking::connectionAbortedError);

    EXPECT_EQ(
        respp::get_category<Domain>(e[0]), test_errors::drivers::Drivers);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[0]), test_errors::drivers::Ethernet);
    EXPECT_EQ(respp::get_code(e[0]), test_errors::drivers::linkErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[1]),
        test_errors::networking::Networking);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[1]),
        test_errors::networking::TcpIpStack);
    EXPECT_EQ(
        respp::get_code(e[1]),
        test_errors::networking::connectionAbortedErrorCode);

    EXPECT_EQ(respp::get_category<Domain>(e[2]), 0);
    EXPECT_EQ(respp::get_category<SubDomain>(e[2]), 0);
    EXPECT_EQ(respp::get_code(e[2]), 0);

    EXPECT_EQ(respp::get_category<Domain>(e[3]), 0);
    EXPECT_EQ(respp::get_category<SubDomain>(e[3]), 0);
    EXPECT_EQ(respp::get_code(e[3]), 0);
}

TEST(
    AggregateError_4x8bit,
    Default_Initialized_and_four_errors_appended_via_shift_operator)
{
    namespace te = test_errors;
    aggregate_result e;

    e << te::drivers::ethLinkError << te::networking::connectionAbortedError
      << te::infrastructure::messageSendingError
      << te::application::rpcClientError;

    EXPECT_EQ(
        respp::get_category<Domain>(e[0]), test_errors::drivers::Drivers);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[0]), test_errors::drivers::Ethernet);
    EXPECT_EQ(respp::get_code(e[0]), test_errors::drivers::linkErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[1]),
        test_errors::networking::Networking);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[1]),
        test_errors::networking::TcpIpStack);
    EXPECT_EQ(
        respp::get_code(e[1]),
        test_errors::networking::connectionAbortedErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[2]),
        test_errors::infrastructure::Infrastructure);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[2]),
        test_errors::infrastructure::Remote);
    EXPECT_EQ(
        respp::get_code(e[2]),
        test_errors::infrastructure::messageSendingErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[3]),
        test_errors::application::Application);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[3]),
        test_errors::application::Client);
    EXPECT_EQ(respp::get_code(e[3]), test_errors::application::rpcErrorCode);
}

TEST(
    AggregateError_4x8bit,
    Initialized_with_two_errors_values_can_be_accessed_via_iterator)
{
    aggregate_result e{
        test_errors::drivers::ethLinkError,
        test_errors::networking::connectionAbortedError};
    aggregate_result::error_iterator_t it(e), end;

    ASSERT_NE(it, end);

    EXPECT_EQ(respp::get_category<Domain>(*it), test_errors::drivers::Drivers);
    EXPECT_EQ(
        respp::get_category<SubDomain>(*it), test_errors::drivers::Ethernet);
    EXPECT_EQ(respp::get_code(*it), test_errors::drivers::linkErrorCode);

    ++it;
    ASSERT_NE(it, end);

    EXPECT_EQ(
        respp::get_category<Domain>(*it), test_errors::networking::Networking);
    EXPECT_EQ(
        respp::get_category<SubDomain>(*it),
        test_errors::networking::TcpIpStack);
    EXPECT_EQ(
        respp::get_code(*it),
        test_errors::networking::connectionAbortedErrorCode);

    ++it;
    ASSERT_EQ(it, end);
}

TEST(AggregateError_4x8bit, Initialized_with_four_errors_loop_via_iterator)
{
    constexpr aggregate_result e{
        test_errors::drivers::ethLinkError,
        test_errors::networking::connectionAbortedError,
        test_errors::infrastructure::messageSendingError,
        test_errors::application::rpcClientError};

    constexpr TestResult results[]
        = {test_errors::drivers::ethLinkError,
           test_errors::networking::connectionAbortedError,
           test_errors::infrastructure::messageSendingError,
           test_errors::application::rpcClientError};

    auto i = 0;
    for (const auto it : e.iterate_errors()) {
        EXPECT_EQ(
            respp::get_category<Domain>(it),
            respp::get_category<Domain>(results[i]));
        EXPECT_EQ(
            respp::get_category<SubDomain>(it),
            respp::get_category<SubDomain>(results[i]));
        EXPECT_EQ(respp::get_code(it), respp::get_code(results[i++]));
    }

    EXPECT_EQ(i, 4);
}

using aggregate_result_replace_topmost = respp::aggregate_result_t<
    uint32_t,
    TestResult,
    respp::detail::replace_topmost<uint32_t, TestResult>>;

TEST(
    AggregateError_4x8bit_Replace_Topmost,
    Default_initialized_and_four_errors_appended_via_shift_operator)
{
    namespace te = test_errors;
    aggregate_result_replace_topmost e;

    e << te::drivers::ethLinkError << te::networking::connectionAbortedError
      << te::infrastructure::messageSendingError
      << te::application::rpcClientError;

    e << te::application::backendAccessError;

    EXPECT_EQ(
        respp::get_category<Domain>(e[0]), test_errors::drivers::Drivers);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[0]), test_errors::drivers::Ethernet);
    EXPECT_EQ(respp::get_code(e[0]), test_errors::drivers::linkErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[1]),
        test_errors::networking::Networking);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[1]),
        test_errors::networking::TcpIpStack);
    EXPECT_EQ(
        respp::get_code(e[1]),
        test_errors::networking::connectionAbortedErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[2]),
        test_errors::infrastructure::Infrastructure);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[2]),
        test_errors::infrastructure::Remote);
    EXPECT_EQ(
        respp::get_code(e[2]),
        test_errors::infrastructure::messageSendingErrorCode);

    EXPECT_EQ(
        respp::get_category<Domain>(e[3]),
        test_errors::application::Application);
    EXPECT_EQ(
        respp::get_category<SubDomain>(e[3]),
        test_errors::application::Client);
    EXPECT_EQ(
        respp::get_code(e[3]),
        test_errors::application::backendAccessErrorCode);
}

}  // namespace result_tests
