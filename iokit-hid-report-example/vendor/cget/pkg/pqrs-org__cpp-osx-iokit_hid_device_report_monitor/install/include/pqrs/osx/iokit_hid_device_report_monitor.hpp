#pragma once

// pqrs::osx::iokit_hid_device_report_monitor v1.0

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
        device_scheduled_(false),
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

    run_loop_thread_->enqueue(^{
      if (auto d = hid_device_.get_device()) {
        IOHIDDeviceRegisterRemovalCallback(*d,
                                           static_device_removal_callback,
                                           this);

        IOHIDDeviceScheduleWithRunLoop(*d,
                                       run_loop_thread_->get_run_loop(),
                                       kCFRunLoopCommonModes);

        device_scheduled_ = true;
      }
    });
  }

  virtual ~iokit_hid_device_report_monitor(void) {
    //
    // dispatcher_client
    //

    detach_from_dispatcher();

    //
    // run_loop_thread
    //

    run_loop_thread_->enqueue(^{
      stop();

      if (auto d = hid_device_.get_device()) {
        // Note:
        // IOHIDDeviceUnscheduleFromRunLoop causes SIGILL if IOHIDDeviceScheduleWithRunLoop is not called before.
        // Thus, we have to check the state by `device_scheduled_`.

        if (device_scheduled_) {
          IOHIDDeviceUnscheduleFromRunLoop(*d,
                                           run_loop_thread_->get_run_loop(),
                                           kCFRunLoopCommonModes);
        }
      }
    });

    //
    // Wait until all tasks are processed
    //

    auto wait = make_thread_wait();
    run_loop_thread_->enqueue(^{
      wait->notify();
    });
    wait->wait_notice();
  }

  void async_start(IOOptionBits open_options,
                   std::chrono::milliseconds open_timer_interval) {
    open_timer_.start(
        [this, open_options] {
          run_loop_thread_->enqueue(^{
            start(open_options);
          });
        },
        open_timer_interval);
  }

  void async_stop(void) {
    run_loop_thread_->enqueue(^{
      stop();
    });
  }

private:
  void start(IOOptionBits open_options) {
    if (auto d = hid_device_.get_device()) {
      if (!open_options_) {
        //
        // Register callback
        //

        IOHIDDeviceRegisterInputReportCallback(*d,
                                               &(report_buffer_[0]),
                                               report_buffer_.size(),
                                               static_input_report_callback,
                                               this);

        //
        // Open
        //

        iokit_return r = IOHIDDeviceOpen(*d,
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

        open_options_ = open_options;

        enqueue_to_dispatcher([this] {
          started();
        });
      }
    }

    open_timer_.stop();
  }

  void stop(void) {
    if (auto d = hid_device_.get_device()) {
      if (open_options_) {
        //
        // Unregister callback
        //

        IOHIDDeviceRegisterInputReportCallback(*d,
                                               &(report_buffer_[0]),
                                               report_buffer_.size(),
                                               nullptr,
                                               this);

        //
        // Close
        //

        IOHIDDeviceClose(*d,
                         *open_options_);

        open_options_ = std::nullopt;

        enqueue_to_dispatcher([this] {
          stopped();
        });
      }
    }

    open_timer_.stop();
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

    self->run_loop_thread_->enqueue(^{
      self->device_removal_callback();
    });
  }

  void device_removal_callback(void) {
    stop();
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
  bool device_scheduled_;
  dispatcher::extra::timer open_timer_;
  std::optional<IOOptionBits> open_options_;
  iokit_return last_open_error_;
  std::vector<uint8_t> report_buffer_;
};
} // namespace osx
} // namespace pqrs
