#pragma once

// (C) Copyright Takayama Fumihiko 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <pqrs/hid/extra/nlohmann_json.hpp>
#include <pqrs/json.hpp>
#include <pqrs/osx/iokit_hid_value.hpp>

namespace pqrs {
namespace osx {
// iokit_hid_value

inline void to_json(nlohmann::json& j, const iokit_hid_value& value) {
  j = nlohmann::json::object();

  j["time_stamp"] = type_safe::get(value.get_time_stamp());

  j["integer_value"] = value.get_integer_value();

  if (auto usage_page = value.get_usage_page()) {
    j["usage_page"] = *usage_page;
  }

  if (auto usage = value.get_usage()) {
    j["usage"] = *usage;
  }

  if (auto logical_max = value.get_logical_max()) {
    j["logical_max"] = *logical_max;
  }

  if (auto logical_min = value.get_logical_min()) {
    j["logical_min"] = *logical_min;
  }
}

inline void from_json(const nlohmann::json& j, iokit_hid_value& hid_value) {
  using namespace std::string_literals;

  if (!j.is_object()) {
    json::requires_object(j, "json");
  }

  for (const auto& [key, value] : j.items()) {
    if (key == "time_stamp") {
      json::requires_number(value, "`"s + key + "`");

      hid_value.set_time_stamp(chrono::absolute_time_point(value.get<uint64_t>()));

    } else if (key == "integer_value") {
      json::requires_number(value, "`"s + key + "`");

      hid_value.set_integer_value(value.get<CFIndex>());

    } else if (key == "usage_page") {
      json::requires_number(value, "`"s + key + "`");

      hid_value.set_usage_page(value.get<hid::usage_page::value_t>());

    } else if (key == "usage") {
      json::requires_number(value, "`"s + key + "`");

      hid_value.set_usage(value.get<hid::usage::value_t>());

    } else if (key == "logical_max") {
      json::requires_number(value, "`"s + key + "`");

      hid_value.set_logical_max(value.get<CFIndex>());

    } else if (key == "logical_min") {
      json::requires_number(value, "`"s + key + "`");

      hid_value.set_logical_min(value.get<CFIndex>());

    } else {
      throw json::unmarshal_error("unknown key: `"s + key + "`"s);
    }
  }
}
} // namespace osx
} // namespace pqrs
