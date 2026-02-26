#import "CGEventTapExample.h"
@import CoreGraphics;

@interface CGEventTapExample ()

@property(weak) IBOutlet NSTextField* text;
@property NSMutableArray<NSString*>* eventStrings;
@property NSUInteger counter;
@property CFMachPortRef eventTap;

- (CGEventRef)callback:(CGEventTapProxy)proxy
                  type:(CGEventType)type
                 event:(CGEventRef)event;

@end

static CGEventRef callback(CGEventTapProxy proxy,
                           CGEventType type,
                           CGEventRef event,
                           void* refcon) {
  CGEventTapExample* p = (__bridge CGEventTapExample*)(refcon);

  if (p) {
    return [p callback:proxy
                  type:type
                 event:event];
  }
  return NULL;
}

@implementation CGEventTapExample

- (void)awakeFromNib {
  [super awakeFromNib];

  self.eventStrings = [NSMutableArray new];

#if 1
  CGEventTapLocation location = kCGHIDEventTap;
#else
  CGEventTapLocation location = kCGSessionEventTap;
#endif

  CGEventMask mask = CGEventMaskBit(kCGEventKeyDown) |
                     CGEventMaskBit(kCGEventKeyUp) |
                     CGEventMaskBit(kCGEventFlagsChanged);

  self.eventTap = CGEventTapCreate(location,
                                   kCGTailAppendEventTap,
                                   kCGEventTapOptionListenOnly,
                                   mask,
                                   callback,
                                   (__bridge void*)(self));

  CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, self.eventTap, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
  CFRelease(source);

  CGEventTapEnable(self.eventTap, true);
}

- (void)dealloc {
  if (self.eventTap) {
    CFRelease(self.eventTap);
  }
}

- (CGEventRef)callback:(CGEventTapProxy)proxy
                  type:(CGEventType)type
                 event:(CGEventRef)event {
  switch (type) {
    case kCGEventTapDisabledByTimeout:
      CGEventTapEnable(self.eventTap, true);
      break;

    case kCGEventKeyDown:
      [self updateEventStrings:[NSString stringWithFormat:@"keyDown %lld pid:%lld",
                                                          CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode),
                                                          CGEventGetIntegerValueField(event, kCGEventSourceUnixProcessID)]];
      break;
    case kCGEventKeyUp:
      [self updateEventStrings:[NSString stringWithFormat:@"keyUp %lld pid:%lld",
                                                          CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode),
                                                          CGEventGetIntegerValueField(event, kCGEventSourceUnixProcessID)]];
      break;

    case kCGEventFlagsChanged:
      [self updateEventStrings:[NSString stringWithFormat:@"flagsChanged 0x%llx pid:%lld",
                                                          CGEventGetFlags(event),
                                                          CGEventGetIntegerValueField(event, kCGEventSourceUnixProcessID)]];
      break;

    default:
      break;
  }

  return event;
}

- (void)updateEventStrings:(NSString*)string {
  self.counter += 1;

  [self.eventStrings addObject:[NSString stringWithFormat:@"%06ld    %@", self.counter, string]];

  while (self.eventStrings.count > 16) {
    [self.eventStrings removeObjectAtIndex:0];
  }

  self.text.stringValue = [self.eventStrings componentsJoinedByString:@"\n"];
}

@end
