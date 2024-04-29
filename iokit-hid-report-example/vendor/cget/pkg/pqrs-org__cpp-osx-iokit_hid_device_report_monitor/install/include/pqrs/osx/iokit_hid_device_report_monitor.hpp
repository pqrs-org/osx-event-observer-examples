#pragma once

// pqrs::osx::iokit_hid_device_report_monitor v1.1

// (C) Copyright Takayama Fumihiko 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See https://www.boost.org/LICENSE_1_0.txt)

#include <IOKit/hid/IOHIDDevice.h>
#include <chrono>
#include <nod/nod.hpp>
#include <optional>
#include <pqrs/cf/run_loop_thread.hpp>
#include <pqrs/dispatcher.hpp>
#include <pqrs/osx/iokit_hid_device.hpp>
#include <pqrs/osx/iokit_return.hpp>
#include <pqrs/osx/iokit_types.hpp>

namespace pqrs {
namespace osx {
class iokit_hid_device_report_monitor final : public dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(void)> started;
  nod::signal<void(void)> stopped;
  nod::signal<void(IOHIDReportType type, uint32_t report_id, std::shared_ptr<std::vector<uint8_t>> report_buffer)> report_arrived;
  nod::signal<void(const std::string&, iokit_return)> error_occurred;

  // Methods

  iokit_hid_device_report_monitor(const iokit_hid_device_report_monitor&) = delete;

  // CFRunLoopRun may get stuck in rare cases if cf::run_loop_thread generation is repeated frequently in macOS 13.
  // If such a condition occurs, cf::run_loop_thread detects it and calls abort to avoid it.
  // However, to avoid the problem itself, cf::run_loop_thread should be provided externally instead of having it internally.
  iokit_hid_device_report_monitor(std::weak_ptr<dispatcher::dispatcher> weak_dispatcher,
                                  std::shared_ptr<cf::run_loop_thread> run_loop_thread,
                                  IOHIDDeviceRef device)
      : dispatcher_client(weak_dispatcher),
        run_loop_thread_(run_loop_thread),
        hid_device_(device),
        open_timer_(*this),
        last_open_error_(kIOReturnSuccess) {
    //
    // Resize report_buffer_
    //

    size_t buffer_size = 32; // use this provisional value if we cannot get max input report size from device.
    if (auto size = hid_device_.find_max_input_report_size()) {
      buffer_size = static_cast<size_t>(*size);
    }

    report_buffer_.resize(buffer_size);

    //
    // Schedule device
    //

    auto wait = make_thread_wait();

    run_loop_thread_->enqueue(^{
      if (auto d = hid_device_.get_device()) {
        IOHIDDeviceRegisterRemovalCallback(*d,
                                           static_device_removal_callback,
                                           this);

        IOHIDDeviceScheduleWithRunLoop(*d,
                                       run_loop_thread_->get_run_loop(),
                                       kCFRunLoopCommonModes);
      }

      wait->notify();
    });

    wait->wait_notice();
  }

  virtual ~iokit_hid_device_report_monitor(void) {
    //
    // dispatcher_client
    //

    detach_from_dispatcher();

    //
    // run_loop_thread
    //

    auto wait = make_thread_wait();

    run_loop_thread_->enqueue(^{
      stop({.check_requested_open_options = false});

      if (auto d = hid_device_.get_device()) {
        IOHIDDeviceUnscheduleFromRunLoop(*d,
                                         run_loop_thread_->get_run_loop(),
                                         kCFRunLoopCommonModes);
      }

      wait->notify();
    });

    wait->wait_notice();
  }

  void async_start(IOOptionBits open_options,
                   std::chrono::milliseconds open_timer_interval) {
    {
      std::lock_guard<std::mutex> lock(open_options_mutex_);

      requested_open_options_ = open_options;
    }

    run_loop_thread_->enqueue(^{
      open_timer_.start(
          [this] {
            run_loop_thread_->enqueue(^{
              start();
            });
          },
          open_timer_interval);
    });
  }

  void async_stop(void) {
    {
      std::lock_guard<std::mutex> lock(open_options_mutex_);

      requested_open_options_ = std::nullopt;
    }

    run_loop_thread_->enqueue(^{
      stop({.check_requested_open_options = true});
    });
  }

