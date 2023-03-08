#pragma once

// (C) Copyright Takayama Fumihiko 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <compare>
#include <functional>
#include <iostream>
#include <type_safe/strong_typedef.hpp>

namespace pqrs {
namespace hid {
namespace product_id {
struct value_t : type_safe::strong_typedef<value_t, uint64_t>,
                 type_safe::strong_typedef_op::equality_comparison<value_t>,
                 type_safe::strong_typedef_op::relational_comparison<value_t> {
  using strong_typedef::strong_typedef;

  constexpr auto operator<=>(const value_t& other) const {
    return type_safe::get(*this) <=> type_safe::get(other);
  }
};

inline std::ostream& operator<<(std::ostream& stream, const value_t& value) {
  return stream << type_safe::get(value);
}
} // namespace product_id
} // namespace hid
} // namespace pqrs

namespace std {
template <>
struct hash<pqrs::hid::product_id::value_t> : type_safe::hashable<pqrs::hid::product_id::value_t> {
};
} // namespace std
