#pragma once

// pqrs::osx::iokit_hid_value v4.0

// (C) Copyright Takayama Fumihiko 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <IOKit/hid/IOHIDValue.h>
#include <optional>
#include <pqrs/hid.hpp>
#include <pqrs/osx/chrono.hpp>
#include <pqrs/osx/iokit_hid_element.hpp>

namespace pqrs {
namespace osx {
class iokit_hid_value final {
public:
  iokit_hid_value(void) : time_stamp_(chrono::absolute_time_point(0)),
                          integer_value_(0) {
  }

  iokit_hid_value(chrono::absolute_time_point time_stamp,
                  CFIndex integer_value,
                  std::optional<hid::usage_page::value_t> usage_page,
                  std::optional<hid::usage::value_t> usage,
                  std::optional<CFIndex> logical_max,
                  std::optional<CFIndex> logical_min) : time_stamp_(time_stamp),
                                                        integer_value_(integer_value),
                                                        usage_page_(usage_page),
                                                        usage_(usage),
                                                        logical_max_(logical_max),
                                                        logical_min_(logical_min) {
  }

  iokit_hid_value(IOHIDValueRef value) : iokit_hid_value() {
    if (value) {
      time_stamp_ = chrono::absolute_time_point(IOHIDValueGetTimeStamp(value));
      integer_value_ = IOHIDValueGetIntegerValue(value);

      auto e = iokit_hid_element(IOHIDValueGetElement(value));
      usage_page_ = e.get_usage_page();
      usage_ = e.get_usage();
      logical_max_ = e.get_logical_max();
      logical_min_ = e.get_logical_min();
    }
  }

  chrono::absolute_time_point get_time_stamp(void) const {
    return time_stamp_;
  }

  iokit_hid_value& set_time_stamp(chrono::absolute_time_point value) {
    time_stamp_ = value;
    return *this;
  }

  CFIndex get_integer_value(void) const {
    return integer_value_;
  }

  iokit_hid_value& set_integer_value(CFIndex value) {
    integer_value_ = value;
    return *this;
  }

  std::optional<hid::usage_page::value_t> get_usage_page(void) const {
    return usage_page_;
  }

  iokit_hid_value& set_usage_page(const std::optional<hid::usage_page::value_t>& value) {
    usage_page_ = value;
    return *this;
  }

  std::optional<hid::usage::value_t> get_usage(void) const {
    return usage_;
  }

  iokit_hid_value& set_usage(const std::optional<hid::usage::value_t>& value) {
    usage_ = value;
    return *this;
  }

  std::optional<CFIndex> get_logical_max(void) const {
    return logical_max_;
  }

  iokit_hid_value& set_logical_max(CFIndex value) {
    logical_max_ = value;
    return *this;
  }

  std::optional<CFIndex> get_logical_min(void) const {
    return logical_min_;
  }

  iokit_hid_value& set_logical_min(CFIndex value) {
    logical_min_ = value;
    return *this;
  }

  bool conforms_to(hid::usage_page::value_t usage_page,
                   hid::usage::value_t usage) const {
    return usage_page_ == usage_page &&
           usage_ == usage;
  }

  bool operator==(const iokit_hid_value& other) const {
    return time_stamp_ == other.time_stamp_ &&
           integer_value_ == other.integer_value_ &&
           usage_page_ == other.usage_page_ &&
           usage_ == other.usage_ &&
           logical_max_ == other.logical_max_ &&
           logical_min_ == other.logical_min_;
  }

  bool operator!=(const iokit_hid_value& other) const {
    return !(*this == other);
  }

private:
  chrono::absolute_time_point time_stamp_;
  CFIndex integer_value_;
  std::optional<hid::usage_page::value_t> usage_page_;
  std::optional<hid::usage::value_t> usage_;
  std::optional<CFIndex> logical_max_;
  std::optional<CFIndex> logical_min_;
};
} // namespace osx
} // namespace pqrs