  bool seized() const {
    std::lock_guard<std::mutex> lock(open_options_mutex_);

    return current_open_options_ != std::nullopt
               ? (*current_open_options_ & kIOHIDOptionsTypeSeizeDevice)
               : false;
  }

private:
  void start(void) {
    bool needs_stop = false;
    IOOptionBits open_options = kIOHIDOptionsTypeNone;

    auto device = hid_device_.get_device();
    if (!device) {
      goto finish;
    }

    //
    // Check requested_open_options_
    //

    {
      std::lock_guard<std::mutex> lock(open_options_mutex_);

      if (requested_open_options_ == std::nullopt ||
          requested_open_options_ == current_open_options_) {
        goto finish;
      }

      if (current_open_options_) {
        needs_stop = true;
      }

      open_options = *requested_open_options_;
    }

    if (needs_stop) {
      stop({.check_requested_open_options = false});
    }

    //
    // Register callback
    //

    IOHIDDeviceRegisterInputReportCallback(*device,
                                           &(report_buffer_[0]),
                                           report_buffer_.size(),
                                           static_input_report_callback,
                                           this);

    //
    // Open the device
    //

    {
      iokit_return r = IOHIDDeviceOpen(*device,
                                       open_options);
      if (!r) {
        if (last_open_error_ != r) {
          last_open_error_ = r;
          enqueue_to_dispatcher([this, r] {
            error_occurred("IOHIDDeviceOpen is failed.", r);
          });
        }

        // Retry
        return;
      }
    }

    {
      std::lock_guard<std::mutex> lock(open_options_mutex_);

      current_open_options_ = requested_open_options_;
    }

    enqueue_to_dispatcher([this] {
      started();
    });

  finish:
    open_timer_.stop();
  }

  struct stop_arguments {
    bool check_requested_open_options;
  };
  void stop(stop_arguments args) {
    // Since `stop()` can be called from within `start()`,
    // we must not stop `open_timer_` in `stop()` in order to preserve the retry when `IOHIDDeviceOpen` error.

    IOOptionBits open_options = kIOHIDOptionsTypeNone;

    auto device = hid_device_.get_device();
    if (!device) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(open_options_mutex_);

      if (current_open_options_ == std::nullopt) {
        return;
      }

      if (args.check_requested_open_options &&
          requested_open_options_ != std::nullopt) {
        return;
      }

      open_options = *current_open_options_;
    }

    //
    // Unregister callback
    //

    IOHIDDeviceRegisterInputReportCallback(*device,
                                           &(report_buffer_[0]),
                                           report_buffer_.size(),
                                           nullptr,
                                           this);

    //
    // Close
    //

    IOHIDDeviceClose(*device,
                     open_options);

    {
      std::lock_guard<std::mutex> lock(open_options_mutex_);

      current_open_options_ = std::nullopt;
    }

    enqueue_to_dispatcher([this] {
      stopped();
    });
  }

  static void static_device_removal_callback(void* context,
                                             IOReturn result,
                                             void* sender) {
    if (result != kIOReturnSuccess) {
      return;
    }

    auto self = static_cast<iokit_hid_device_report_monitor*>(context);
    if (!self) {
      return;
    }

    self->device_removal_callback();
  }

  void device_removal_callback(void) {
    stop({.check_requested_open_options = false});
  }

  static void static_input_report_callback(void* context,
                                           IOReturn result,
                                           void* sender,
                                           IOHIDReportType type,
                                           uint32_t report_id,
                                           uint8_t* report,
                                           CFIndex report_length) {
    auto self = static_cast<iokit_hid_device_report_monitor*>(context);
    if (!self) {
      return;
    }

    self->input_report_callback(result,
                                type,
                                report_id,
                                report,
                                report_length);
  }

  void input_report_callback(IOReturn result,
                             IOHIDReportType type,
                             uint32_t report_id,
                             uint8_t* report,
                             CFIndex report_length) {
    pqrs::osx::iokit_return r = result;
    if (!r) {
      error_occurred("input report callback error", r);
      return;
    }

    auto buffer = std::make_shared<std::vector<uint8_t>>(report_length);
    memcpy(&((*buffer)[0]), report, report_length);

    enqueue_to_dispatcher([this, type, report_id, buffer] {
      report_arrived(type, report_id, buffer);
    });
  }

  std::shared_ptr<cf::run_loop_thread> run_loop_thread_;

  iokit_hid_device hid_device_;
  dispatcher::extra::timer open_timer_;
  std::optional<IOOptionBits> requested_open_options_;
  std::optional<IOOptionBits> current_open_options_;
  mutable std::mutex open_options_mutex_;
  iokit_return last_open_error_;
  std::vector<uint8_t> report_buffer_;
};
} // namespace osx
} // namespace pqrs
