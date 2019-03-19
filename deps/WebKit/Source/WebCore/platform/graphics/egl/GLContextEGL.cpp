/*
 * Copyright (C) 2012 Igalia, S.L.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "/home/denis/job/webkit.js/src/WebView.h" // TODO <<<<<<<<<<<<<<<<<<<<<<<
#include "config.h"
#include "GLContextEGL.h"

#if USE(EGL)
#if PLATFORM(JS)
#include "DebuggerJS.h"
#endif

#include "GraphicsContext3D.h"
#include <wtf/OwnPtr.h>

#if USE(CAIRO)
#include <cairo.h>
#endif

#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include "OpenGLShims.h"
#endif

#if ENABLE(ACCELERATED_2D_CANVAS) && USE(CAIRO)
#include <cairo-gl.h>
#endif

/*#include "SDL2/SDL_video.h"
#include "SDL2/SDL_opengles2.h"
#include "SDL2/SDL.h"*/

#include <SDL2/SDL.h>
//#include <SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>

#include <emscripten.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <emscripten/html5.h>

#include "SDL_opengles2.h"
#include "SDL2/SDL.h"

//#include "SDL2/video/emscripten/SDL_emscriptenvideo.h"
//#include "emscripten/SDL_emscriptenvideo.h"

#include <cstdio>



namespace WebCore {

static EGLDisplay gSharedEGLDisplay = EGL_NO_DISPLAY;

#if USE(OPENGL_ES_2)
static const EGLenum gGLAPI = EGL_OPENGL_ES_API;
#else
static const EGLenum gGLAPI = EGL_OPENGL_API;
#endif

static EGLDisplay sharedEGLDisplay()
{
    printf("GLContextEGL::sharedEGLDisplay 1... \n");
// https://github.com/Nekuromento/SDL2D/blob/0c3d27dec7b7044beb5b865fa2be5e8308589e92/Engine/GFX/Window.cpp
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
#if PLATFORM(X11)
		    webkitTrace();
        gSharedEGLDisplay = eglGetDisplay(GLContext::sharedX11Display());//SDL_GL_CreateContext(window_);//eglGetDisplay(GLContext::sharedX11Display());
#elif PLATFORM(JS)
        webkitTrace();
        if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
          webkitTrace();
          // TODO
          //gSharedEGLDisplay = SDL_GL_CreateContext(window_);
				  gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//(unsigned *)62000; //SDL_GL_CreateContext(window_);/* Emscripten hack for default display, is this the ptr or the val? */
          //gSharedEGLDisplay = eglGetCurrentDisplay();
        }
#else
		    webkitTrace();
        gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif
        if (gSharedEGLDisplay != EGL_NO_DISPLAY && (!eglInitialize(gSharedEGLDisplay, 0, 0) || !eglBindAPI(gGLAPI))) {
		        webkitTrace();
            gSharedEGLDisplay = EGL_NO_DISPLAY;
        }
    }
    if (!gSharedEGLDisplay) {
      printf("sharedEGLDisplay !gSharedEGLDisplay!!!\n");
      initialized = false;
    }
    printf("GLContextEGL::sharedEGLDisplay 2... \n");
    return gSharedEGLDisplay;
}

static const EGLint gContextAttributes[] = {
#if USE(OPENGL_ES_2)
    EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
    EGL_NONE
};

static bool getEGLConfig(EGLConfig* config, GLContextEGL::EGLSurfaceType surfaceType)
{

    printf("GLContextEGL::getEGLConfig 1... \n");
    EGLint attributeList[] = {
#if USE(OPENGL_ES_2)
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#else
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_NONE,
        EGL_NONE
    };

    switch (surfaceType) {
    case GLContextEGL::PbufferSurface:
        attributeList[13] = EGL_PBUFFER_BIT;
        break;
    case GLContextEGL::PixmapSurface:
        attributeList[13] = EGL_PIXMAP_BIT;
        break;
    case GLContextEGL::WindowSurface:
        attributeList[13] = EGL_WINDOW_BIT;
        break;
    }

    EGLint numberConfigsReturned;
    return eglChooseConfig(sharedEGLDisplay(), attributeList, config, 1, &numberConfigsReturned) && numberConfigsReturned;
}

