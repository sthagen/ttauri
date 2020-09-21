// Copyright 2019 Pokitec
// All rights reserved.

#include "ButtonWidget.hpp"
#include "../GUI/utils.hpp"
#include "../utils.hpp"
#include <cmath>
#include <typeinfo>

namespace tt {

using namespace std::literals;

ButtonWidget::ButtonWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
{
    [[maybe_unused]] ttlet label_cbid = label.add_callback([this](auto...) {
        requestConstraint = true;
    });
}

ButtonWidget::~ButtonWidget() {}

[[nodiscard]] WidgetUpdateResult ButtonWidget::updateConstraints() noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
        return result;
    }

    labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);

    _size = labelCell->preferredExtent() + Theme::margin2Dx2;

    return WidgetUpdateResult::Self;
}

void ButtonWidget::draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    auto context = drawContext;

    context.cornerShapes = vec{Theme::roundingRadius};
    if (value) {
        context.fillColor = theme->accentColor;
    }

    // Move the border of the button in the middle of a pixel.
    context.transform = drawContext.transform;
    context.drawBoxIncludeBorder(rectangle());

    if (*enabled) {
        context.color = theme->foregroundColor;
    }
    context.transform = drawContext.transform * mat::T{0.0f, 0.0f, 0.001f};
    labelCell->draw(context, rectangle(), Alignment::MiddleCenter, base_line_position(), true);

    Widget::draw(drawContext, displayTimePoint);
}

void ButtonWidget::handleCommand(command command) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (!*enabled) {
        return;
    }

    if (command == command::gui_activate) {
        if (compare_then_assign(value, !value)) {
            window.requestRedraw = true;
        }
    }
    Widget::handleCommand(command);
}

void ButtonWidget::handleMouseEvent(MouseEvent const &event) noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    Widget::handleMouseEvent(event);

    if (*enabled) {
        if (compare_then_assign(pressed, static_cast<bool>(event.down.leftButton))) {
            window.requestRedraw = true;
        }

        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton && rectangle().contains(event.position)) {
            handleCommand(command::gui_activate);
        }
    }
}

HitBox ButtonWidget::hitBoxTest(vec position) const noexcept
{
    tt_assume(mutex.is_locked_by_current_thread());

    if (rectangle().contains(position)) {
        return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

} // namespace tt
