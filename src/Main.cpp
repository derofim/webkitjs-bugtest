#include "WebView.h"
#include "IntSize.h"

#ifdef DEBUG
#include "Logging.h"
#include <wtf/text/WTFString.h>
#endif

#ifdef CAIRO_TEST
#include "cairo.h"
#include "cairo-gl.h"
#include "PlatformContextCairo.h"
#endif

#include "config.h"
#include "SDL.h"
#include "SDL_syswm.h"
//#include <SDL2/SDL_syswm.h>

#include <emscripten.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <emscripten/html5.h>
#include "SDL_opengles2.h"

#include <cstdio>
#include <string>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <cmath>

using WebCore::WebView;

static GLfloat const kVertexData[] = {
    1.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 1.0f
};

namespace WebCore {
	static WebCore::WebView* mainView = NULL;
#ifdef DEBUG
	String logLevelString() { return getenv("WEBKIT_DEBUG"); }
#endif
}

static GLuint programObject;

// must be POT
static int width = 512;
// must be POT
static int height = 512;

static GLint uniformTex;
static float zoom = 1.0;

static SDL_Window *window;
static SDL_GLContext contextDisplay;
    EGLConfig   glConfig;
    EGLContext  glContext;
    EGLSurface  glSurface;
    EGLint      numConfigs;

//Main loop flag
static bool quit = false;

//Event handler
static SDL_Event e;

static GLuint vertexPosObject;

bool makeCurrent() {
  SDL_GL_MakeCurrent(window, nullptr);

  return !SDL_GL_MakeCurrent(window, contextDisplay);
}

#ifdef CAIRO_TEST
//static cairo_device_t* cairo_device;

/*static cairo_surface_t* cairo_surface;

static cairo_t *cairo_context;*/

static GLuint cairo_texture = 0;

bool makeCairoCurrent() {
  SDL_GL_MakeCurrent(window, nullptr);

  return !SDL_GL_MakeCurrent(window, contextDisplay);
}

// https://www.cairographics.org/samples/
inline void cairo_draw(cairo_surface_t* surface) {
	static double s = 1.0;

  // TODO: https://github.com/acomminos/scrap-engine/blob/f9984bcf67fd83644d93e385215a3d741dfcc2ec/src/scrap/gl/cairo_renderer.cc#L125

	//cairo_t* cr = cairo_create(surface);

	cairo_set_source_rgba(WebView::cairo_context_, 0.0, 1.0, 0.0, 1.0);
	
  //cairo_paint(WebView::cairo_context_);
  cairo_save(WebView::cairo_context_);
  cairo_set_operator(WebView::cairo_context_, CAIRO_OPERATOR_CLEAR);
  cairo_paint(WebView::cairo_context_);
  cairo_restore(WebView::cairo_context_);

	cairo_translate(WebView::cairo_context_, width / 2, height / 2);
	cairo_scale(WebView::cairo_context_, s, s);
	cairo_arc(WebView::cairo_context_, 0.0, 0.0, width / 4, 0.0, 2.0 * 3.14159);
  
	cairo_set_source_rgba(WebView::cairo_context_, 1.0, 0.0, 0.0, 1.0);
	cairo_fill(WebView::cairo_context_);

	cairo_set_source_rgba(WebView::cairo_context_, 1.0, 1.0, 0.0, 1.0);
  cairo_set_line_width (WebView::cairo_context_, 40.96);
  cairo_move_to (WebView::cairo_context_, 76.8, 84.48);
  cairo_rel_line_to (WebView::cairo_context_, 51.2, -51.2);
  cairo_rel_line_to (WebView::cairo_context_, 51.2, 51.2);
  cairo_set_line_join (WebView::cairo_context_, CAIRO_LINE_JOIN_MITER); /* default */
  cairo_stroke (WebView::cairo_context_);

	cairo_surface_flush(surface);
	cairo_destroy(WebView::cairo_context_);

	s += 1.0 / 180.0;

	if(s >= 2.0) s = 1.0;
}
#endif

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);
	if (shader == 0)
		return 0;

	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			printf("Error compiling shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

int Init()
{
	char vShaderStr[] =
		"attribute vec2 vPosition;                \n"
		"attribute vec2 vUV;                \n"
    "varying vec2 v_texcoord;\n"
		//"uniform mat4 uMVPMatrix; \n"
		//"uniform float zoom;	\n"
		"void main()                              \n"
		"{                                        \n"
    "    v_texcoord = vUV;\n"
    "    gl_Position = vec4(vPosition, -1, 1);\n"
//		"   gl_Position = uMVPMatrix * vPosition;              \n"
//		"   gl_Position.x = gl_Position.x * zoom; \n"
//		"   gl_Position.y = gl_Position.y * zoom; \n"
		"}                                        \n";

	char fShaderStr[] =
		"precision mediump float;\n"
    "uniform sampler2D u_tex;\n"
    "varying vec2 v_texcoord;\n"
//		"uniform vec4 vColor;"
		"void main()                                  \n"
		"{                                            \n"
//		"  gl_FragColor = vColor;        \n"
    "    vec4 colour = texture2D(u_tex, v_texcoord);\n"
    "    colour.rgba = colour.bgra; // cairo outputs in ARGB\n"
    "    gl_FragColor = colour;\n"
		"}                                            \n";

	GLuint vertexShader;
	GLuint fragmentShader;
	GLint linked;

	vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

	programObject = glCreateProgram();
	if (programObject == 0)
		return 0;

	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	glBindAttribLocation(programObject, 0, "vPosition");
	glBindAttribLocation(programObject, 1, "vUV");
	glLinkProgram(programObject);
	uniformTex = glGetUniformLocation(programObject, "u_tex");
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint infoLen = 0;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 1)
		{
			char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen));
			glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
			printf("Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(programObject);
		return GL_FALSE;
	}

