#import "AppDelegate.h"
#import "IOKitHIDValueExample.h"
#include <pqrs/cf/run_loop_thread.hpp>
#include <pqrs/dispatcher.hpp>

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;
@property(weak) IBOutlet IOKitHIDValueExample* iokitHIDValueExample;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  self.window.level = NSFloatingWindowLevel;

  pqrs::dispatcher::extra::initialize_shared_dispatcher();
  pqrs::cf::run_loop_thread::extra::initialize_shared_run_loop_thread();

  [self.iokitHIDValueExample initializeIOKitHIDValueExample];
}

- (void)applicationWillTerminate:(NSNotification*)notification {
  [self.iokitHIDValueExample terminateIOKitHIDValueExample];

  pqrs::cf::run_loop_thread::extra::terminate_shared_run_loop_thread();
  pqrs::dispatcher::extra::terminate_shared_dispatcher();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  return YES;
}

@end
