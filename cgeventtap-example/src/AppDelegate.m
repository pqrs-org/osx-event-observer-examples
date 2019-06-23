#import "AppDelegate.h"

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  NSDictionary* options = @{(__bridge NSString*)(kAXTrustedCheckOptionPrompt) : @YES};
  AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);

  [self.window setLevel:NSFloatingWindowLevel];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  return YES;
}

@end
