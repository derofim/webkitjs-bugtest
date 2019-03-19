/*
 * Copyright (C) 2014 True Interactions. BSD-style License
 */

#include <emscripten.h>
#include <emscripten/html5.h>

//#define SDL_GL_CREATE

#include "config.h"
#include "WebView.h"
#include "Page.h"
#include "Frame.h"
#include "MainFrame.h"
#include "ChromeClientJS.h"
#include "FrameLoaderClientJS.h"
#include "WebKitJSStrategies.h"
#include "DebuggerJS.h"
#include "Chrome.h"
#include "DocumentLoader.h"
#include "ElementIterator.h"
#include "FrameView.h"
#include "ImageBuffer.h"
#include "ImageObserver.h"
#include "IntRect.h"
#include "MainFrame.h"
#include "RenderStyle.h"
#include "Settings.h"
#include "RuntimeEnabledFeaturesJS.h"
#include "GraphicsContext.h"
#include "EmptyClients.h"
#if USE(TILED_BACKING_STORE)
#include "TiledBackingStore.h"
#endif

#include <platform/cairo/WidgetBackingStore.h>
#include <platform/cairo/WidgetBackingStoreCairo.h>
#include "cairo.h"
#include "cairo-gl.h"
#include "PlatformContextCairo.h"

#if USE(ACCELERATED_COMPOSITING)
#include "GL/glew.h"
#include "GLContext.h"
#endif

//#include <SDL.h>
#include <SDL2/SDL.h>
//#include <SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>

#include <stdio.h>
#include <string>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <cmath>

using namespace WebCore;
using namespace WTF;

namespace WebCore {

     SDL_Window * WebView::kWindow_ = nullptr;
     SDL_GLContext * WebView::kContext = nullptr;

  //cairo_t *WebView::cairo_context_;
  //cairo_surface_t *WebView::cairo_surface_;

