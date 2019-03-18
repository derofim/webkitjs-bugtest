webkit.js
=========

TODO: llvm trap https://groups.google.com/forum/#!msg/emscripten-discuss/SlZevdrVatY/LI-iLSYxTUoJ

A port of WebKit (specifically, webcore+libs) to JavaScript aimed at running in both node.js and browsers. Currently available for contributors; and testers.  However this is not recommended for production or beta environments.

See the [webkit.js Google Group](https://groups.google.com/forum/#!forum/webkitjs) for Discussions & Questions

Goals
----

  - Produce a renderer in pure JavaScript that supports rendering to WebGL/Canvas contexts.
  - Develop frameworks for image/webpage capturing and automated web testing (similar to PhantomJS).
  - Develop a framework for prototyping CSS filters, HTML elements and attributes.
  - Experimental harness for pure JavaScript rendering performance.
  - A next-gen browser built entirely with javascript and utilizing multi-core parallel processing primitives with WebCL (or other draft prototype in the works).
 
Status
----

**Current Issues**

* ~~Fixing canvas size bitblt issues; canvas width and height can become inaccurate due to hi-dpi rendering.~~
* ~~Alpha channels are being flattened for unknown reasons, in addition gradients and animations in css are not rendering.~~
* ~~Inline SVG seg-faults. Most likely due to threading issue.~~
* ~~Certain CSS3 key-frame animations cause either a segfault or jump to the last frame, uncertain but it seems to be an issue with ChromeClient and TextureMapperGL.~~
* Mouse and keyboard (and other) events
* IN-PROGRESS: Resource loader has yet to be implemented, callback for browser (and eventually node)
* (Fixed, see emscripten post, waiting for it to land) ~~Support for WebGL within webworkers via proxy (emscripten is currently working on this)~~
* Enhancing the API.  Adding mouse event callbacks, scrolling, keyboard callbacks, and snapshot to it.  Until there's a stable (well tested) API this will remain as an experiment.


**Supported Features**

The entire HTML5, CSS3 and SVG rendering is supported; the following features are not supported and probably will never be.

* Encrypted Media
* JavaScript Debugger / Inspector (Could be added, although huge penalty in code size -- greater than 5MB)
* Quota's and File System (Browser Only)
* Web Timing (Implemented, but inaccurate due to limitations in JS spec).
* Native OS Widgets (although "webkit" standard widgets, or any css styled widgets, e.g., input fields are supported).

Fonts are rendered via freetype2, fontconfig and cairo for consistent font rendering across all platforms. Native video and audio decoding is supported but not included in the regular build. Eventually i'll post instructions on compiling in native video/audio support.


Building
-----------

**Requirements**

webkit.js can be built on linux, osx, or windows.

* Xcode (OSX only)
* Visual Studio (Windows only)
* [Cygwin](http://dev.chromium.org/developers/how-tos/cygwin) (Windows only)
* Command Line Utilities for Xcode, with `$ xcode-select --install` (Mac OS X only)
* At least 16GB of free space (seriously)
* At least 4GB of RAM

**Building webkit.js step-by-step**

Tested on ubuntu

* Install gcc

* Make sure /usr/bin/python points to /usr/bin/python2.7:
```
file /usr/bin/python
# /usr/bin/python: symbolic link to /usr/bin/python3
# if not:
sudo rm /usr/bin/python
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

* Install emscripten from https://emscripten.org/docs/getting_started/downloads.html
```
#  check em-config
em-config EMSCRIPTEN_ROOT
```

Check env var:
```
ls $EMSCRIPTEN/system
```

* Install ninja-build from https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages#pre-built-ninja-packages
```
sudo -E apt-get -y install ninja-build
```

* Clone the repo to your local drive.
```
git clone https://github.com/trevorlinton/webkit.js webkitjs ; cd webkitjs
git pull 
git lfs pull
gclient sync 
# skip submodule init errors
git submodule update --init --recursive || true
ls deps/libjpeg_turbo
```

* Get depot_tools
```
mkdir install-tools
cd install-tools
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=`pwd`/depot_tools:"$PATH"
which ninja # .../webkit.js/install-tools/depot_tools/ninja
cd ..
```

* Build curl, install and copy .h:

```
cd deps/curl
./buildconf
emconfigure ./configure "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
cd ../..
```

```
# replace 
#  include <sys/poll.h>
# To:
#  include <poll.h>
# see https://github.com/emscripten-core/emscripten/issues/5447
```
```
# 1919  cd deps/curl/
# 1920  mkdir build
# 1921  cd    build
# 1922  cmake .. -GNinja -DCMAKE_USE_MBEDTLS=1 -DCMAKE_USE_OPENSSL=0 -DCURL_ZLIB=1 -DUSE_NGHTTP2=1 -DENABLE_ARES=1
# 1923  ninja
# 1924  ninja install
# 1925  sudo ninja install
# 1926  ldconfig
# 1927  sudo ldconfig
# 1928  cd ../..
# 1929  history 
#
#sudo cp deps/curl/build/lib/curl_config.h /usr/local/lib 
#sudo cp deps/curl/build/include/curl/curlbuild.h /usr/local/include/curl/curlbuild.h 
#
#cp ./deps/curl/build/lib/curl_config.h ./deps/curl/lib/curl_config.h
#cp deps/curl/curlbuild.h ./deps/curl/build/include/curl/curlbuild.h
export PATH=/usr/local/include:$PATH
```

* Run the setup script to grab all of the dependencies and tools.
```
bash ./setup.sh
```

* Run the configuration script to generate build and project files.
```
bash ./config.sh
```

* Then to build; run this (generates the actual js file):
```
bash ./build.sh
```

* OR if you want a debug version run this:
```
bash ./build.sh debug
```

Running in the Browser
--------------
See demo.html and webkit.js in the "bin" folder, runs best in Firefox (Chrome/Safari seem to have large long hiccups and freezes due to garbage collection).

Demo
--------------
http://trevorlinton.github.io/ 

IN DEV!!!
IN DEV!!!
IN DEV!!!
--------------

Contributing
--------------

**Getting around the code**

* `/deps/WebKit/` This is a modified version of Apple's official repo.
* `/deps/WebKit/Source/WebCore` This is where 90% of the work is done.
* `/deps/WebKit/Source/WebCore/bindings/scripts` This is an important folder where WebKit autogenerates bindings
* `/deps/WebKit/Source/WTF/` is a cross-platform library for common tasks such as HashMaps, etc.
* `/deps/WebKit/Source/WTF/PlatformJS.h` these are C++ pre-process settings for PLATFORM(JS)
* `/features.gypi` You can turn various features in WebCore on and off here
* `/common.gypi` You can change various compiler settings, system paths, etc.
* `/webkit.gyp` This contains the build arch and is the first file used when generating project files with gyp.
* `/sources.gypi` This file contains the list of all of the sources to compile.
* `/derived.gypi` This file is auto-generated by make-source-files.sh in the tools directory, it hunts down any changes vs. what's in derived.gypi currently and adds it in.  This is convenient only when you've done a pull from WebKit and need to make sure you've properly added/removed files based on what's happened upstream. 
* `/config.sh` This autogenerates the project files, it also tries to find em++/emcc and all of your tools, if config files, check the file to make sure its finding emscripten and clang.
* `/src` This contains all of the derived sources, bindings, and custom code to link in with WebCore and bind it to canvas, node and javascript contexts. This will also be where the API will be built.
* `/bin` This contains the latest builds for debug and release modes.  Note that the debug builds are compressed for the sake of github.
* `/src/WebView.cpp` This is the main 'essential' class and loading class for everything.
* `/src/Main.cpp` This is a proxy from C++ to C, so we can use the emscripten exported symbols features.
* `/src/Symbols.exports` This contains a list of C functions that are exported to javascript (and thus accessible to the javascript API.
* `/src/webkit.api.js` This is the javascript file thats exposed to javascript programmers.

**It's important to know**

* The code within the WebKit folder is pulled from upstream, be careful not to move any files, remove any files or heavily refactor any source file as it will cause headaches when merging.
* Enabling/disabling settings within the `features.gypi` will have a lot of consequences, most of the disabled features are disabled because there's no possible work around for including the platform specific or network layer code (bindings to forward this to the native browser/nodejs module will need to be built). In addition, if features.gypi is modified that you update featurs.make within the tools folder.
* A good amount of the bindings and code within the WebCore are auto generated from scripts, be careful when you have build errors to make sure you're not modifying a "Derived Sources" file, otherwise you'll find your changes will be just over-written the next time this script runs.
* Do not modify code within `src/WebCoreDerived`, it's autogenerated and will most likely be overwritten if the derived sources needs to rebuild.
* Be careful adding files/changing settings in your native IDE/toolchain, these settings are intially set in `config.sh` and `common.gypi` (and related gypi/gyp files). While not all settings will be overwritten with a new config, there's a chance some of your settings may need to be added to these files.



**Roadmap**

* **DONE** ~~A build toolchain similar to GYP/gconfig. QtWebkit has one already, possibly re-map that.~~
* **DONE** ~~Create "Debug" and "Release" modes that allow for easier debugging. In addition creating anything that helps debug and spot problems easier.~~
* **DONE** ~~Integration of WTF library into WebCore~~
* **Bad Idea -** ~~Start smaller with GYP and only develop one pass layout system from CSS/HTML/DOM code with minimal features and build up.~~
* **Bad Idea -** ~~Take each file one by one in ./webkitjs/debug/ and port?...~~
* **DONE** ~~Dependency and/or symbol graph that rebuilds automatically after a compile (expressed as a HTML doc?) The core reason for this is to visualize dependencies between classes, unresolved symbols still to be developed, and spot key integration points. This can be done by regex over the existing ./webkitjs/debug for symbols and building a D3 graph to show the symbols dependency possibly? Is there already key software that does this? Can emscripten/llvm spit this out?~~
* **DONE** ~~Identify what key import symbols may require significant retooling.~~
* **DONE** ~~Integrate libxml.js (rather than depending on browser pass through decoding to a buffer)~~
* **DONE** ~~Integrate libxslt.js (currently unsupported)~~
* **DONE** ~~Integrate ffmpeg.js ?.... pure JavaScript video support?..... .~~
* **DONE** ~~Integrate libpng.js (rather than depending on browser pass through decoding to a buffer)~~
* **DONE** ~~Integrate libjpeg-turbo.js/libjpeg.js (rather than depending on browser pass through decoding to a buffer)~~
* **DONE** ~~Integrate zlib (rather than depending on browser pass through decoding to a buffer)~~
* **DONE** ~~Integrate both node and web browser environments for testing demo of basic HTML~~
* **DONE** ~~Integrate support for font resource loading and virtual file system for fontconfig~~
* **Bad Idea -** ~~Use embind/cppfilter.js to automatically generate all the WebCore C++ interfaces (derived from WebCore.exp.in) directly into JavaScript, then simply reuse existing webcore demos/examples.~~
* **DONE** ~~Generate a webkit.js API based on the WebCore C++ interfaces exported to JavaScript.~~
* **DONE** ~~Explore best methods for creating demo's and painting within WebKitJS.cpp to the host context.~~
* **DONE** ~~Experiment with emscripten outlining, lto, and optimization techniques to prevent variable/heap/stack limitations and reduce code size.~~
* **DONE** ~~Generate simple JavaScript library to create, use and manage webkit.js rendering.~~
* **DONE** ~~Investigate if SKIA might provide better rendering -- no, skia does not support accelerated back-tile compositing needed by WebKit~~
* **DONE** ~~Support for hidpi rendering on retina displays, support for device scale factors and clean font rendering~~
* **In Progress** Create examples, demos and how-to guides (documentation, etc).
* **DONE** Create hooks into webkit layout tests to ensure functionality.
* **DONE** Fix all layout tests for still-frame and render tree unit tests.
* **DONE** Fix timing animation (CSS3) bug
* Scripts to auto-generate code with Emscripten JavaScript Bindings to enable DOM interactions with JavaScript (e.g., IDL generation, and some other bindings/scripts tasks)
* **In Progress** Removal of "oddity" code (e.g., no mans land code, existing dead code, platform specific code) and optimization of file size and runtime
* **In Progress** Benchmark harnass between different browsers and webkit.js
* Everything else that's actually fun and note worthy (listed above in Goals).

License
----
[BSD License](http://www.webkit.org/coding/bsd-license.html). Use of this source code is governed by a BSD-style license that can be found in the LICENSE files contained in the root of the respective source code.

&copy; True Interactions 2014 (www.trueinteractions.com)

www.twitter.com/trevorlinton
