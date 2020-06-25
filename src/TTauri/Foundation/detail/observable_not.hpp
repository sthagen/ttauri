// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable_unary.hpp"

namespace tt::detail {

template<typename OT>
class observable_not final : public observable_unary<bool,OT> {
public:
    observable_not(std::shared_ptr<observable_base<OT>> const &operand) noexcept :
        observable_unary<bool,OT>(operand) {}

    virtual bool load() const noexcept override {
        ttlet lock = std::scoped_lock(observable_unary<bool,OT>::mutex);
        return !this->operand_cache;
    }

    virtual void store(bool const &new_value) noexcept override {
        this->operand->store(static_cast<OT>(!new_value));
    }
};

}
