import Cocoa

class ExampleView: NSView {
  @IBOutlet private var text: NSTextField!
  private var eventStrings: [String] = []
  private var counter: UInt64 = 0

  override var acceptsFirstResponder: Bool { return true }

  override func keyDown(with event: NSEvent) {
    updateEventStrings(String(format: "keyDown %d", event.keyCode))
  }

  override func keyUp(with event: NSEvent) {
    updateEventStrings(String(format: "keyUp %d", event.keyCode))
  }

  override func flagsChanged(with event: NSEvent) {
    updateEventStrings(String(format: "flagsChanged %d", event.keyCode))
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
