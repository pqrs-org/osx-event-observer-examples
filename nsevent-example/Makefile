all:
	mkdir -p build_xcode \
		&& cd build_xcode \
		&& cmake -G Xcode .. \
		&& xcodebuild -configuration Release SYMROOT="$(CURDIR)/build_xcode/build"

clean:
	rm -rf build_xcode

run:
	open build_xcode/build/Release/nsevent-example.app