PassOwnPtr<GLContextEGL> GLContextEGL::createWindowContext(EGLNativeWindowType window, GLContext* sharingContext, SDL_Window *sdl_window)
{
    printf("GLContextEGL::createWindowContext... \n");

    if (!sdl_window) {
      printf("Invalid window_!!! %s\n", SDL_GetError());
    }

    printf("GLContextEGL::createWindowContext 1... \n");
    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      //gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//(unsigned *)62000; //SDL_GL_CreateContext(sdl_window);
      gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//eglGetCurrentDisplay();
    }
    printf("GLContextEGL::createWindowContext 2... \n");
    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      printf("Invalid gSharedEGLDisplay 1!!! %s\n", SDL_GetError());
    }
    printf("GLContextEGL::createWindowContext 3... \n");
    //EGLContext eglSharingContext = eglGetCurrentContext();//sharingContext ? static_cast<GLContextEGL*>(sharingContext)->m_context : 0;
    EGLContext eglSharingContext = sharingContext ? static_cast<GLContextEGL*>(sharingContext)->m_context : 0;
    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      gSharedEGLDisplay = static_cast<GLContextEGL*>(sharingContext)->m_context;
    }

    printf("GLContextEGL::createWindowContext 4... \n");
    //EGLDisplay display = eglGetCurrentDisplay();//sharedEGLDisplay();
    EGLDisplay display = sharedEGLDisplay();
    if (display == EGL_NO_DISPLAY) {
        printf("Invalid display 1!!! %s\n", SDL_GetError());
        //return nullptr;
    }

    printf("GLContextEGL::createWindowContext 5... \n");
    EGLConfig config;
    if (!getEGLConfig(&config, WindowSurface)) {
        printf("Invalid getEGLConfig 1!!! %s\n", SDL_GetError());
        //return nullptr;
    }

    printf("GLContextEGL::createWindowContext 6... \n");
  EGLContext context = (EGLContext)SDL_GL_GetCurrentContext();
	if(NULL == context) {
		printf("Invalid context 1!!! %s\n", SDL_GetError());
	}

   /* EGLContext context = eglGetCurrentContext();//eglCreateContext(display, config, eglSharingContext, gContextAttributes);
    //EGLContext context = eglCreateContext(display, config, eglSharingContext, gContextAttributes);
    if (context == EGL_NO_CONTEXT) {
        printf("Invalid context 1!!! %s\n", SDL_GetError());
        //return nullptr;
    }*/



    // TODO>>> EGL_DRAW? or EGL_READ
    /*EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);//eglCreateWindowSurface(display, config, window, 0);
    //EGLSurface surface = eglCreateWindowSurface(display, config, window, 0);
    if (surface == EGL_NO_SURFACE) {
        printf("Invalid surface 1!!! %s\n", SDL_GetError());
        //return nullptr;
    }*/

// TODO
    EGLSurface surface = EGL_NO_SURFACE;

   /* SDL_SysWMinfo sysInfo;
    SDL_VERSION(&sysInfo.version); // Set SDL version
    if ( !SDL_GetWindowWMInfo(sdl_window, &sysInfo) ) { 
      printf("Unable to SDL_GetWindowWMInfo: %s\n", SDL_GetError());
    }

  surface = sysInfo.android.surface;*/

    /*SDL_VideoDevice *device = SDL_GetVideoDevice();
    SDL_Window *window2 = (device ? device->windows : NULL);*/
    // https://github.com/etlegacy/etlegacy-libs/blob/master/sdl2/src/video/emscripten/SDL_emscriptenvideo.c#L289
    /*SDL_WindowData *driverdata = (SDL_WindowData *)sdl_window->driverdata;
    if (driverdata) {

         if (driverdata->egl_surface != EGL_NO_SURFACE) {
             surface = driverdata->egl_surface;
             if (surface) 
              printf("surface OK 1!!! %s\n", SDL_GetError());
         } else {
            printf("Invalid driverdata egl_surface 1!!! %s\n", SDL_GetError());
         }
    } else {
      printf("Invalid driverdata 1!!! %s\n", SDL_GetError());
    }*/



    printf("GLContextEGL::createWindowContext 7... \n");
    return adoptPtr(new GLContextEGL(context, surface, WindowSurface));
}

PassOwnPtr<GLContextEGL> GLContextEGL::createPbufferContext(EGLContext sharingContext)
{
    printf("GLContextEGL::createPbufferContext 1... \n");

    EGLDisplay display = sharedEGLDisplay();
    if (display == EGL_NO_DISPLAY) {
        printf("GLContextEGL::createPbufferContext !display!!!\n");
        return nullptr;
    }

    EGLConfig config;
    if (!getEGLConfig(&config, PbufferSurface)) {
        printf("GLContextEGL::createPbufferContext !getEGLConfig!!!\n");
        return nullptr;
    }

    //EGLContext context = eglGetCurrentContext();//eglCreateContext(display, config, sharingContext, gContextAttributes);
    EGLContext context = eglCreateContext(display, config, sharingContext, gContextAttributes);
    if (context == EGL_NO_CONTEXT) {
        printf("GLContextEGL::createPbufferContext !context!!!\n");
        return nullptr;
    }

    static const int pbufferAttributes[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttributes);
    if (surface == EGL_NO_SURFACE) {
        printf("GLContextEGL::createPbufferContext !surface!!!\n");
        eglDestroyContext(display, context);
        return nullptr;
    }

    return adoptPtr(new GLContextEGL(context, surface, PbufferSurface));
}

