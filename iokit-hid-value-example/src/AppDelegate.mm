#import "AppDelegate.h"
#import "IOKitHIDValueExample.h"
#include <pqrs/dispatcher.hpp>

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;
@property(weak) IBOutlet IOKitHIDValueExample* iokitHIDValueExample;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  self.window.level = NSFloatingWindowLevel;

  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  [self.iokitHIDValueExample initializeIOKitHIDValueExample];
}

- (void)applicationWillTerminate:(NSNotification*)notification {
  [self.iokitHIDValueExample terminateIOKitHIDValueExample];

  pqrs::dispatcher::extra::terminate_shared_dispatcher();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  return YES;
}

@end
