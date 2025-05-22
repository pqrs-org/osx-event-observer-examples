all:
	mkdir -p build_xcode \
		&& cd build_xcode \
		&& cmake -G Xcode .. \
		&& xcodebuild -configuration Release SYMROOT="$(CURDIR)/build_xcode"

clean:
	rm -rf build_xcode

run:
	bash ../scripts/codesign.sh build_xcode/Release
	open build_xcode/Release/iokit-hid-value-example.app
