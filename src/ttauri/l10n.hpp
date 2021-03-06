// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text/language.hpp"
#include "text/translation.hpp"
#include "forward_value.hpp"
#include "cast.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

namespace tt {
namespace detail {
class l10n_args_base {
public:
    virtual ~l10n_args_base() {}

    /** Format text from the arguments and the given format string.
     * @param fmt The format string.
     */
    [[nodiscard]] virtual std::string format(std::string_view fmt) const noexcept = 0;

    /** Format text from the arguments and the given format string.
     * @patam loc The locale to use when formatting.
     * @param fmt The format string.
     */
    [[nodiscard]] virtual std::string format(std::locale const &loc, std::string_view fmt) const noexcept = 0;

    /** The numeric value of the first numeric argument.
     * @return The numeric value of the first numeric argument or zero.
     */
    [[nodiscard]] virtual long long n() const noexcept = 0;

    /** Make a unique copy of the arguments.
     */
    [[nodiscard]] virtual std::unique_ptr<l10n_args_base> unique_copy() const noexcept = 0;

    [[nodiscard]] virtual bool equal_to(l10n_args_base const &rhs) const noexcept = 0;

    [[nodiscard]] bool friend operator==(l10n_args_base const &lhs, l10n_args_base const &rhs) noexcept
    {
        return lhs.equal_to(rhs);
    }
};

/** Delayed formatting.
 * This class will capture all the arguments so that it may be passed
 * to another thread. Then call the function operator to do the actual formatting.
 */
template<typename... Values>
class l10n_args : public l10n_args_base {
public:
    l10n_args(l10n_args &&) noexcept = default;
    l10n_args(l10n_args const &) noexcept = default;
    l10n_args &operator=(l10n_args &&) noexcept = default;
    l10n_args &operator=(l10n_args const &) noexcept = default;

    /** Construct a l10n arguments.
     *
     * All arguments are passed by forwarding-references so that values can be
     * moved into the storage of the l10n object.
     *
     * Arguments passed by reference will be copied. Arguments passed by std::string_view
     * or std::span will be copied into a std::string or std::vector.
     *
     * Literal strings will not be copied, instead a pointer is taken.
     *
     * @param args The parameters to std::format excluding format string and locale.
     */
    template<typename... Args>
    l10n_args(Args const &...args) noexcept : _values(args...)
    {
    }

    [[nodiscard]] std::unique_ptr<l10n_args_base> unique_copy() const noexcept
    {
        return std::make_unique<l10n_args>(*this);
    }

    [[nodiscard]] virtual bool equal_to(l10n_args_base const &rhs) const noexcept
    {
        if (auto *rhs_ = dynamic_cast<l10n_args const *>(&rhs)) {
            return _values == rhs_->_values;
        } else {
            return false;
        }
    }

    [[nodiscard]] std::string format(std::string_view fmt) const noexcept override
    {
        return std::apply(format_wrapper<Values const &...>, std::tuple_cat(std::tuple{fmt}, _values));
    }

    [[nodiscard]] std::string format(std::locale const &loc, std::string_view fmt) const noexcept override
    {
        return std::apply(format_locale_wrapper<Values const &...>, std::tuple_cat(std::tuple{loc, fmt}, _values));
    }

    template<size_t I>
    [[nodiscard]] long long n_recurse() const noexcept
    {
        if constexpr (I < sizeof...(Values)) {
            if constexpr (std::is_integral_v<decltype(std::get<I>(_values))>) {
                return narrow_cast<long long>(std::get<I>(_values));
            } else {
                return n_recurse<I + 1>();
            }
        } else {
            return 0;
        }
    }

    [[nodiscard]] long long n() const noexcept override
    {
        return n_recurse<0>();
    }

private:
    std::tuple<Values...> _values;

    template<typename... Args>
    static std::string format_wrapper(std::string_view fmt, Args const &...args)
    {
        return std::format(fmt, args...);
    }

    template<typename... Args>
    static std::string format_locale_wrapper(std::locale const &loc, std::string_view fmt, Args const &...args)
    {
        return std::format(loc, fmt, args...);
    }
};

template<typename... Args>
l10n_args(Args &&...) -> l10n_args<forward_value_t<Args>...>;

} // namespace detail

/** A localizable message.
 *
 * The translation and formatting of the message is delayed until displaying
 * it to the user. This allows the user to change the language while the
 * application is running.
 */
class l10n {
public:
    /** Construct an empty message.
     */
    constexpr l10n() noexcept : _msg_id(), _args(nullptr) {}

    l10n(l10n &&) noexcept = default;
    l10n &operator=(l10n &&) noexcept = default;

    l10n(l10n const &other) noexcept : _msg_id(other._msg_id), _args(other._args ? other._args->unique_copy() : nullptr) {}

    l10n &operator=(l10n const &other) noexcept
    {
        _msg_id = other._msg_id;
        _args = other._args ? other._args->unique_copy() : nullptr;
        return *this;
    }

    /** Check if the message is in use.
     */
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return not _msg_id.empty();
    }

    /** Construct a localizable message.
     *
     * It is recommended to use the parentheses form of the constructor so that
     * it will look like a function which is recognized by the `gettext` tool.
     *
     * @param msg_id A English string that is looked up in the translation
     *               database or, when not found, as-is. The msg_id may contain
     *               placeholders using the `std::format` format. Plurality is
     *               based on the first `std::integral` arguments.
     * @param args Arguments passed to `std::format`. The arguments are copied
     *             into the `l10n` object and used when formatting the
     *             translated string.
     */
    template<typename... Args>
    l10n(std::string_view msg_id, Args const &...args) noexcept :
        _msg_id(msg_id), _args(sizeof...(Args) ? std::make_unique<detail::l10n_args<forward_value_t<Args>...>>(args...) : nullptr)
    {
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] std::string
    operator()(std::vector<language *> const &languages = language::preferred_languages()) const noexcept
    {
        if (_args) {
            auto fmt = ::tt::get_translation(_msg_id, _args->n(), languages);
            return _args->format(fmt);
        } else {
            return std::string{::tt::get_translation(_msg_id, 0, languages)};
        }
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param locale The locale to use when formatting the message.
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] std::string
    operator()(std::locale const &loc, std::vector<language *> const &languages = language::preferred_languages()) const noexcept
    {
        if (_args) {
            auto fmt = ::tt::get_translation(_msg_id, _args->n(), languages);
            return _args->format(loc, fmt);
        } else {
            return std::string{::tt::get_translation(_msg_id, 0, languages)};
        }
    }

    /** Compare two localizable messages.
     *
     * @param lhs A localizable message.
     * @param rhs A localizable message.
     * @return True if both messages are equal.
     */
    [[nodiscard]] friend bool operator==(l10n const &lhs, l10n const &rhs) noexcept
    {
        if (lhs._args == rhs._args) {
            return lhs._msg_id == rhs._msg_id;
        } else if (lhs._args and rhs._args) {
            return lhs._msg_id == rhs._msg_id and *lhs._args == *rhs._args;
        } else {
            return false;
        }
    }

private:
    std::string _msg_id;
    std::unique_ptr<detail::l10n_args_base> _args;
};

} // namespace tt
