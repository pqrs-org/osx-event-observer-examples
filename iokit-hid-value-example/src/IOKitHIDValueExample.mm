#import <CoreGraphics/CoreGraphics.h>

#import "IOKitHIDValueExample.h"
#include <pqrs/osx/iokit_hid_manager.hpp>
#include <pqrs/osx/iokit_hid_queue_value_monitor.hpp>
#include <pqrs/osx/iokit_hid_value.hpp>
#include <pqrs/weakify.h>

@interface IOKitHIDValueExample ()

@property(weak) IBOutlet NSTextField* text;
@property NSMutableArray<NSString*>* eventStrings;
@property NSUInteger counter;
@property std::shared_ptr<pqrs::osx::iokit_hid_manager> hidManager;
@property std::shared_ptr<std::unordered_map<pqrs::osx::iokit_registry_entry_id::value_t, std::shared_ptr<pqrs::osx::iokit_hid_queue_value_monitor>>> monitors;

@end

@implementation IOKitHIDValueExample

- (void)initializeIOKitHIDValueExample {
  self.eventStrings = [NSMutableArray new];
  self.monitors = std::make_shared<std::unordered_map<pqrs::osx::iokit_registry_entry_id::value_t, std::shared_ptr<pqrs::osx::iokit_hid_queue_value_monitor>>>();

  std::vector<pqrs::cf::cf_ptr<CFDictionaryRef>> matching_dictionaries{
      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::generic_desktop,
          pqrs::hid::usage::generic_desktop::keyboard),

      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::generic_desktop,
          pqrs::hid::usage::generic_desktop::mouse),

      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::generic_desktop,
          pqrs::hid::usage::generic_desktop::pointer),

      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::generic_desktop,
          pqrs::hid::usage::generic_desktop::joystick),

      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::generic_desktop,
          pqrs::hid::usage::generic_desktop::game_pad),
  };

  self.hidManager = std::make_shared<pqrs::osx::iokit_hid_manager>(pqrs::dispatcher::extra::get_shared_dispatcher(),
                                                                   pqrs::cf::run_loop_thread::extra::get_shared_run_loop_thread(),
                                                                   matching_dictionaries);
  self.hidManager->device_matched.connect([self](auto&& registry_entry_id, auto&& device_ptr) {
    if (device_ptr) {
      std::string device_name;

      {
        auto d = pqrs::osx::iokit_hid_device(*device_ptr);

        [self updateEventStrings:@"device matched"];
        if (auto manufacturer = d.find_string_property(CFSTR(kIOHIDManufacturerKey))) {
          device_name += *manufacturer + " ";
          [self updateEventStrings:[NSString stringWithFormat:@"    manufacturer:%s", manufacturer->c_str()]];
        }
        if (auto product = d.find_string_property(CFSTR(kIOHIDProductKey))) {
          device_name += *product;
          [self updateEventStrings:[NSString stringWithFormat:@"    product:%s", product->c_str()]];
        }
      }

      auto m = std::make_shared<pqrs::osx::iokit_hid_queue_value_monitor>(pqrs::dispatcher::extra::get_shared_dispatcher(),
                                                                          pqrs::cf::run_loop_thread::extra::get_shared_run_loop_thread(),
                                                                          *device_ptr);
      (*self.monitors)[registry_entry_id] = m;

      m->started.connect([self, registry_entry_id, device_name] {
        [self updateEventStrings:[NSString stringWithFormat:@"started: %llu %s",
                                                            type_safe::get(registry_entry_id),
                                                            device_name.c_str()]];
      });

      m->stopped.connect([self, registry_entry_id, device_name] {
        [self updateEventStrings:[NSString stringWithFormat:@"stopped: %llu %s",
                                                            type_safe::get(registry_entry_id),
                                                            device_name.c_str()]];
      });

      m->values_arrived.connect([self, registry_entry_id](auto&& values) {
        for (const auto& value_ptr : *values) {
          pqrs::osx::iokit_hid_value hid_value(*value_ptr);

          if (auto usage_page = hid_value.get_usage_page()) {
            if (auto usage = hid_value.get_usage()) {
              [self updateEventStrings:[NSString stringWithFormat:@"eid:%lld t:%lld value: (UsagePage,Usage):(%d,%d) %ld",
                                                                  type_safe::get(registry_entry_id),
                                                                  type_safe::get(hid_value.get_time_stamp()),
                                                                  type_safe::get(*usage_page),
                                                                  type_safe::get(*usage),
                                                                  static_cast<long>(hid_value.get_integer_value())]];
            }
          }
        }
      });

      m->error_occurred.connect([self, device_name](auto&& message, auto&& iokit_return) {
        [self updateEventStrings:[NSString stringWithFormat:@"error_occurred: %s %s %s",
                                                            message.c_str(),
                                                            iokit_return.to_string().c_str(),
                                                            device_name.c_str()]];
      });

      m->async_start(kIOHIDOptionsTypeNone,
                     std::chrono::milliseconds(3000));
    }
  });

  self.hidManager->device_terminated.connect([self](auto&& registry_entry_id) {
    [self updateEventStrings:[NSString stringWithFormat:@"device terminated: %llu", type_safe::get(registry_entry_id)]];
    self.monitors->erase(registry_entry_id);
  });

  self.hidManager->error_occurred.connect([self](auto&& message, auto&& kern_return) {
    [self updateEventStrings:[NSString stringWithFormat:@"error_occurred: %s", message.c_str()]];
  });

  self.hidManager->async_start();
}

- (void)terminateIOKitHIDValueExample {
  self.monitors->clear();

  self.hidManager = nullptr;
}

- (void)updateEventStrings:(NSString*)string {
  @weakify(self);
  dispatch_async(
      dispatch_get_main_queue(),
      ^{
        @strongify(self);
        if (!self) {
          return;
        }

        self.counter += 1;

        [self.eventStrings addObject:[NSString stringWithFormat:@"%06ld    %@", self.counter, string]];

        while (self.eventStrings.count > 16) {
          [self.eventStrings removeObjectAtIndex:0];
        }

        self.text.stringValue = [self.eventStrings componentsJoinedByString:@"\n"];
      });
}

@end
