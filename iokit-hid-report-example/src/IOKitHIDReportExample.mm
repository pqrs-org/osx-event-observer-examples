#import <CoreGraphics/CoreGraphics.h>

#import "IOKitHIDReportExample.h"
#include <iomanip>
#include <pqrs/osx/iokit_hid_device_report_monitor.hpp>
#include <pqrs/osx/iokit_hid_manager.hpp>
#include <pqrs/weakify.h>

@interface IOKitHIDReportExample ()

@property(weak) IBOutlet NSTextField* text;
@property NSMutableArray<NSString*>* eventStrings;
@property NSUInteger counter;
@property std::shared_ptr<pqrs::osx::iokit_hid_manager> hidManager;
@property std::shared_ptr<std::unordered_map<pqrs::osx::iokit_registry_entry_id::value_t, std::shared_ptr<pqrs::osx::iokit_hid_device_report_monitor>>> monitors;

@end

@implementation IOKitHIDReportExample

- (void)initializeIOKitHIDReportExample {
  self.eventStrings = [NSMutableArray new];
  self.monitors = std::make_shared<std::unordered_map<pqrs::osx::iokit_registry_entry_id::value_t, std::shared_ptr<pqrs::osx::iokit_hid_device_report_monitor>>>();

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

      // Headset
      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::consumer,
          pqrs::hid::usage::consumer::consumer_control),

      // Special devices
      pqrs::osx::iokit_hid_manager::make_matching_dictionary(
          pqrs::hid::usage_page::consumer,
          pqrs::hid::usage::consumer::programmable_buttons),
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

      auto m = std::make_shared<pqrs::osx::iokit_hid_device_report_monitor>(pqrs::dispatcher::extra::get_shared_dispatcher(),
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

      m->report_arrived.connect([self, registry_entry_id](auto&& type, auto&& report_id, auto&& report_buffer) {
        std::stringstream hex;
        for (size_t i = 0; i < report_buffer->size(); ++i) {
          if (i > 0) {
            hex << ":";
          }
          hex << std::hex
              << std::setfill('0')
              << std::setw(2)
              << static_cast<int>((*report_buffer)[i]);
        }

        [self updateEventStrings:[NSString stringWithFormat:@"eid:%lld type:%d report_id:%d report_buffer.size():%ld buffer:%s",
                                                            type_safe::get(registry_entry_id),
                                                            type,
                                                            report_id,
                                                            report_buffer->size(),
                                                            hex.str().c_str()]];
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

- (void)terminateIOKitHIDReportExample {
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