	WebView::WebView(SDL_Window *window, SDL_GLContext& context, int width, int height, bool accelerated = false) : 
    context_(context), window_(window)
	{
    kWindow_ = window;
    kContext = &context;
    
    printf("creating WebView...\n");

    if (!context) {
      printf("1 WebView !context! 1\n");
    }
    if (!context_) {
      printf("2 WebView !context! 1\n");
    }

    //WebView::cairo_context_ = cairo_context;

    //GLContext::setGlobalWindow(window_);

#if USE(ACCELERATED_COMPOSITING)
    //GLContext::currentContext()->gWindow = window_;
    GLContext::gWindow = window_;
    //GLContext::gContext = context_;
#endif

		webkitTrace();
		WebKitJSStrategies::initialize();
		webkitTrace();
		m_private = new WebViewPrivate();
		webkitTrace();
		m_private->transparent = false;
		Page::PageClients pageClients;
		fillWithEmptyClients(pageClients);
		webkitTrace();

    // https://github.com/emscripten-core/emscripten/blob/master/tools/ports/sdl.py#L10
		/*if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
      printf("Error SDL_INIT_VIDEO %s\n", SDL_GetError());
			//abort();
    }*/

    if (!window_) {
      printf("Unable to create window: %s\n", SDL_GetError());
    }
#if USE(ACCELERATED_COMPOSITING)
////
/*if(!m_private->glContext)
  m_private->glContext = GLContext::createContextForWindow(0, GLContext::sharingContext(), sdl_window);*/
////
    if(m_private->glContext)
      m_private->glContext->setWindow(window_);
    else
    {
      printf("(makes warning) !m_private->glContext......: %s\n", SDL_GetError());
    }
    
#endif

    if (!context_) {
      printf("Unable to create context: %s\n", SDL_GetError());
    }
		webkitTrace();

		m_private->chromeClient = WebCore::ChromeClientJS::createClient(this);
		webkitTrace();
		pageClients.chromeClient = m_private->chromeClient->toChromeClient();
		m_private->mainFrame = new WebCore::WebFrameJS(this);
		webkitTrace();
		pageClients.loaderClientForMainFrame = WebCore::FrameLoaderClientJS::createClient(m_private->mainFrame);
		m_private->corePage = new Page(pageClients);
		webkitTrace();
		m_private->corePage->addLayoutMilestones(DidFirstVisuallyNonEmptyLayout);
    m_private->corePage->setGroupName("webkit.js");
		//m_private->corePage->setGroupName(L"webkit.js");

		webkitTrace();
		m_private->corePage->settings().setMediaEnabled(false);
		m_private->corePage->settings().setScreenFontSubstitutionEnabled(true);
		m_private->corePage->settings().setWebSecurityEnabled(false);
		m_private->corePage->settings().setJavaEnabled(false);
		m_private->corePage->settings().setMockScrollbarsEnabled(false);
		m_private->corePage->settings().setLoadsImagesAutomatically(true);
		m_private->corePage->settings().setDefaultFixedFontSize(13);
		m_private->corePage->settings().setDefaultFontSize(16);
		m_private->corePage->settings().setStandardFontFamily("Liberation");
		m_private->corePage->settings().setScreenFontSubstitutionEnabled(true);
		m_private->corePage->settings().setScriptEnabled(false);
		m_private->corePage->settings().setPluginsEnabled(false);

		webkitTrace();
    // TODO: https://github.com/pqrkchqps/MusicBrowser/blob/03216439d1cc3dae160f440417fcb557bb72f8e4/src/webkit/glue/webpreferences.cc
		if(accelerated) {
		  webkitTrace();
			m_private->accelerated = true;
			m_private->corePage->settings().setMinimumAccelerated2dCanvasSize(1);
			m_private->corePage->settings().setAcceleratedCompositedAnimationsEnabled(true);
			m_private->corePage->settings().setAcceleratedDrawingEnabled(true);
			m_private->corePage->settings().setAcceleratedFiltersEnabled(true);
			m_private->corePage->settings().setAcceleratedCompositingEnabled(true);
			m_private->corePage->settings().setAcceleratedDrawingEnabled(true);
			m_private->corePage->settings().setTiledBackingStoreEnabled(false);
			m_private->corePage->settings().setForceCompositingMode(true);
			m_private->corePage->settings().setApplyDeviceScaleFactorInCompositor(true);
      // Enable gpu-accelerated 2d canvas if requested on the command line.
      // m_private->corePage->settings().setAccelerated2dCanvasEnabled(true);
		} else {
		  webkitTrace();
			m_private->accelerated = false;
			//m_private->corePage->settings().setAcceleratedCompositedAnimationsEnabled(false);
			//m_private->corePage->settings().setAcceleratedDrawingEnabled(false);
			//m_private->corePage->settings().setAcceleratedFiltersEnabled(false);
			//m_private->corePage->settings().setAcceleratedCompositingEnabled(false);
			//m_private->corePage->settings().setAcceleratedDrawingEnabled(false);
			//m_private->corePage->settings().setTiledBackingStoreEnabled(false);
    }

		webkitTrace();
    printf("initializeScreens...\n");
    printf("WebView: width = %d height = %d \n", width, height);
          if(!m_private->glContext) {
        printf("1 pre postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }
		initializeScreens(width, height);
          if(!m_private->glContext) {
        printf("2 pre postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }
    postinitializeScreens(width, height);
          if(!m_private->glContext) {
        printf("3 pre postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }

    if(m_private->glContext)
      m_private->glContext->setWindow(window_);
    else
    {
      printf("2 !m_private->glContext......: %s\n", SDL_GetError());
    }

		webkitTrace();
		m_private->mainFrame->init();
		m_private->corePage->setIsVisible(true, true);
		m_private->corePage->setIsInWindow(true);

		webkitTrace();
    printf("resize...\n");
		resize(width, height);

		webkitTrace();
		if(!m_private->accelerated) {
			EM_ASM({
				postMessage({target:'status',context:'ready'});
			});
		} else
			m_private->mainFrame->coreFrame()->view()->enterCompositingMode();

	}

	WebView::~WebView() {
		webkitTrace();
		if (m_private->mainFrame && m_private->mainFrame->coreFrame())
			m_private->mainFrame->coreFrame()->loader().detachFromParent();

		delete m_private->corePage;
		m_private->corePage = 0;
		delete m_private;
	}

	void WebView::setUrl(char *) {
		webkitTrace();
	}

	char *WebView::url() {
		webkitTrace();
		return NULL;
	}

	void WebView::setHtml(const char *data, int length) {
		webkitTrace();
		WebCore::MainFrame& frame = m_private->mainFrame->coreFrame()->mainFrame();
		ASSERT(frame.isMainFrame());
		WebCore::FrameLoader& loader = frame.loader();
		ASSERT(loader.activeDocumentLoader());
		loader.activeDocumentLoader()->writer().begin(URL());
		loader.activeDocumentLoader()->writer().addData(data, length);
		loader.activeDocumentLoader()->writer().end();
	}

	char *WebView::html() {
		webkitTrace();
		return NULL;
	}

	WebCore::FloatRect WebView::positionAndSize() {
		webkitTrace();
		return m_private->size;
	}

#if USE(ACCELERATED_COMPOSITING)
	WebCore::GLContext *WebView::glWindowContext(SDL_Window *sdl_window) {
      printf("WebView::glWindowContext...\n");
		webkitTrace();

		if (m_private->glContext) {
			return m_private->glContext.get();
    } else {
      printf("no glWindowContext :()...%s\n", SDL_GetError());
    }

		webkitTrace();

    if (!sdl_window) {
      printf("WebView::glWindowContext: Invalid sdl_window!!!\n");
    }

    //->>>>>>>
    if(context_) {
      //GLContext::sharingContext()->m_context = reinterpret_cast<EGLContext>(context_);
    } else {
		  webkitTrace();
      printf("glWindowContext !context_ !\n");
    }

    //->>>>>>>
		//m_private->glContext = GLContext::createContextForWindow(1, GLContext::sharingContext(), sdl_window);
    if(!m_private->glContext)
      m_private->glContext = GLContext::createContextForWindow(0, GLContext::sharingContext(), sdl_window);
    
		if(!m_private->glContext) {
      printf("no glContext...%s\n", SDL_GetError());
    }

    m_private->glContext->setWindow(sdl_window); // TODO
		webkitTrace();
      printf("WebView::glWindowContext 2...\n");
		return m_private->glContext.get();
	}
#endif

void WebView::postinitializeScreens(int width, int height) {
        printf("WebView::postinitializeScreens...%s\n", SDL_GetError());
#if USE(ACCELERATED_COMPOSITING)
			//m_private->glContext = GLContext::createContextForWindow(1, GLContext::sharingContext(), window_);
      if(!m_private->glContext)
			  m_private->glContext = GLContext::createContextForWindow(0, GLContext::sharingContext(), window_);
      if(!m_private->glContext) {
        printf("WebView::postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }
      m_private->glContext->setWindow(window_); // TODO
			ASSERT(m_private->glContext->makeContextCurrent());
			GLContext* context = GLContext::getCurrent();
			ASSERT(context->makeContextCurrent());
#endif
        printf("WebView::postinitializeScreens AcceleratedContext::create 0...\n");

#if USE(ACCELERATED_COMPOSITING)
        printf("WebView::postinitializeScreens AcceleratedContext::create 1...\n");
			if(!m_private->acceleratedContext) {
        printf("WebView::postinitializeScreens AcceleratedContext::create 2...\n");
				m_private->acceleratedContext = AcceleratedContext::create1(this);
      }
        printf("WebView::postinitializeScreens AcceleratedContext::create 3...\n");
			if(!m_private->acceleratedContext) {
        printf("WebView::postinitializeScreens no m_private->acceleratedContext...!!!!!!!!!!!!!!!!!!%s\n", SDL_GetError());
      }
      // >>>>>>>>.
      //m_private->acceleratedContext->initialize();
#endif

			if ( !m_private->sdl_screen ) {
        printf("WebView::postinitializeScreens no m_private->sdl_screen ...%s\n", SDL_GetError());
				//SDL_Quit();
				//exit(2);
			}
}

	void WebView::initializeScreens(int width, int height) {
    printf("initializeScreens: width = %d height = %d \n", width, height);

		webkitTrace();

    if (!window_) {
      printf("WebView::initializeScreens: Invalid window_!!!\n");
    }

		if(!m_private->sdl_screen) {
      printf("creating sdl_screen... %s\n", SDL_GetError());
      //m_private->sdl_screen = SDL_GetWindowSurface(window_);
      m_private->sdl_screen = SDL_GetWindowSurface(window_);//SDL_SetVideoMode(width, height, 32, SDL_OPENGL );
			//SDL_FreeSurface(m_private->sdl_screen);
    }
		if(!m_private->sdl_screen) {
      printf("invalid sdl_screen!!! %s\n", SDL_GetError());
    }

		m_private->size = FloatRect(0,0,(float)width, (float)height);

/*
#ifdef __EMSCRIPTEN__
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#endif*/

		if(m_private->accelerated) {
#ifdef SDL_GL_CREATE
			SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
			SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,	8);
			SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
#endif
// Remember SDL_SetVideoMode()? It's completely gone. SDL 2.0 allows you to have multiple windows, so the old function didn't make sense any more.
			//m_private->sdl_screen = SDL_GetWindowSurface(window_);
      printf("SDL_SetVideoMode 1: width = %d height = %d \n", width, height);

      //->>>>>>>
      m_private->sdl_screen = SDL_GetWindowSurface(window_);//SDL_SetVideoMode(width, height, 32, SDL_OPENGL );
      if(!m_private->sdl_screen) {
        printf("no SDL_SetVideoMode!...%s\n", SDL_GetError());
        //SDL_FreeSurface(m_private->sdl_screen);
      }

    if(context_) {
      printf("SD with context_...%s\n", SDL_GetError());
    } else {
      printf("SD no context_... nooo%s\n", SDL_GetError());
    }

#ifdef SDL_GL_CREATE
			SDL_GL_SetSwapInterval(1);
#endif

    if(context_) {
      printf("2 SD with context_...%s\n", SDL_GetError());
    } else {
      printf("2 SD no context_... nooo%s\n", SDL_GetError());
    }

      if (!window_) {
        printf("WebView::initializeScreens: Invalid window_!!!\n");
      }
        printf("AcceleratedContext::create -2...\n");


    if(GLContext::sharingContext()) {
      printf("GLContext::sharingContext()...%s\n", SDL_GetError());
    } else {
      printf("GLContext::sharingContext()!!! nooo%s\n", SDL_GetError());
    }

    //->>>>>>>
    //->>>>>>>
    if(context_) {
		  webkitTrace();
      printf("view context_ exists\n");
      //GLContext::sharingContext()->m_context = reinterpret_cast<EGLContext>(context_);
    } else {
		  webkitTrace();
      printf("view !context_ !\n");
    }
        printf("AcceleratedContext::create -1...\n");

#if USE(ACCELERATED_COMPOSITING)
			//m_private->glContext = GLContext::createContextForWindow(1, GLContext::sharingContext(), window_);
      if(!m_private->glContext)
			  m_private->glContext = GLContext::createContextForWindow(0, GLContext::sharingContext(), window_);
      if(!m_private->glContext) {
        printf("no glContext!...%s\n", SDL_GetError());
      }
      m_private->glContext->setWindow(window_); // TODO
			ASSERT(m_private->glContext->makeContextCurrent());
			GLContext* context = GLContext::getCurrent();
			ASSERT(context->makeContextCurrent());
#endif
        printf("AcceleratedContext::create 0...\n");

#if USE(ACCELERATED_COMPOSITING)
        printf("AcceleratedContext::create 1...\n");
			if(!m_private->acceleratedContext) {
        printf("AcceleratedContext::create 2...\n");
				m_private->acceleratedContext = AcceleratedContext::create1(this);
      }
        printf("AcceleratedContext::create 3...\n");
			if(!m_private->acceleratedContext) {
        printf("no m_private->acceleratedContext...!!!!!!!!!!!!!!!!!!%s\n", SDL_GetError());
      }
      // >>>>>>>>.
      //m_private->acceleratedContext->initialize();
#endif

			if ( !m_private->sdl_screen ) {
        printf("no m_private->sdl_screen ...%s\n", SDL_GetError());
				//SDL_Quit();
				//exit(2);
			}
		} else {
// Remember SDL_SetVideoMode()? It's completely gone. SDL 2.0 allows you to have multiple windows, so the old function didn't make sense any more.
			//m_private->sdl_screen = SDL_GetWindowSurface(window_);//SDL_SetVideoMode( width, height, 32, SDL_SWSURFACE );
      printf("SDL_SetVideoMode 2: width = %d height = %d \n", width, height);
      m_private->sdl_screen = SDL_GetWindowSurface(window_);//SDL_SetVideoMode(width, height, 32, SDL_OPENGL );
			if (!m_private->sdl_screen) {
        printf("no SDL_SetVideoMode...%s\n", SDL_GetError());
				//SDL_Quit();
				//exit(2);
			}
			SDL_LockSurface(m_private->sdl_screen);
		}
	}

	void WebView::resize(int width, int height)
	{
    printf("SDL_SetViresizedeoMode: width = %d height = %d \n", width, height);
		webkitTrace();
		IntSize oldSize = IntSize(m_private->size.width(), m_private->size.height());
          if(!m_private->glContext) {
        printf("(warn) 1 pre postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }
		initializeScreens(width, height);
          if(!m_private->glContext) {
        printf("2 pre postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }
    postinitializeScreens(width, height);
          if(!m_private->glContext) {
        printf("3 pre postinitializeScreens no glContext!...%s\n", SDL_GetError());
      }
		if(m_private->chromeClient) {
			m_private->chromeClient->widgetSizeChanged(oldSize, IntSize(width,height));
    } else {
      printf("no chromeClient ...%s\n", SDL_GetError());
    }
	}

	void WebView::scrollBy(int offsetX, int offsetY)
	{
		webkitTrace();
		Frame* coreFrame = m_private->mainFrame->coreFrame();
		if (!coreFrame->view())
			return;
		coreFrame->view()->scrollBy(IntSize(offsetX, offsetY));
	}

	/*void WebView::handleSDLEvent(const SDL_Event& event)
	{
		// Note: do not add a trace or printf here, this funciton executes quite
		// a few times that it will cause the browser to slow down to a crawl
		// to finish writing console messages.
		switch (event.type) {
			case SDL_WINDOWEVENT_RESIZED:
			//resize(event.resize.w, event.resize.h);
      // https://wiki.libsdl.org/SDL_WindowEvent
      resize(event.window.data1, event.window.data2);
			break;
			case SDL_VIDEOEXPOSE:
			break;
			case SDL_USEREVENT:
				// We use SDL_USEREVENT to fire the shared timer.
				//fireSharedTimer();
			break;
			case SDL_MOUSEMOTION:
				//handleMotionEvent(m_private->corePage->mainFrame(), event.motion);
			break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				//handleMouseEvent(m_private->corePage->mainFrame(), event.button);
			break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				//handleKeyboardEvent(m_private->corePage, event.key);
			default:
			break;
		}

	}*/

	void WebView::scalefactor(float t) {
		webkitTrace();
//		m_private->corePage->setDeviceScaleFactor(t);
// #if USE(TILED_BACKING_STORE)
//		fprintf(stderr, "tiled backing store set to %f\n",t);
//		m_private->mainFrame->coreFrame()->tiledBackingStore()->setContentsScale(t);
//#endif
	}

	void WebView::resizeEvent(void *) {
		// notImplemented();
	}

	void WebView::paintEvent(void *) {
		// notImplemented();
	}

	void WebView::changeEvent(void *) {
		// notImplemented();
	}

	void WebView::mouseMoveEvent(void *) {
		// notImplemented();
	}

	void WebView::mousePressEvent(void *) {
		// notImplemented();
	}

	void WebView::mouseDoubleClickEvent(void *) {
		// notImplemented();
	}

	void WebView::mouseReleaseEvent(void *) {
		// notImplemented();
	}

	void WebView::contextMenuEvent(void *) {
		// notImplemented();
	}

	void WebView::wheelEvent(void *) {
		// notImplemented();
	}

	void WebView::keyPressEvent(void *) {
		// notImplemented();
	}

	void WebView::keyReleaseEvent(void *) {
		// notImplemented();
	}

	void WebView::dragEnterEvent(void *) {
		// notImplemented();
	}

	void WebView::dragLeaveEvent(void *) {
		// notImplemented();
	}

	void WebView::dragMoveEvent(void *) {
		// notImplemented();
	}

	void WebView::dropEvent(void *) {
		// notImplemented();
	}

	void WebView::focusInEvent(void *) {
		// notImplemented();
	}

	void WebView::focusOutEvent(void *) {
		// notImplemented();
	}

	void WebView::inputMethodEvent(void *) {
		// notImplemented();
	}

	bool WebView::focusNextPrevChild(bool next) {
		// notImplemented();
		return false;
	}

 }


