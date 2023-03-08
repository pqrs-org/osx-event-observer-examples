#import "AppDelegate.h"
#import "IOKitHIDReportExample.h"
#include <pqrs/cf/run_loop_thread.hpp>
#include <pqrs/dispatcher.hpp>

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;
@property(weak) IBOutlet IOKitHIDReportExample* iokitHIDReportExample;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  self.window.level = NSFloatingWindowLevel;

  pqrs::dispatcher::extra::initialize_shared_dispatcher();
  pqrs::cf::run_loop_thread::extra::initialize_shared_run_loop_thread();

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
