#import "AppDelegate.h"

@interface AppDelegate ()

@property(weak) IBOutlet NSWindow* window;
@property NSTimer* timer;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  NSDictionary* options = @{(__bridge NSString*)(kAXTrustedCheckOptionPrompt) : @YES};
  if (!AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options)) {
    self.timer = [NSTimer scheduledTimerWithTimeInterval:3.0
                                                 repeats:YES
                                                   block:^(NSTimer* timer) {
                                                     [self relaunchIfProcessTrusted];
                                                   }];
  }

  [self.window setLevel:NSFloatingWindowLevel];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  return YES;
}

- (void)relaunchIfProcessTrusted {
  if (AXIsProcessTrusted()) {
    [NSTask launchedTaskWithLaunchPath:[[NSBundle mainBundle] executablePath] arguments:@[]];
    [NSApp terminate:nil];
  }
}

@end
