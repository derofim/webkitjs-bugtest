/*
 * Copyright (C) 2012 Igalia S.L.
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
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301 USA
 */

#ifndef GLContext_h
#define GLContext_h

#include "GraphicsContext3D.h"
#include "Widget.h"
#include <wtf/Noncopyable.h>
#include <wtf/PassOwnPtr.h>

#if USE(EGL) && !PLATFORM(GTK) && !PLATFORM(NIX) && !PLATFORM(JS)
#include "eglplatform.h"
typedef EGLNativeWindowType GLNativeWindowType;
#else
typedef uint64_t GLNativeWindowType;
#endif

#include "GLContext.h"
#include <EGL/egl.h>

#if USE(CAIRO)
typedef struct _cairo_device cairo_device_t;
#endif

#if PLATFORM(X11)
typedef struct _XDisplay Display;
#endif

#include <SDL2/SDL.h>

namespace WebCore {

class GLContext {
    WTF_MAKE_NONCOPYABLE(GLContext);
public:
    enum EGLSurfaceType { PbufferSurface, WindowSurface, PixmapSurface };
    
    static PassOwnPtr<GLContext> createContextForWindow(GLNativeWindowType windowHandle, GLContext* sharingContext, SDL_Window *sdl_window);
    static PassOwnPtr<GLContext> createOffscreenContext(GLContext* sharing, SDL_Window *sdl_window);
    static PassOwnPtr<GLContext> createOffscreenContext(GLContext* sharing);
    static GLContext* getCurrent();
    static GLContext* sharingContext();

    GLContext();
    virtual ~GLContext();
    virtual bool makeContextCurrent();
    virtual void swapBuffers() = 0;
    virtual void waitNative() = 0;
    virtual bool canRenderToDefaultFramebuffer() = 0;
    virtual IntSize defaultFrameBufferSize() = 0;

#if USE(CAIRO)
    virtual cairo_device_t* cairoDevice() = 0;
#endif

#if PLATFORM(X11)
    static Display* sharedX11Display();
    static void cleanupSharedX11Display();
#endif

    static void addActiveContext(GLContext*);
    static void removeActiveContext(GLContext*);
    static void cleanupActiveContextsAtExit();

#if USE(3D_GRAPHICS)
    virtual PlatformGraphicsContext3D platformContext() = 0;
#endif

    void setWindow(SDL_Window *sdl_window);
    //static void setGlobalWindow(SDL_Window *sdl_window);

    // TODO
    SDL_Window *window_;

    static SDL_Window *gWindow;

    EGLContext m_context;
    //EGLSurface m_surface;
    EGLSurfaceType m_type;

    static OwnPtr<GLContext> sharingC;
};

} // namespace WebCore

#endif // GLContext_h
