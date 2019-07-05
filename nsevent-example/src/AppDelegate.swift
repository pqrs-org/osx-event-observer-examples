import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  @IBOutlet var window: NSWindow!
  @IBOutlet private var text: NSTextField!
  private var timer: Timer?
  private var eventStrings: [String] = []
  private var counter: UInt64 = 0

  func applicationDidFinishLaunching(_: Notification) {
    window.level = NSFloatingWindowLevel

    let options: NSDictionary = [kAXTrustedCheckOptionPrompt.takeRetainedValue() as NSString: true]
    if !AXIsProcessTrustedWithOptions(options) {
      timer = Timer.scheduledTimer(
        withTimeInterval: 3.0,
        repeats: true
      ) { _ in self.relaunchIfProcessTrusted() }
    }

    NSEvent.addGlobalMonitorForEvents(
      matching: [NSEventMask.keyDown, NSEventMask.keyUp, NSEventMask.flagsChanged],
      handler: { (event: NSEvent) in
        switch event.type {
        case .keyDown:
          self.updateEventStrings(String(format: "keyDown %d", event.keyCode))
        case .keyUp:
          self.updateEventStrings(String(format: "keyUp %d", event.keyCode))
        case .flagsChanged:
          self.updateEventStrings(String(format: "flagsChanged %d", event.keyCode))
        default:
          break
        }
      }
    )
  }

  func applicationWillTerminate(_: Notification) {}

  func applicationShouldTerminateAfterLastWindowClosed(_: NSApplication) -> Bool {
    return true
  }

  private func relaunchIfProcessTrusted() {
    if AXIsProcessTrusted() {
      let task = Process()
      task.executableURL = URL(fileURLWithPath: Bundle.main.executablePath!)
      try! task.run()
      NSApplication.shared().terminate(self)
    }
  }

  private func updateEventStrings(_ string: String) {
    counter += 1
    eventStrings.append(String(format: "%06d    %@", counter, string))

    while eventStrings.count > 16 {
      eventStrings.remove(at: 0)
    }

    text.stringValue = eventStrings.joined(separator: "\n")
  }
}