#ifdef CAIRO_TEST
    glGenTextures(1, &cairo_texture);
#endif

	// No clientside arrays, so do this in a webgl-friendly manner
	glGenBuffers(1, &vertexPosObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertexData), kVertexData,
                 GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	return GL_TRUE;
}

//#define DRAW_TEST

#ifdef DRAW_TEST
///
// Draw using the shader pair created in Init()
//
void Draw()
{

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 0.0f, 0.0f, 0.5f);

#ifdef CAIRO_TEST
  if(makeCairoCurrent()) {
    //cairo_draw(cairo_surface);

	//cairo_set_source_rgba(cairo_context, 1.0, 0.0, 0.0, 1.0);

  cairo_save(WebView::cairo_context_);
  cairo_paint(WebView::cairo_context_);
  cairo_restore(WebView::cairo_context_);
   // Do any pending drawing for the surface and also restore
   // any temporary modification's cairo has made to the surface's state.
   // This function must be called before switching from drawing on the surface
   // with cairo to drawing on it directly with native APIs.
   // If the surface doesn't support direct access, then this function does nothing.
	cairo_surface_flush(WebView::cairo_surface_);

    cairo_gl_surface_swapbuffers(WebView::cairo_surface_);
  } else {
    printf("!makeCairoCurrent \n");
  }

  if(makeCurrent()) {
    int x = 0;
    int y = 0;
	
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programObject);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cairo_texture);
    cairo_surface_flush(WebView::cairo_surface_);
    unsigned char *data = cairo_image_surface_get_data(WebView::cairo_surface_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 cairo_image_surface_get_width(WebView::cairo_surface_),
                 cairo_image_surface_get_height(WebView::cairo_surface_), 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(uniformTex, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
  //
	glVertexAttribPointer(0,  2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          NULL);
	glEnableVertexAttribArray(0);
  //
	glVertexAttribPointer(1,  2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	int w, h, fs;
#ifdef __EMSCRIPTEN__
	emscripten_get_canvas_size(&w, &h, &fs); // width, height, isFullscreen
#else
	w = width;
	h = height;
#endif
	float xs = (float)h / w;
	float ys = 1.0f;
	float mat[] = { xs, 0, 0, 0, 0, ys, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

 glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

  } else {
    printf("!makeCurrent \n");
  }
#endif

}
#endif // DRAW_TEST

extern "C" {
	void setHtml(const char *html) { WebCore::mainView->setHtml(html,strlen(html)); }
	void setTransparent(bool transparent) { WebCore::mainView->setTransparent(transparent); }
	void scrollBy(int x, int y) { WebCore::mainView->scrollBy(x,y); }
	void resize(int w, int h) { WebCore::mainView->resize(w,h); }
	void scalefactor(float sf) { WebCore::mainView->scalefactor(sf); }
	void createWebKit(SDL_Window *window, SDL_GLContext& contextDisplay, int width, int height, bool accel) { WebCore::mainView = new WebCore::WebView(window, contextDisplay, width, height, accel); }
}

void tick() {
  if(!WebCore::mainView) {
    printf("!webview!\n");
    return;
  }

#ifdef __EMSCRIPTEN__
	if (quit) emscripten_cancel_main_loop();
#endif
	while (SDL_PollEvent(&e) != 0)
	{

    //WebCore::mainView->handleSDLEvent(e);
  /*  
  resize(width + 1, height + 1);
  scrollBy(1,1);
  scalefactor(2);
  setTransparent(false);
  setHtml(std::string(R"raw(<html><body style="background-color:blue;height:100%;width:100%"></body></html>)raw").c_str());*/
  //WebCore::mainView = new WebCore::WebView(window, contextDisplay, width, height, true);
 
 
 
  //////// eglSwapBuffers(contextDisplay, glSurface);



/*
		//Update screen
		SDL_GL_SwapWindow(window);


  resize(width + 1, height + 1);
  scrollBy(1,1);
  scalefactor(2);
  setTransparent(false);
  setHtml(std::string(R"raw(<html><body style="background-color:blue;height:100%;width:100%"></body></html>)raw").c_str());
  //WebCore::mainView = new WebCore::WebView(window, contextDisplay, width, height, true);*/

		if (e.type == SDL_QUIT)
		{
			quit = true;
		}

		else if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
		{
			//Get mouse position
			int x, y;
			SDL_GetMouseState(&x, &y);
			//std::cout << "x = " << x << " y = " << y << " type = " << e.type << std::endl;
		}

		else if (e.type == SDL_KEYDOWN)
		{
			//Select surfaces based on key press
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
				zoom+= 1.0 / 100.0;
				break;
			case SDLK_DOWN:
				zoom -= 1.0 / 100.0;
				break;
			default:
				break;
			}
		}
	}
	//Render

#ifdef DRAW_TEST
	Draw();
#endif // DRAW_TEST
}

/*void emulateGLUperspective(GLfloat fovY, GLfloat aspect, GLfloat zNear,
                           GLfloat zFar)
{
    GLfloat fW, fH;
    fH = tan(fovY / 180 * M_PI) * zNear / 2;
    fW = fH * aspect;
    glFrustumf(-fW, fW, -fH, fH, zNear, zFar);
}*/

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

#ifdef __EMSCRIPTEN__
	emscripten_set_canvas_size(width, height); // using SDL_SetVideoMode 
#endif

	window = SDL_CreateWindow("sdl_gl_read", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

	if (!window) {
		printf("Unable to create window: %s\n", SDL_GetError());
		return 1;
	}

#ifdef __EMSCRIPTEN__
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
  // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  // EGLDisplay glDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	//contextDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);//
  contextDisplay = (unsigned *)62000;//SDL_GL_CreateContext(window); //(unsigned *)62000; //SDL_GL_CreateContext(window);
	if (!contextDisplay) {
		printf("Unable to create contextDisplay: %s\n", SDL_GetError());
		return 1;
	}

  // https://github.com/emscripten-core/emscripten/blob/aae300219122ab62a50f2bcfeca5ba0be2e1040d/site/source/docs/porting/multimedia_and_graphics/EGL-Support-in-Emscripten.rst
  // https://github.com/emscripten-core/emscripten/blob/5187ea263120ff57424a184c1bc73259e24dc9a9/tests/test_egl.c
  // https://gist.github.com/EXL/1aaa9d273652addd5b33f9504c8180c7
  EGLint major = 2, minor = 1;
  EGLBoolean ret = eglInitialize(contextDisplay, &major, &minor);
  assert(eglGetError() == EGL_SUCCESS);
  assert(ret == EGL_TRUE);
  assert(major * 10000 + minor >= 10004);

    ret = eglGetConfigs(contextDisplay, NULL, 0, &numConfigs);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);

    EGLint egl_config_attr[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_BUFFER_SIZE, 32,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SAMPLES, 1,
        EGL_SAMPLE_BUFFERS, 2,
        EGL_NONE
    };
    //EGLint egl_config_attr[] = {
    //    EGL_BUFFER_SIZE,                16,
    //    EGL_DEPTH_SIZE,                 16,
    //    EGL_STENCIL_SIZE,                0,
    //    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    //    EGL_NONE
    //};
    eglChooseConfig(contextDisplay, egl_config_attr, &glConfig, 1, &numConfigs);
    /*SDL_SysWMinfo sysInfo;
    SDL_VERSION(&sysInfo.version); // Set SDL version
    if ( !SDL_GetWindowWMInfo(window, &sysInfo) ) { 
      printf("Unable to SDL_GetWindowWMInfo: %s\n", SDL_GetError());
      return 1;
    }*/

    EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    glContext = eglCreateContext(contextDisplay, glConfig, NULL, contextAttribs);
    if (glContext == EGL_NO_CONTEXT) {
        printf("Could not create EGL context\n");
    }

    assert(eglGetCurrentContext() == 0); // Creating a contextDisplay does not yet activate it.
    assert(eglGetError() == EGL_SUCCESS);

    // EGLNativeWindowType dummyWindow;
    /*glSurface = eglCreateWindowSurface(contextDisplay, glConfig,
                                          dummyWindow, 0);*/
    /*glSurface = eglCreateWindowSurface(contextDisplay, glConfig,
                                       (EGLNativeWindowType)sysInfo.info.x11.window, 0); // X11?*/
    // https://github.com/spurious/SDL-mirror/blob/7a03352cafcc855499bc459e1e58a446e38e7236/src/video/emscripten/SDL_emscriptenvideo.c#L239
    glSurface =  eglCreateWindowSurface(contextDisplay, glConfig, 0, 0);

    // https://github.com/juj/SDL-emscripten/blob/master/src/video/SDL_egl.c#L746
    ret = eglMakeCurrent(contextDisplay, glSurface, glSurface, glContext);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);

    eglSwapInterval(contextDisplay, 1);
    assert(eglGetError() == EGL_SUCCESS);
    assert(ret == EGL_TRUE);

    SDL_GL_MakeCurrent(window, contextDisplay);
    printf("SDL_GL_MakeCurrent...%s\n", SDL_GetError());

    EGLint ewidth, eheight;
    eglQuerySurface(contextDisplay, glSurface, EGL_WIDTH, &ewidth);
    eglQuerySurface(contextDisplay, glSurface, EGL_HEIGHT, &eheight);

    printf("eglQuerySurface (%d, %d)\n", ewidth, eheight);

    eglSwapInterval(contextDisplay, 1);
    glClearColor(1.0f, 0.0f, 0.0f, 0.5f);
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glViewport(0, 0, ewidth, eheight);
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //emulateGLUperspective(45.0f, (float) ewidth / (float) eheight, 0.1f, 100.0f);
    glViewport(0, 0, ewidth, eheight);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

  // SDL_Surface* sdl_screen

