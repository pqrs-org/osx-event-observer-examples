import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  @IBOutlet var window: NSWindow!

  func applicationDidFinishLaunching(_: Notification) {
    window.level = NSWindow.Level.floating
  }

  func applicationWillTerminate(_: Notification) {}

  func applicationShouldTerminateAfterLastWindowClosed(_: NSApplication) -> Bool {
    return true
  }
}
