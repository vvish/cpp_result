#pragma once

#include <climits>
#include <type_traits>
#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace respp
{
template <typename Iterator>
class iterator_pair {
public:
    iterator_pair(Iterator first, Iterator last) : m_begin(first), m_end(last)
    {}
    Iterator begin() const
    {
        return m_begin;
    }
    Iterator end() const
    {
        return m_end;
    }

private:
    Iterator m_begin;
    Iterator m_end;
};

template <typename Iterator>
iterator_pair<Iterator> make_iterator_pair(Iterator f, Iterator l)
{
    return iterator_pair<Iterator>(f, l);
}

namespace detail
{
constexpr uint8_t bits_in_byte = CHAR_BIT;

template <typename T>
constexpr uint8_t sizeof_in_bits_v = sizeof(T) * bits_in_byte;

template <typename T>
constexpr T generate_mask(uint8_t const offset, uint8_t const length)
{
    // prefix  length  offset
    // 1111    00      11
    const auto prefix_length = sizeof_in_bits_v<T> - length - offset;
    T mask{};
    for (auto i = 0; i < prefix_length; ++i)
        mask = (mask << 1) + 1;

    mask <<= length;

    for (auto i = 0; i < offset; ++i)
        mask = (mask << 1) + 1;

    return mask;
}

template <typename T, uint8_t offset, uint8_t length>
constexpr T mask = generate_mask<T>(offset, length);

template <typename C1>
constexpr uint8_t sum_widths()
{
    return C1::bit_width;
}

template <typename C1, typename C2, typename... Cs>
constexpr uint8_t sum_widths()
{
    return C1::bit_width + sum_widths<C2, Cs...>();
}

template <typename Cat, typename C, typename... Cs>
struct count_bits_before {
    static constexpr int value
        = std::is_same_v<Cat, C>
              ? 0
              : (count_bits_before<Cat, Cs...>::value >= 0)
                    ? C::bit_width + count_bits_before<Cat, Cs...>::value
                    : -1;
};

template <typename Cat, typename C>
struct count_bits_before<Cat, C> {
    static constexpr int value = std::is_same_v<Cat, C> ? 0 : -1;
};

template <typename Ut, uint8_t Offset, typename C>
constexpr Ut place_category(Ut container, C category)
{
    constexpr auto offset_from_the_lsb
        = sizeof_in_bits_v<Ut> - Offset - C::bit_width;

    constexpr auto mask = ~detail::mask<Ut, offset_from_the_lsb, C::bit_width>;
    return container | (mask & (category.value << offset_from_the_lsb));
};

template <typename Ut, uint8_t Offset, typename C1, typename C2, typename... Cs>
constexpr Ut place_category(Ut container, C1 c1, C2 c2, Cs... cs)
{
    constexpr auto offset_from_the_lsb
        = sizeof_in_bits_v<Ut> - Offset - C1::bit_width;
    constexpr auto mask = ~detail::mask<Ut, offset_from_the_lsb, C1::bit_width>;
    return place_category<Ut, Offset + C1::bit_width, C2, Cs...>(
        container | (mask & (c1.value << offset_from_the_lsb)), c2, cs...);
};

}  // namespace detail

template <typename Token, uint8_t BitWidth>
struct category_t {
    static constexpr uint8_t bit_width = BitWidth;
    using underlaying_type = uint32_t;
    underlaying_type value;  // value should be of minimal width
};

template <typename Token, uint8_t BitWidth>
constexpr bool operator==(
    category_t<Token, BitWidth> const &lhs, category_t<Token, BitWidth> const &rhs)
{
    return lhs.value == rhs.value;
}

template <typename Token, uint8_t BitWidth>
constexpr bool operator==(
    category_t<Token, BitWidth> const &lhs,
    typename category_t<Token, BitWidth>::underlaying_type rhs)
{
    return lhs.value == rhs;
}

template <typename Ut, typename... Cs>
struct result_t {
    using underlaying_type = Ut;
    static constexpr uint8_t bits_occupied_by_categories
        = detail::sum_widths<Cs...>();

    static_assert(
        bits_occupied_by_categories <= detail::sizeof_in_bits_v<Ut>,
        "The underlying type is too small to contain category");

    static constexpr result_t success{0};

    underlaying_type result;

    static constexpr result_t make(Cs const &... categories, Ut code)
    {
        auto const r
            = detail::place_category<underlaying_type, 0>({}, categories...);

        constexpr auto bits_remaining_for_code
            = detail::sizeof_in_bits_v<
                  underlaying_type> - bits_occupied_by_categories;
        constexpr auto mask
            = ~detail::mask<underlaying_type, 0, bits_remaining_for_code>;
        return result_t{static_cast<underlaying_type>(r | (code & mask))};
    }

    friend constexpr bool operator==(result_t const &lhs, result_t const &rhs)
    {
        return lhs.result == rhs.result;
    }
};

template <typename Result>
class error_iterator_t;

namespace detail
{
template <typename Ut, typename Result>
struct place_while_space_is_available {
    static constexpr void place_result(Ut &container, Result const &r)
    {
        using result_underlaying_type = typename Result::underlaying_type;
        constexpr uint8_t capacity
            = sizeof(Ut) / sizeof(result_underlaying_type);
        for (auto shift_value = 0; shift_value < capacity; ++shift_value) {
            auto const shift_in_bits
                = detail::sizeof_in_bits_v<
                      result_underlaying_type> * shift_value;
            auto const slot_value = static_cast<result_underlaying_type>(
                container >> shift_in_bits);
            if (!slot_value) {
                container |= (r.result << shift_in_bits);
                break;
            }
        }
    }
};

template <typename Ut, typename Result>
struct replace_topmost {
    static constexpr void place_result(Ut &container, Result const &r)
    {
        using result_underlaying_type = typename Result::underlaying_type;
        constexpr uint8_t capacity
            = sizeof(Ut) / sizeof(result_underlaying_type);
        auto shift_value = 0;
        for (; shift_value < capacity - 1; ++shift_value) {
            auto const shift_in_bits
                = detail::sizeof_in_bits_v<
                      result_underlaying_type> * shift_value;
            auto const slot_value = static_cast<result_underlaying_type>(
                container >> shift_in_bits);
            if (!slot_value) {
                break;
            }
        }
        auto const shift_in_bits
            = detail::sizeof_in_bits_v<
                  typename Result::underlaying_type> * shift_value;

        container &= detail::generate_mask<Ut>(
            shift_in_bits, detail::sizeof_in_bits_v<result_underlaying_type>);
        container |= (r.result << shift_in_bits);
    }
};

}  // namespace detail

template <
    typename Ut,
    typename Result,
    typename PlacementStrategy
    = detail::place_while_space_is_available<Ut, Result>>
struct aggregate_result_t {
    using underlaying_type = Ut;
    using result = Result;
    using placement_strategy = PlacementStrategy;

    static constexpr uint8_t capacity
        = sizeof(Ut) / sizeof(typename result::underlaying_type);
    static_assert(
        capacity >= 2,
        "The aggregate result should have space for at least two errors");

    underlaying_type container;

    class error_iterator_t {
    public:
        using aggregate_result = aggregate_result_t;
        using single_result = typename aggregate_result::result;
        using underlaying_type = typename aggregate_result::underlaying_type;
        static constexpr uint8_t capacity = aggregate_result::capacity;

        error_iterator_t operator++()
        {
            m_container >>= detail::sizeof_in_bits_v<
                typename single_result::underlaying_type>;
            return error_iterator_t(*this);
        }

        error_iterator_t operator++(int)
        {
            auto const previous_iterator(*this);

            m_container >>= sizeof(single_result::underlaying_type) * 8;
            return previous_iterator;
        }

        const single_result operator*() const
        {
            return single_result{
                static_cast<typename single_result::underlaying_type>(
                    m_container)};
        }

        error_iterator_t() : m_container(0)
        {}

        error_iterator_t(aggregate_result_t const &result)
            : m_container(result.container)
        {}

        error_iterator_t(error_iterator_t const &i) = default;
        error_iterator_t(error_iterator_t &&i) = default;

        error_iterator_t &operator=(error_iterator_t const &i) = default;
        error_iterator_t &operator=(error_iterator_t &&i) = default;

        friend constexpr bool operator==(
            error_iterator_t const &lhs, error_iterator_t const &rhs)
        {
            return lhs.m_container == rhs.m_container;
        }

        friend constexpr bool operator!=(
            error_iterator_t const &lhs, error_iterator_t const &rhs)
        {
            return lhs.m_container != rhs.m_container;
        }

    private:
        underlaying_type m_container;
    };

    constexpr result operator[](size_t const index) const
    {
        const auto shift_value
            = index
              * detail::sizeof_in_bits_v<typename result::underlaying_type>;
        return result{static_cast<typename result::underlaying_type>(
            container >> shift_value)};
    }

    constexpr aggregate_result_t() : container{}
    {}

    constexpr aggregate_result_t(std::initializer_list<result> results)
        : container{}
    {
        for (auto const &r : results) {
            append(r);
        }
    }

    constexpr aggregate_result_t(result const &result)
        : container(result.result)
    {}

    constexpr void append(result const &result)
    {
        placement_strategy::place_result(container, result);
    }

    iterator_pair<error_iterator_t> iterate_errors() const
    {
        return make_iterator_pair(error_iterator_t(*this), error_iterator_t{});
    }

    friend aggregate_result_t &operator<<(
        aggregate_result_t &r, result const &result)
    {
        r.append(result);
        return r;
    }
};

template <typename CatToFind, typename Ut, typename... Cs>
constexpr CatToFind get_category(result_t<Ut, Cs...> result)
{
    constexpr auto bits_offset
        = detail::count_bits_before<CatToFind, Cs...>::value;
    static_assert(bits_offset >= 0, "The category is not found");

    constexpr auto offset_from_the_lsb
        = detail::sizeof_in_bits_v<Ut> - bits_offset - CatToFind::bit_width;
    return CatToFind{static_cast<Ut>(
        (result.result
         & ~detail::mask<Ut, offset_from_the_lsb, CatToFind::bit_width>)
        >> offset_from_the_lsb)};
}

template <typename Ut, typename... Cs>
constexpr Ut get_code(result_t<Ut, Cs...> result)
{
    constexpr auto bits_from_lsb
        = detail::sizeof_in_bits_v<Ut> - detail::sum_widths<Cs...>();
    return (result.result & ~detail::mask<Ut, 0, bits_from_lsb>);
}

template <typename Ut, typename... Cs>
constexpr bool is_success(result_t<Ut, Cs...> result)
{
    return result_t<Ut, Cs...>::success == result;
}

}  // namespace respp

#define MAKE_RESULT_CATEGORY(name, w) \
    struct name##TAG {};              \
    using name = ::respp::category_t<name##TAG, w>

#define MAKE_RESULT_TYPE(name, ut, ...) \
    using name = ::respp::result_t<ut, __VA_ARGS__>
