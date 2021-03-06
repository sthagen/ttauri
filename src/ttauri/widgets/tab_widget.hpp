// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_widget.hpp"
#include "tab_delegate.hpp"
#include "default_tab_delegate.hpp"

namespace tt {

/** A graphical element that shows only one of a predefined set of mutually
 * exclusive child widgets.
 *
 * A tab widget is generally controlled by a `toolbar_tab_button_widget` or
 * another selection widget.
 *
 * @image html toolbar_tab_button_widget.gif
 *
 * In the following example we create three tabs on the window which observes a
 * `value` controlled by a set of toolbar tab buttons. Each tab is configured
 * with a different value: 0, 1 and 2.
 *
 * @snippet widgets/tab_example.cpp Create three tabs
 *
 * @note A `tab_button` is not directly controlled by a
 *       `toolbar_tab_button_widget`. This is accomplished by sharing a delegate
 *       or a observable between the toolbar tab button and the tab widget.
 */
class tab_widget final : public widget {
public:
    using super = widget;
    using delegate_type = tab_delegate;

    /** Construct a tab widget with a delegate.
     *
     * @param window The window that this widget is shown on.
     * @param parent The owner of this widget.
     * @param delegate The delegate that will control this widget.
     */
    tab_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept;

    /** Construct a tab widget with an observable value.
     *
     * @param window The window that this widget is shown on.
     * @param parent The owner of this widget.
     * @param value The value or observable value to monitor for which child widget
     *              to display.
     */
    template<typename Value>
    tab_widget(gui_window &window, widget *parent, Value &&value) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        tab_widget(window, parent, make_unique_default_tab_delegate(std::forward<Value>(value)))
    {
    }

    /** Make and add a child widget.
     *
     * @pre A widget with the same @a value must not have been added before.
     * @tparam WidgetType The type of the widget to create.
     * @tparam Key The type of the key, must be convertible to `size_t`.
     * @param key The value used as a key to select this newly added widget.
     * @param args The arguments to pass to the constructor of widget to add.
     */
    template<typename WidgetType, typename Key, typename... Args>
    WidgetType &make_widget(Key const &key, Args &&...args) noexcept
    {
        tt_axiom(is_gui_thread());

        if (auto delegate = _delegate.lock()) {
            delegate->add_tab(*this, static_cast<size_t>(key), std::size(_children));
        }
        auto &widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        return widget;
    }

    /// @privatesection
    void init() noexcept override;
    void deinit() noexcept override;
    [[nodiscard]] float margin() const noexcept override;
    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;
    [[nodiscard]] widget const *find_next_widget(
        widget const *current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept override;
    /// @endprivatsectopn
private:
    weak_or_unique_ptr<delegate_type> _delegate;
    typename delegate_type::callback_ptr_type _delegate_callback;

    tab_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept;
    [[nodiscard]] auto find_selected_child() const noexcept;
    [[nodiscard]] auto find_selected_child() noexcept;
    [[nodiscard]] widget const &selected_child() const noexcept;
    void draw_child(draw_context context, hires_utc_clock::time_point displayTimePoint, widget &child) noexcept;
};

} // namespace tt