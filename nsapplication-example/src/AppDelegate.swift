import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  @IBOutlet var window: NSWindow!

  func applicationDidFinishLaunching(_: Notification) {
    window.level = NSFloatingWindowLevel
  }

  func applicationWillTerminate(_: Notification) {}

  func applicationShouldTerminateAfterLastWindowClosed(_: NSApplication) -> Bool {
    return true
  }
}
