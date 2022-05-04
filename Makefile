all:
	make -C cgeventtap-example
	make -C iokit-hid-value-example
	make -C nsapplication-example
	make -C nsevent-example
	make -C nsview-example

clean:
	make -C cgeventtap-example clean
	make -C iokit-hid-value-example clean
	make -C nsapplication-example clean
	make -C nsevent-example clean
	make -C nsview-example clean

dist: all
	rm -rf osx-event-observer-examples
	mkdir -p osx-event-observer-examples
	rsync -aH \
		cgeventtap-example/build_xcode/build/Release/cgeventtap-example.app \
		osx-event-observer-examples
	rsync -aH \
		iokit-hid-value-example/build_xcode/build/Release/iokit-hid-value-example.app \
		osx-event-observer-examples
	rsync -aH \
		nsapplication-example/build_xcode/build/Release/nsapplication-example.app \
		osx-event-observer-examples
	rsync -aH \
		nsevent-example/build_xcode/build/Release/nsevent-example.app \
		osx-event-observer-examples
	rsync -aH \
		nsview-example/build_xcode/build/Release/nsview-example.app \
		osx-event-observer-examples
	bash ./scripts/codesign.sh osx-event-observer-examples
	hdiutil create -nospotlight osx-event-observer-examples.dmg -srcfolder osx-event-observer-examples -fs 'Journaled HFS+'
	rm -rf osx-event-observer-examples
	mkdir -p dist
	mv osx-event-observer-examples.dmg dist

notarize:
	xcrun notarytool \
		submit dist/osx-event-observer-examples.dmg \
		--keychain-profile "pqrs.org notarization" \
		--wait
	$(MAKE) staple
	say "notarization completed"

staple:
	xcrun stapler staple dist/osx-event-observer-examples.dmg

check-staple:
	xcrun stapler validate dist/osx-event-observer-examples.dmg
