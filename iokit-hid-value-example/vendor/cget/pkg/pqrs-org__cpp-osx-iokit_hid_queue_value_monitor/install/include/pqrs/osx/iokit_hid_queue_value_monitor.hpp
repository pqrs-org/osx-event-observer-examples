#pragma once

// pqrs::osx::iokit_hid_queue_value_monitor v2.0

// (C) Copyright Takayama Fumihiko 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDQueue.h>
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
class iokit_hid_queue_value_monitor final : public dispatcher::extra::dispatcher_client {
public:
  // Signals (invoked from the dispatcher thread)

  nod::signal<void(void)> started;
  nod::signal<void(void)> stopped;
  nod::signal<void(std::shared_ptr<std::vector<cf::cf_ptr<IOHIDValueRef>>>)> values_arrived;
  nod::signal<void(const std::string&, iokit_return)> error_occurred;

  // Methods

  iokit_hid_queue_value_monitor(const iokit_hid_queue_value_monitor&) = delete;

  // CFRunLoopRun may get stuck in rare cases if cf::run_loop_thread generation is repeated frequently in macOS 13.
  // If such a condition occurs, cf::run_loop_thread detects it and calls abort to avoid it.
  // However, to avoid the problem itself, cf::run_loop_thread should be provided externally instead of having it internally.
  iokit_hid_queue_value_monitor(std::weak_ptr<dispatcher::dispatcher> weak_dispatcher,
                                std::shared_ptr<cf::run_loop_thread> run_loop_thread,
                                IOHIDDeviceRef device)
      : dispatcher_client(weak_dispatcher),
        run_loop_thread_(run_loop_thread),
        hid_device_(device),
        device_scheduled_(false),
        open_timer_(*this),
        last_open_error_(kIOReturnSuccess) {
    // Schedule device

    run_loop_thread_->enqueue(^{
      if (hid_device_.get_device()) {
        IOHIDDeviceRegisterRemovalCallback(*(hid_device_.get_device()),
                                           static_device_removal_callback,
                                           this);

        IOHIDDeviceScheduleWithRunLoop(*(hid_device_.get_device()),
                                       run_loop_thread_->get_run_loop(),
                                       kCFRunLoopCommonModes);

        device_scheduled_ = true;
      }
    });
  }

  virtual ~iokit_hid_queue_value_monitor(void) {
    // dispatcher_client

    detach_from_dispatcher();

    // run_loop_thread

    run_loop_thread_->enqueue(^{
      stop();

      if (hid_device_.get_device()) {
        // Note:
        // IOHIDDeviceUnscheduleFromRunLoop causes SIGILL if IOHIDDeviceScheduleWithRunLoop is not called before.
        // Thus, we have to check the state by `device_scheduled_`.

        if (device_scheduled_) {
          IOHIDDeviceUnscheduleFromRunLoop(*(hid_device_.get_device()),
                                           run_loop_thread_->get_run_loop(),
                                           kCFRunLoopCommonModes);
        }
      }
    });

    // Wait until all tasks are processed

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
    if (hid_device_.get_device()) {
      // Start queue before `IOHIDDeviceOpen` in order to avoid events drop.
      start_queue();

      if (!open_options_) {
        iokit_return r = IOHIDDeviceOpen(*(hid_device_.get_device()),
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
    if (hid_device_.get_device()) {
      stop_queue();

      if (open_options_) {
        IOHIDDeviceClose(*(hid_device_.get_device()),
                         *open_options_);

        open_options_ = std::nullopt;

        enqueue_to_dispatcher([this] {
          stopped();
        });
      }
    }

    open_timer_.stop();
  }

  void start_queue(void) {
    if (!queue_) {
      const CFIndex depth = 1024;
      queue_ = hid_device_.make_queue(depth);

      if (queue_) {
        for (const auto& e : hid_device_.make_elements()) {
          IOHIDQueueAddElement(*queue_, *e);
        }

        IOHIDQueueRegisterValueAvailableCallback(*queue_,
                                                 static_queue_value_available_callback,
                                                 this);

        IOHIDQueueScheduleWithRunLoop(*queue_,
                                      run_loop_thread_->get_run_loop(),
                                      kCFRunLoopCommonModes);

        IOHIDQueueStart(*queue_);
      }
    }
  }

  void stop_queue(void) {
    if (queue_) {
      IOHIDQueueStop(*queue_);

      // IOHIDQueueUnscheduleFromRunLoop might cause SIGSEGV if it is not called in run_loop_thread_.

      IOHIDQueueUnscheduleFromRunLoop(*queue_,
                                      run_loop_thread_->get_run_loop(),
                                      kCFRunLoopCommonModes);

      queue_ = nullptr;
    }
  }

  static void static_device_removal_callback(void* context,
                                             IOReturn result,
                                             void* sender) {
    if (result != kIOReturnSuccess) {
      return;
    }

    auto self = static_cast<iokit_hid_queue_value_monitor*>(context);
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

  static void static_queue_value_available_callback(void* context,
                                                    IOReturn result,
                                                    void* sender) {
    if (result != kIOReturnSuccess) {
      return;
    }

    auto self = static_cast<iokit_hid_queue_value_monitor*>(context);
    if (!self) {
      return;
    }

    self->run_loop_thread_->enqueue(^{
      self->queue_value_available_callback();
    });
  }

  void queue_value_available_callback(void) {
    if (queue_) {
      auto values = std::make_shared<std::vector<cf::cf_ptr<IOHIDValueRef>>>();

      while (auto v = IOHIDQueueCopyNextValueWithTimeout(*queue_, 0.0)) {
        values->emplace_back(v);

        CFRelease(v);
      }

      // macOS Catalina (10.15) call the `ValueAvailableCallback`
      // even if `IOHIDDeviceOpen` is failed. (A bug of macOS)
      // Thus, we should ignore the events when `IOHIDDeviceOpen` is failed.
      // (== open_options_ == std::nullopt)

      if (!open_options_) {
        return;
      }

      enqueue_to_dispatcher([this, values] {
        values_arrived(values);
      });
    }
  }

  std::shared_ptr<cf::run_loop_thread> run_loop_thread_;

  iokit_hid_device hid_device_;
  bool device_scheduled_;
  dispatcher::extra::timer open_timer_;
  std::optional<IOOptionBits> open_options_;
  iokit_return last_open_error_;
  cf::cf_ptr<IOHIDQueueRef> queue_;
};
} // namespace osx
} // namespace pqrs
