import Cocoa

@objc(ExampleApplication)
class ExampleApplication: NSApplication {
  @IBOutlet private var text: NSTextField!
  private var eventStrings: [String] = []
  private var counter: UInt64 = 0

  override func sendEvent(_ event: NSEvent) {
    switch event.type {
    case .keyDown:
      updateEventStrings(String(format: "keyDown %d", event.keyCode))
    case .keyUp:
      updateEventStrings(String(format: "keyUp %d", event.keyCode))
    case .flagsChanged:
      updateEventStrings(String(format: "flagsChanged %d", event.keyCode))
    default:
      break
    }

    super.sendEvent(event)
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
