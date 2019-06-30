#import "AppDelegate.h"
#import "IOKitHIDValueExample.h"
#include <pqrs/dispatcher.hpp>

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;
@property(weak) IBOutlet IOKitHIDValueExample* iokitHIDValueExample;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  pqrs::dispatcher::extra::initialize_shared_dispatcher();

  [self.iokitHIDValueExample initializeIOKitHIDValueExample];

  [self.window setLevel:NSFloatingWindowLevel];
}

- (void)applicationWillTerminate:(NSNotification*)notification {
  [self.iokitHIDValueExample terminateIOKitHIDValueExample];

  pqrs::dispatcher::extra::terminate_shared_dispatcher();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  return YES;
}

@end
