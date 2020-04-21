#pragma once

// (C) Copyright Takayama Fumihiko 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <pqrs/json.hpp>
#include <pqrs/osx/iokit_types.hpp>

namespace pqrs {
namespace osx {
namespace iokit_hid_location_id {
inline void to_json(nlohmann::json& j, const value_t& value) {
  j = type_safe::get(value);
}

inline void from_json(const nlohmann::json& j, value_t& value) {
  if (!j.is_number()) {
    using namespace std::string_literals;
    throw json::unmarshal_error("json must be number, but is `"s + j.dump() + "`"s);
  }

  value = value_t(j.get<type_safe::underlying_type<value_t>>());
}
} // namespace iokit_hid_location_id

namespace iokit_keyboard_type {
inline void to_json(nlohmann::json& j, const value_t& value) {
  j = type_safe::get(value);
}

inline void from_json(const nlohmann::json& j, value_t& value) {
  if (!j.is_number()) {
    using namespace std::string_literals;
    throw json::unmarshal_error("json must be number, but is `"s + j.dump() + "`"s);
  }

  value = value_t(j.get<type_safe::underlying_type<value_t>>());
}
} // namespace iokit_keyboard_type

namespace iokit_registry_entry_id {
inline void to_json(nlohmann::json& j, const value_t& value) {
  j = type_safe::get(value);
}

inline void from_json(const nlohmann::json& j, value_t& value) {
  if (!j.is_number()) {
    using namespace std::string_literals;
    throw json::unmarshal_error("json must be number, but is `"s + j.dump() + "`"s);
  }

  value = value_t(j.get<type_safe::underlying_type<value_t>>());
}
} // namespace iokit_registry_entry_id
} // namespace osx
} // namespace pqrs