#ifndef __EMSCRIPTEN__
	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
	}
#endif

	//Use Vsync
	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}

#ifdef DRAW_TEST
	Init();


#ifdef CAIRO_TEST
    // TODO: surface creation error handling
    // https://github.com/acomminos/scrap-engine/blob/f9984bcf67fd83644d93e385215a3d741dfcc2ec/src/scrap/gl/cairo_renderer.cc
    WebView::cairo_surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    WebView::cairo_context_ = cairo_create(WebView::cairo_surface_);
/*
  // https://github.com/cubicool/cairo-gl-sdl2/blob/master/src/common/SDL2.hpp
  cairo_device = cairo_egl_device_create(
		(unsigned *)62000, //window.getDisplay(),
		reinterpret_cast<EGLContext>(contextDisplay)
	);
	if(!cairo_device) {
		std::cout << "Couldn't create device; fatal." << std::endl;
    return 3;
	}

	glGenTextures(1, &cairo_texture);
	glBindTexture(GL_TEXTURE_2D, cairo_texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_BGRA_EXT,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	cairo_surface = cairo_gl_surface_create_for_texture(
		cairo_device,
		CAIRO_CONTENT_COLOR_ALPHA,
		cairo_texture,
		width,
		height
	);

	if(!cairo_surface) {
		std::cerr << "Couldn't create surface; fatal." << std::endl;

		return 4;
	}*/
