// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/WindowWidget.hpp"
#include "TTauri/GUI/WindowTrafficLightsWidget.hpp"
#include "TTauri/GUI/WindowToolbarWidget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

using namespace std;

WindowWidget::WindowWidget(Window &window) noexcept :
    Widget(window)
{
    toolbar = &addWidget<WindowToolbarWidget>();
    window.addConstraint(toolbar->box.left == box.left);
    window.addConstraint(toolbar->box.right == box.right);
    window.addConstraint(toolbar->box.top == box.top);

    window.addConstraint(box.left == 0);
    window.addConstraint(box.bottom == 0);
    // A upper bound constraint is needed to allow the suggest(width, limit::max()) and suggest(height, limit::max()) to
    // fallback on a upper bound, otherwise it will select the lower bounds instead.
    window.addConstraint(box.width <= std::numeric_limits<uint16_t>::max());
    window.addConstraint(box.height <= std::numeric_limits<uint16_t>::max());

    backgroundColor = theme->fillColor(nestingLevel());
}

void WindowWidget::draw(DrawContext const &drawContext, cpu_utc_clock::time_point displayTimePoint) noexcept
{
    backgroundColor = theme->fillColor(nestingLevel());
    Widget::draw(drawContext, displayTimePoint);
}

HitBox WindowWidget::hitBoxTest(vec position) noexcept
{
    constexpr float BORDER_WIDTH = 5.0;

    auto r = HitBox{this, elevation};

    if (position.x() <= (box.left.value() + BORDER_WIDTH)) {
        if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
            r.type = HitBox::Type::BottomLeftResizeCorner;
        } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopLeftResizeCorner;
        } else {
            r.type = HitBox::Type::LeftResizeBorder;
        }

    } else if (position.x() >= (box.right.evaluate() - BORDER_WIDTH)) {
        if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
            r.type = HitBox::Type::BottomRightResizeCorner;
        } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
            r.type = HitBox::Type::TopRightResizeCorner;
        } else {
            r.type = HitBox::Type::RightResizeBorder;
        }

    } else if (position.y() <= (box.bottom.value() + BORDER_WIDTH)) {
        r.type = HitBox::Type::BottomResizeBorder;

    } else if (position.y() >= (box.top.evaluate() - BORDER_WIDTH)) {
        r.type = HitBox::Type::TopResizeBorder;
    }

    if (r.type != HitBox::Type::Outside) {
        // Resize corners need to override anything else, so that it is
        // always possible to resize a window.
        return r;
    }

    r = std::max(r, toolbar->hitBoxTest(position));
    for (auto& widget : children) {
        r = std::max(r, widget->hitBoxTest(position));
    }

    return r;
}

}
