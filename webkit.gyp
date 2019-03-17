#
#
# BEFORE YOU MODIFY THIS, READ THIS.
#
#
# We do not include native .gyp files from projects. Why? It's very easy with defines
# and exclusions to just specify what you want, and what you dont.
#
# Files get into here through derived.gypi, the files included
{
  'includes':[
		'tools/features.gypi',
		'tools/sources.gypi',
		'tools/derived.gypi',
		'tools/common.gypi',
	],
  'targets': [
	{
		'target_name':'webkit',
		'type':'js_library',
		'dependencies':[
			'xml',
			'jpeg_turbo',
			'png',
			'freetype',
#			'icu',
			'harfbuzz',
			'cairo',
			'curl',
			'zlib',
			'libpixman',
			'fontconfig',
			'webcore_xml',
			'webcore_wtf',
			'webcore_svg',
			'webcore_loader',
			'webcore_html',
			'webcore_dom',
			'webcore_css',
			'webcore_rendering',
			'webcore_page',
			'webcore_style',
			'webcore_derived',
			'webcore_platform',
			'webcore_history',
			'webcore_editing',
			'webcore_storage',
			'webcore_angle',
			'webcore_support',
		],
		'sources':[
			'<(DEPTH)/src/WebView.h',
			'<(DEPTH)/src/WebView.cpp',
			#'<(DEPTH)/src/GLWebView.h',
			#'<(DEPTH)/src/GLWebView.cpp',
			'<(DEPTH)/src/Main.cpp',
			'<(DEPTH)/src/webkit.api.js',
			'<(DEPTH)/src/webkit.pre.js',
			'<(DEPTH)/src/webkit.post.js',
		],
		'defines+':['<@(feature_defines)','CAIRO_HAS_FT_FONT','CAIRO_HAS_FC_FONT','CAIRO_HAS_EGL_FUNCTIONS'],
		'include_dirs':['<@(webcore_includes)','<(DEPTH)/src/',],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
		'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_support',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)','CAIRO_HAS_FT_FONT','CAIRO_HAS_FC_FONT','CAIRO_HAS_EGL_FUNCTIONS'],
		'sources':[
			'<(DEPTH)/src/WebCoreSupport/ApplicationCacheHostJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/ChromeClientJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/ChromeClientJS.h',
			'<(DEPTH)/src/WebCoreSupport/DocumentLoaderJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/DocumentLoaderJS.h',
			'<(DEPTH)/src/WebCoreSupport/FileSystemJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/FrameJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/FrameJS.h',
			'<(DEPTH)/src/WebCoreSupport/FrameLoaderClientJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/FrameLoaderClientJS.h',
			'<(DEPTH)/src/WebCoreSupport/FrameNetworkingContextJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/FrameNetworkingContextJS.h',
			'<(DEPTH)/src/WebCoreSupport/SharedBufferJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/StorageAreaJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/StorageJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/StorageNamespaceJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/RenderThemeJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/RenderThemeJS.h',
			'<(DEPTH)/src/WebCoreSupport/WebKitJSStrategies.cpp',
			'<(DEPTH)/src/WebCoreSupport/WebKitJSStrategies.h',
			'<(DEPTH)/src/WebCoreSupport/RuntimeEnabledFeaturesJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/RuntimeEnabledFeaturesJS.h',
			'<(DEPTH)/src/WebCoreSupport/DebuggerJS.h',
			'<(DEPTH)/src/WebCoreSupport/DOMPluginArray.h',
			'<(DEPTH)/src/WebCoreSupport/khrplatform.h',
			'<(DEPTH)/src/WebCoreSupport/PageBindings.cpp',
			'<(DEPTH)/src/WebCoreSupport/PageBindings.h',
			'<(DEPTH)/src/WebCoreSupport/PasteboardJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/ScriptController.cpp',
			'<(DEPTH)/src/WebCoreSupport/ScriptValue.h',
			'<(DEPTH)/src/WebCoreSupport/AbstractView.h',
			'<(DEPTH)/src/WebCoreSupport/SharedTimer.cpp',
			'<(DEPTH)/src/WebCoreSupport/AcceleratedContext.h',
			'<(DEPTH)/src/WebCoreSupport/AcceleratedContext.cpp',
			'<(DEPTH)/src/WebCoreSupport/WebFrameJS.h',
			'<(DEPTH)/src/WebCoreSupport/WebFrameJS.cpp',
			'<(DEPTH)/src/WebCoreSupport/Stubs/HTMLPluginElement.cpp',
			'<(DEPTH)/src/WebCoreSupport/GraphicsLayerCairo.cpp',
			'<(DEPTH)/src/WebCoreSupport/GraphicsLayerCairo.h',
			'<(DEPTH)/src/EmscriptenSupport.cpp',
		],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)','<(DEPTH)/src/',],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
		'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_xml',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_xml_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_wtf',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_wtf_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_svg',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_svg_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_loader',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_loader_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_html',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_html_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_dom',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_dom_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_css',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_css_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_rendering',
    #
		'type':'static_library',
    #
		'defines+': ['<@(feature_defines)'],
		'sources':['<@(webcore_rendering_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_page',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_page_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_style',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_style_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'webcore_derived',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_derived_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_platform',
    #
		'type':'static_library',
    #
		'defines+':[ '<@(feature_defines)', 'CAIRO_HAS_FT_FONT', 'CAIRO_HAS_FC_FONT', 'CAIRO_HAS_EGL_FUNCTIONS', ],
		'sources':['<@(webcore_platform_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'], #,'<(DEPTH)/deps/skia/include/core/','<(DEPTH)/deps/skia/include/config/'
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_history',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_history_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
},
	{
		'target_name':'webcore_editing',
    #
		'type':'static_library',
    #
		'defines+':['<@(feature_defines)'],
		'sources':['<@(webcore_editing_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name': 'webcore_storage',
    #
		'type':'static_library',
    #
		'defines+': ['<@(feature_defines)'],
		'sources': ['<@(webcore_storage_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name': 'webcore_angle',
    #
		'type':'static_library',
    #
		'defines+': ['<@(feature_defines)'],
		'sources': ['<@(webcore_angle_files)',],
		'sources/':[ ['exclude','<(webcore_excludes)'] ],
		'include_dirs':['<@(webcore_includes)'],
		'cflags+':['-Werror -include ../deps/WebKit/Source/WebCore/WebCorePrefix.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name': 'xml',
    #
		'type':'static_library',
    #
		'sources':['<@(libxml2)',],
		'include_dirs':[
			'<(DEPTH)/deps/zlib', '<(DEPTH)/deps/libxml2/include', '<(DEPTH)/deps/libxml2/config',
		],
		'sources/':[['exclude','(test|rngparser|trio|runxmlconf|runtest|runsuite|python/|macos/|win32config|nano|elfgccheck|tutorial|win32/|example|doc/|xmlcatalog\\.c$|testlimits\\.c$|gjobread\\.c$|macos_main\\.c$|xmllint\\.c$)'],],
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name':'jpeg_turbo',
    #
		'type':'static_library',
    #
		'sources':['<@(libjpeg_turbo)',],
		'sources/':[
		['exclude','(cjpeg\\.c$|djpeg\\.c$|example\\.c$|java/|jccolext\\.c$|jcstest\\.c$|jdcolext\\.c$|jdmrgext\\.c$|jpegtran\\.c$|md5cmp\\.c$|rdjpgcom\\.c$|jsimd_arm\\.c$|jsimd_i386\\.c$|jsimd_mips_dspr2_asm\\.h$|jsimd_mips\\.c$|jsimd_x86_64\\.c$|jsimdcfg\\.inc\\.h$|tjbench\\.c$|tjunittest\\.c$|tjutil\\.c$|tjutil\\.h$|turbojpeg-jni\\.c$|wrjpgcom\\.c$)'],
		],
		'include_dirs': [
			'<(DEPTH)/deps/libjpeg_turbo',
			'<(DEPTH)/deps/libjpeg_turbo/config',
		],
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'png',
    #
		'type':'static_library',
    #
		'sources':['<@(libpng)',],
		'include_dirs':['<(DEPTH)/deps/libpng', '<(DEPTH)/deps/zlib/' ],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name': 'freetype',
    #
		'type':'static_library',
    #
		'sources': ['<@(freetype)',],
		'sources/': [ ['exclude', '(builds/|util/|/tools|autofit/af|gzip/ad|gzip/inf|gzip/z|/ftmac\\.c$|/ftbase\\.c$|/bdf\\.c$|ftcache\\.c$|cff\\.c$|/type1cid\\.c$|/gxvalid\\.c$|/otvalid\\.c$|pcf\\.c$|pfr\\.c$|/raster\\.c$|sfnt\\.c$|/smooth\\.c$|/truetype\\.c$|/type1\\.c$|/type42\\.c$|ftzopen\\.c$|/psaux\\.c$|/pshinter\\.c$|/psnames\\.c$|ttsbit0\\.c$|/gxvfgen\\.c$|/infutil\\.c$)'] ],
		'defines+': [
			'FT2_BUILD_LIBRARY',
			'FT_CONFIG_CONFIG_H=<ftconfig.h>',
		],
		'include_dirs': [
			'<(DEPTH)/deps/freetype',
			'<(DEPTH)/deps/freetype/include',
			'<(DEPTH)/deps/freetype/include/freetype/config',
			'<(DEPTH)/deps/zlib'
		],
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name':'libpixman',
    #
		'type':'static_library',
    #
		'defines': ['HAVE_CONFIG_H','PIXMAN_NO_TLS'],
		'sources': ['<@(pixman)',],
		'sources/': [ ['exclude', '(demos/|test/|pixman-sse2\\.c$|pixman-ssse3\\.c$|pixman-vmx\\.c$|pixman-region\\.c$)'] ],
		'include_dirs': [
			'<(DEPTH)/deps/pixman',
			'<(DEPTH)/deps/pixman/pixman',
		],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name':'harfbuzz',
    #
		'type':'static_library',
    #
		'defines': ['HB_OT','HB_NO_MT','HAVE_OT', 'HAVE_ICU'],
		'sources': ['<@(harfbuzz_files)',],
		'sources/': [ ['exclude', '(glib|coretext|gobject|io1|jcstest)'] ],
		'include_dirs': [
			'<(DEPTH)/deps/harfbuzz',
			'<(DEPTH)/deps/freetype',
			'<(DEPTH)/deps/freetype/include',
			'<(DEPTH)/deps/freetype/include/freetype/config/',
		],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-DHAVE_ICU=1 -Wno-error'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
#	{
#		'target_name':'icu',
#    #
#		'type':'static_library',
#    #
#		'defines': ['HB_OT','HB_NO_MT','HAVE_OT',],
#		'sources': ['<@(harfbuzz_files)',],
#		'sources/': [ ['exclude', '(glib|coretext|icu|gobject|io1|jcstest)'] ],
#		'include_dirs': [
#			'<(DEPTH)/deps/harfbuzz',
#			'<(DEPTH)/deps/freetype',
#			'<(DEPTH)/deps/freetype/include',
#			'<(DEPTH)/deps/freetype/include/freetype/config/',
#		],
#		'cxx':'<(emscripten_cc)',
#		'cflags+':['-Werror'],
#    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
#		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
#  },
	{
		'target_name': 'cairo',
    #
		'type':'static_library',
    #
		'defines+': ['CAIRO_NO_MUTEX','HAVE_CONFIG_H'], #,'PIXMAN_NO_TLS','HAVE_UINT64_T'
		'sources': ['<@(cairo)',],
		'sources/': [['exclude', '(prefs/|perf/|util/|test/|os2|win32|beos|qt|boilerplate|arm-|sse2|pdf|quartz|script-|wgl|xcb|vg-|-drm|skia-|tee-|xlib-|xml-|-vmx|utils/|cogl|directfb-|glx-|check-has-hidden|perceptualdiff\\.c$|cairo-ps-surface\\.c$|cairo-pdf-surface\\.c$|cairo-svg-surface\\.c$|cairo-time\\.c$|/test-|check-link\\.c$)'],], 
		'include_dirs': [
			'<(DEPTH)/deps/cairo',
			'<(DEPTH)/deps/cairo/src',
			'<(DEPTH)/deps/cairo/pixman',
			'<(DEPTH)/deps/cairo/config',
			'<(DEPTH)/deps/zlib',
			'<(DEPTH)/deps/libpng/',
			'<(DEPTH)/deps/freetype',
			'<(DEPTH)/deps/freetype/include',
			'<(DEPTH)/deps/freetype/include/freetype/config/',
			'<(DEPTH)/deps/fontconfig',
			'<(DEPTH)/deps/pixman/pixman',
			'<(DEPTH)/src/WebCoreSupport/',
		],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
	},
	{
		'target_name': 'fontconfig',
    #
		'type':'static_library',
    #
		'defines+': ['FONTCONFIG_PATH="/usr/lib/fontconfig/"','FC_CACHEDIR="/usr/lib/fontconfig/cache"','ENABLE_LIBXML2'],
		'sources': ['<@(fontconfig_files)',],
		'include_dirs': [
			'<(DEPTH)/deps/fontconfig',
			'<(DEPTH)/deps/fontconfig/src',
			'<(DEPTH)/deps/fontconfig/config',
			'<(DEPTH)/deps/freetype/',
			'<(DEPTH)/deps/freetype/include',
			'<(DEPTH)/deps/freetype/include/freetype/config/',
			'<(DEPTH)/deps/libxml2/include/',
			'<(DEPTH)/deps/skia/third_party/fontconfig/config/fc-glyphname/'
		],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-Werror -include ../deps/fontconfig/config/config.h'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name': 'curl',
    #
		'type':'static_library',
    #
		'sources': ['<@(curl)',],
		'sources/': [['exclude','(docs/|m4/|packages/|perl/|tests/|vs/|winbuild/|CMake/|tool_|vtls/|macos/)'],],
		'include_dirs': [
			'<(DEPTH)/deps/zlib',
		  '<(DEPTH)/deps/curl',
		  '<(DEPTH)/deps/curl/include',
		  '<(DEPTH)/deps/curl/build',
		  '<(DEPTH)/deps/curl/build/include',
		  '<(DEPTH)/deps/curl/build/include/curl',
			'<(DEPTH)/deps/curl/build/lib',
			'<(DEPTH)/deps/curl/lib',
		],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },
	{
		'target_name':'zlib',
    #
		'type':'static_library',
    #
		'sources':['<@(zlib_files)'],
		'include_dirs':['<(DEPTH)/deps/zlib',],
		'cxx':'<(emscripten_cc)',
		'cflags+':['-Werror'],
    'ldflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
		'jsflags+':['-s ASSERTIONS=1 -s FULL_ES2=1  -s ASM_JS=1 -s WASM=0 -s USE_SDL=2 -std=c++11 -s USE_ICU=1'],
  },

	#'sources/': [['exclude', '(fcobjs.c$)'],],
	#{
	#  'target_name':'webcore_derived_sources',
	#  'type':'none',
	#  'sources':[
	#    '<(DEPTH)/WebKit/Source/WebCore/dom/EventNames.in',
	#    '<(DEPTH)/WebKit/Source/WebCore/DerivedSources.make',
	#  ],
	#  'actions':[
	#    {
	#      'action_name':'generate',
	#      'inputs':['<(_sources)'],
	#      'outputs':['<(DEPTH)/src/derived/derived.STAMP'],
	#      'action':['bash -c "/usr/bin/perl -I <(DEPTH)/deps/WebKit/Source/WebCore/bindings/scripts <(DEPTH)/deps/WebKit/Source/WebCore/dom/make_event_factory.pl --input  <(DEPTH)/deps/WebKit/Source/WebCore/dom/EventNames.in  --outputDir=<(DEPTH)/webcore_bindings/derived ; export SRCROOT=<(DEPTH)/deps/WebKit/Source/WebCore ; export SOURCE_ROOT=<(DEPTH)/deps/WebKit/Source/WebCore ; export WebCore=<(DEPTH)/deps/WebKit/Source/WebCore ; export InspectorScripts=<(DEPTH)/deps/WebKit/Source/JavaScriptCore/inspector/scripts/ ; make -C <(DEPTH)/src/derived -f <(DEPTH)/deps/WebKit/Source/WebCore/DerivedSources.make ; touch <(DEPTH)/src/derived/derived.STAMP'],
	#    },
	#  ],
	#}
  ],
}