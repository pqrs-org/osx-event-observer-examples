[![License](https://img.shields.io/badge/license-Public%20Domain-blue.svg)](https://github.com/tekezo/osx-event-observer-examples/blob/master/LICENSE.md)

# osx-event-observer-examples

- cgeventtap-example
  - `CGEventTapCreate`
  - ![cgeventtap-example](docs/images/cgeventtap-example.png)
- iokit-hid-value-example
  - `IOHIDQueueRegisterValueAvailableCallback`
  - ![iokit-hid-value-example](docs/images/iokit-hid-value-example.png)
- nsapplication-example
  - `[NSApplication sendEvent]`
  - ![cgeventtap-example](docs/images/nsapplication-example.png)
- nsevent-example
  - `[NSEvent addGlobalMonitorForEvents]`
  - (Equivalent to `CGEventTapCreate` but cannot modify received events)
  - ![nsevent-example](docs/images/nsevent-example.png)
- nsview-example
  - `[NSView keyDown]`
  - ![nsview-example](docs/images/nsview-example.png)

---

## System Requirements

- macOS 10.12 or later

---

## Building example apps

### Requirements

- CMake (`brew install cmake`)

### Instructions

Open terminal and execute `make` command.

---

## Note

User approval of Accessibility is required to use cgeventtap-example and nsevent-example.
(User approval of Input Monitoring is also required since macOS 10.15)

![processes](docs/images/accessibility.png)

User approval of Accessibility and Input Monitoring is required
to use `iokit-hid-value-example` since macOS 10.15.

![processes](docs/images/input-monitoring.png)