#endif // CAIRO_TEST

#endif // DRAW_TEST



	printf("createWebKit... \n");
  createWebKit(window, contextDisplay, width, height, true);

  /*
  //WebView->setViewWindow(scr);
  //setHtml(std::string("<html><body style='background-color:red;height:500px;width:500px'></body></html>").c_str());
  resize(width + 1, height + 1);
  scrollBy(1,1);
  scalefactor(2);
  setTransparent(false);
  setHtml(std::string(R"raw(<html><body style="background-color:blue;height:100%;width:100%"></body></html>)raw").c_str());
  //WebCore::mainView = new WebCore::WebView(window, contextDisplay, width, height, true);
  eglSwapBuffers(contextDisplay, glSurface);
  */

#ifdef __EMSCRIPTEN__
	printf("emscripten_set_main_loop... \n");
	emscripten_set_main_loop(tick, 0, 1);
#else
	printf("main_loop... \n");
	while (!quit) 
	{
		tick();
	}
#endif

#ifdef CAIRO_TEST
	cairo_surface_destroy(WebView::cairo_surface_);
  glDeleteTextures(1, &cairo_texture);
  glDeleteBuffers(1, &vertexPosObject);
  cairo_destroy(WebView::cairo_context_);
#endif

	SDL_DestroyWindow(window);
	window = NULL;

	//Quit SDL subsystems
	SDL_Quit();
	return 0;
}