PassOwnPtr<GLContextEGL> GLContextEGL::createPixmapContext(EGLContext sharingContext)
{
        printf("1 GLContextEGL::createPixmapContext ! !!! !!! !!!\n");

#if PLATFORM(X11)
    EGLDisplay display = sharedEGLDisplay();
    if (display == EGL_NO_DISPLAY)
        return nullptr;

    EGLConfig config;
    if (!getEGLConfig(&config, PixmapSurface))
        return nullptr;

    //EGLContext context = eglGetCurrentContext();//eglCreateContext(display, config, sharingContext, gContextAttributes);
    EGLContext context = eglCreateContext(display, config, sharingContext, gContextAttributes);
    if (context == EGL_NO_CONTEXT)
        return nullptr;

    EGLint depth;
    if (!eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth))
        return nullptr;

    Pixmap pixmap = XCreatePixmap(sharedX11Display(), DefaultRootWindow(sharedX11Display()), 1, 1, depth);
    if (!pixmap)
        return nullptr;

    EGLSurface surface = eglCreatePixmapSurface(display, config, pixmap, 0);

    if (surface == EGL_NO_SURFACE)
        return nullptr;

    return adoptPtr(new GLContextEGL(context, surface, PixmapSurface));
#else
    {
        printf("GLContextEGL::createPixmapContext ! !!! !!! !!!\n");
    }
    return nullptr;
#endif
}

PassOwnPtr<GLContextEGL> GLContextEGL::createContext(EGLNativeWindowType window, GLContext* sharingContext, SDL_Window *sdl_window)
{
    printf("GLContextEGL::createContext... \n");

    if (!sdl_window) {
      printf("createContext: Invalid window_!!!\n %s", SDL_GetError());
    }

    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      //gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//(unsigned *)62000; //SDL_GL_CreateContext(sdl_window);
      //gSharedEGLDisplay = (unsigned *)62000; //SDL_GL_CreateContext(sdl_window);
      gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//eglGetCurrentDisplay();
    }
    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      printf("Invalid gSharedEGLDisplay 2!!!\n %s", SDL_GetError());
    }
    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      gSharedEGLDisplay = static_cast<GLContextEGL*>(sharingContext)->m_context;
    }

    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
        printf("GLContextEGL::createContext gSharedEGLDisplay ! !!! !!! !!!\n");
    }

    if (!sharedEGLDisplay()) {
        printf("createContext: Invalid sharedEGLDisplay!!! %s\n", SDL_GetError());
        //return nullptr;
    }

    static bool initialized = false;
    static bool success = true;
    if (!initialized) {
#if !USE(OPENGL_ES_2)
        success = initializeOpenGLShims();
#endif
        initialized = true;
    }
    if (!success) {
      printf("Invalid initializeOpenGLShims!!!\n %s", SDL_GetError());
      //return nullptr;
    }

    //EGLContext eglSharingContext = eglGetCurrentContext();
    EGLContext eglSharingContext = sharingContext ? static_cast<GLContextEGL*>(sharingContext)->m_context : 0;

    OwnPtr<GLContextEGL> context = window ? createWindowContext(window, sharingContext, sdl_window) : nullptr;
#if !PLATFORM(JS)
    if (!context)
        context = createPixmapContext(eglSharingContext);

    if (!context)
        context = createPbufferContext(eglSharingContext);
#else
		if (!context)
				context = createWindowContext(2, sharingContext, sdl_window); // hack for emscripten, 1=onscreen, 2=offscreen
#endif

		return context.release();
}

GLContextEGL::GLContextEGL(EGLContext context, EGLSurface surface, EGLSurfaceType type)
    : /*m_context(context)
    , m_surface(surface)
    , m_type(type)*/
#if USE(CAIRO)
    //, 
    m_cairoDevice(0)
