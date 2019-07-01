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
	rm -rf dist
	mkdir -p dist
	rsync -a cgeventtap-example/build_xcode/build/Release/cgeventtap-example.app dist
	rsync -a iokit-hid-value-example/build_xcode/build/Release/iokit-hid-value-example.app dist
	rsync -a nsapplication-example/build_xcode/build/Release/nsapplication-example.app dist
	rsync -a nsevent-example/build_xcode/build/Release/nsevent-example.app dist
	rsync -a nsview-example/build_xcode/build/Release/nsview-example.app dist
	bash ./scripts/codesign.sh dist
