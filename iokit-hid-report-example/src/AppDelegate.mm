#import "AppDelegate.h"
#import "IOKitHIDReportExample.h"
#include <pqrs/cf/run_loop_thread.hpp>
#include <pqrs/dispatcher.hpp>
#include <pqrs/osx/autorelease_pool.hpp>

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;
@property(weak) IBOutlet IOKitHIDReportExample* iokitHIDReportExample;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  self.window.level = NSFloatingWindowLevel;

  pqrs::dispatcher::extra::initialize_shared_dispatcher(
      pqrs::osx::autorelease_pool::execute_with_autorelease_pool);
  pqrs::cf::run_loop_thread::extra::initialize_shared_run_loop_thread(
      pqrs::cf::run_loop_thread::failure_policy::abort,
      pqrs::osx::autorelease_pool::execute_with_autorelease_pool);

  [self.iokitHIDReportExample initializeIOKitHIDReportExample];
}

- (void)applicationWillTerminate:(NSNotification*)notification {
  [self.iokitHIDReportExample terminateIOKitHIDReportExample];

  pqrs::cf::run_loop_thread::extra::terminate_shared_run_loop_thread();
  pqrs::dispatcher::extra::terminate_shared_dispatcher();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  return YES;
}

@end
