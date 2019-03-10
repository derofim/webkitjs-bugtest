#include "WebView.h"
#include "IntSize.h"
#include <emscripten.h>

#ifdef DEBUG
#include "Logging.h"
#include <wtf/text/WTFString.h>
#endif

#include "cairo.h"
#include "cairo-gl.h"
#include "PlatformContextCairo.h"

#include "config.h"
#include "SDL.h"

#include <emscripten.h>
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
static SDL_GLContext context;

//Main loop flag
static bool quit = false;

//Event handler
static SDL_Event e;

static GLuint vertexPosObject;

//static cairo_device_t* cairo_device;

/*static cairo_surface_t* cairo_surface;

static cairo_t *cairo_context;*/

static GLuint cairo_texture = 0;

bool makeCurrent() {
  SDL_GL_MakeCurrent(window, nullptr);

  return !SDL_GL_MakeCurrent(window, context);
}

bool makeCairoCurrent() {
  SDL_GL_MakeCurrent(window, nullptr);

  return !SDL_GL_MakeCurrent(window, context);
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

    glGenTextures(1, &cairo_texture);

	// No clientside arrays, so do this in a webgl-friendly manner
	glGenBuffers(1, &vertexPosObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertexData), kVertexData,
                 GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	return GL_TRUE;
}

#define DRAW_TEST

#ifdef DRAW_TEST
///
// Draw using the shader pair created in Init()
//
void Draw()
{

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
}
#endif // DRAW_TEST

void tick() {
  if(!WebCore::mainView) return;

#ifdef __EMSCRIPTEN__
	if (quit) emscripten_cancel_main_loop();
#endif
	while (SDL_PollEvent(&e) != 0)
	{

    WebCore::mainView->handleSDLEvent(e);

		//Update screen
		SDL_GL_SwapWindow(window);
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

extern "C" {
	void setHtml(const char *html) { WebCore::mainView->setHtml(html,strlen(html)); }
	void setTransparent(bool transparent) { WebCore::mainView->setTransparent(transparent); }
	void scrollBy(int x, int y) { WebCore::mainView->scrollBy(x,y); }
	void resize(int w, int h) { WebCore::mainView->resize(w,h); }
	void scalefactor(float sf) { WebCore::mainView->scalefactor(sf); }
	void createWebKit(SDL_Window *window, SDL_GLContext& context, int width, int height, bool accel) { WebCore::mainView = new WebCore::WebView(window, context, width, height, accel); }
}

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
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	context = SDL_GL_CreateContext(window);

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

	Init();

#ifdef DRAW_TEST

    // TODO: surface creation error handling
    // https://github.com/acomminos/scrap-engine/blob/f9984bcf67fd83644d93e385215a3d741dfcc2ec/src/scrap/gl/cairo_renderer.cc
    WebView::cairo_surface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    WebView::cairo_context_ = cairo_create(WebView::cairo_surface_);
/*
  // https://github.com/cubicool/cairo-gl-sdl2/blob/master/src/common/SDL2.hpp
  cairo_device = cairo_egl_device_create(
		(unsigned *)62000, //window.getDisplay(),
		reinterpret_cast<EGLContext>(context)
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
#endif // DRAW_TEST

	printf("createWebKit... \n");
  createWebKit(window, context, width, height, true);
  //setHtml(std::string("<html><body style='background-color:red;height:500px;width:500px'></body></html>").c_str());
  setHtml(std::string(R"raw(<html><body style='background-color:blue;height:100%;width:100%'></body></html>)raw").c_str());
  //WebCore::mainView = new WebCore::WebView(window, context, width, height, /*accel*/ true);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(tick, 0, 1);
#else
	while (!quit) 
	{
		tick();
	}
#endif

	cairo_surface_destroy(WebView::cairo_surface_);
  glDeleteTextures(1, &cairo_texture);
  glDeleteBuffers(1, &vertexPosObject);
  cairo_destroy(WebView::cairo_context_);

	SDL_DestroyWindow(window);
	window = NULL;

	//Quit SDL subsystems
	SDL_Quit();
	return 0;
}