#endif
	{
    printf("GLContextEGL::GLContextEGL 1... \n");
    //m_display = eglGetCurrentDisplay();

    /*
    m_surface = eglGetCurrentSurface(EGL_DRAW);
    m_context = eglGetCurrentContext();
    */

//    m_surface = surface;
    m_context = context;

  /*if(!m_surface) {
    printf("GLContextEGL::GLContextEGL !m_surface\n");
  }*/

  if(!m_context) {
    printf("GLContextEGL::GLContextEGL !m_context\n");
  }

/*m_context = context;
m_surface = surface;*/
m_type = type;


#if PLATFORM(JS)

    if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      //gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//(unsigned *)62000; //SDL_GL_CreateContext(sdl_window);
      gSharedEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//eglGetCurrentDisplay();
    }
    /*if (!gSharedEGLDisplay || gSharedEGLDisplay == EGL_NO_DISPLAY) {
      gSharedEGLDisplay = static_cast<GLContextEGL*>(sharingContext)->m_context;
    }*/
	// TODO
  /*if (!window_) {
    printf("Invalid window_!!!\n");
  }
  gSharedEGLDisplay = SDL_GL_CreateContext(window_);*/
#endif
    printf("GLContextEGL::GLContextEGL 2... \n");
}

GLContextEGL::~GLContextEGL()
{
    printf("GLContextEGL::~GLContextEGL 1... \n");

#if USE(CAIRO)
    if (m_cairoDevice)
        cairo_device_destroy(m_cairoDevice);
#endif

SDL_GL_MakeCurrent(WebView::kWindow_, WebView::kContext);

    /*EGLDisplay display = sharedEGLDisplay();
    if (m_context) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(display, m_context);
    }*/

    /*if (m_surface)
        eglDestroySurface(display, m_surface);*/
    printf("GLContextEGL::~GLContextEGL 2... \n");
}

bool GLContextEGL::canRenderToDefaultFramebuffer()
{
#if PLATFORM(JS)
	return true;
#endif
    return m_type == WindowSurface;
}

IntSize GLContextEGL::defaultFrameBufferSize()
{
#if PLATFORM(JS)
	
    return IntSize(512, 512);
#endif
    if (!canRenderToDefaultFramebuffer())
        return IntSize();

    EGLint width = 512, height = 512;
    /*if (!eglQuerySurface(sharedEGLDisplay(), m_surface, EGL_WIDTH, &width)
        || !eglQuerySurface(sharedEGLDisplay(), m_surface, EGL_HEIGHT, &height))
        return IntSize();*/

    return IntSize(width, height);
}

bool GLContextEGL::makeContextCurrent()
{
#if PLATFORM(JS)
	
#endif
/*if(!m_surface) {
  printf("!!!! makeContextCurrent !m_surface\n");
}*/


if(!m_context) {
  printf("!!!! makeContextCurrent !GLContextEGL::m_context\n");
}
    //ASSERT(m_context && m_surface);

    GLContext::makeContextCurrent();
    /*if (eglGetCurrentContext() == m_context)
        return true;*/

    if ((EGLContext)SDL_GL_GetCurrentContext() == m_context)
        return true;
        

    return  SDL_GL_MakeCurrent(WebView::kWindow_, WebView::kContext);

    //return eglMakeCurrent(sharedEGLDisplay(), m_surface, m_surface, m_context);
}

void GLContextEGL::swapBuffers()
{
  // >>>>>>>
#if PLATFORM(JS)
	::glFlush();
#else
	/*ASSERT(m_surface);
	eglSwapBuffers(sharedEGLDisplay(), m_surface);*/
  
        SDL_GL_SwapBuffers( );
#endif
}

void GLContextEGL::waitNative()
{
#if PLATFORM(JS)
	//SDL_Delay(Uint32 ms)
#endif
   // eglWaitNative(EGL_CORE_NATIVE_ENGINE);
}

#if USE(CAIRO)
cairo_device_t* GLContextEGL::cairoDevice()
	{
  printf("!!!! GLContextEGL::cairoDevice()1\n");
#if PLATFORM(JS)
	
#endif
    if (m_cairoDevice)
        return m_cairoDevice;

#if ENABLE(ACCELERATED_2D_CANVAS)
    m_cairoDevice = cairo_egl_device_create(sharedEGLDisplay(), m_context);
#endif

    return m_cairoDevice;
}
#endif

#if ENABLE(WEBGL) || USE(3D_GRAPHICS)
PlatformGraphicsContext3D GLContextEGL::platformContext()
{
  printf("!!!! GLContextEGL::platformContext()1\n");
#if PLATFORM(JS)
	
#endif
    return m_context;
}
#endif

} // namespace WebCore

#endif // USE(EGL)
