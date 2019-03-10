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

/*
	Draws on screen using opengl 3 in PC or openglES 2 in browser.
	Precompile command : "D:\exarion\vendor\emscripten\SDK\emscripten\1.35.0\emcc" --clear-cache
	Compile command : "D:\exarion\vendor\emscripten\SDK\emscripten\1.35.0\emcc" Source.cpp -s USE_SDL=2 -s FULL_ES2=1 -o test.html -O3 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s SDL2_IMAGE_FORMATS="['png']" -s EXPORTED_FUNCTIONS="['_main', '_mainLoop']" -std=c++11 -Werror -s WARN_ON_UNDEFINED_SYMBOLS=1 -s SIMD=1 -s NO_EXIT_RUNTIME=1 -s AGGRESSIVE_VARIABLE_ELIMINATION=1 -s SEPARATE_ASM=1
	Run generated page in server.
*/
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

namespace WebCore {
	static WebCore::WebView* mainView = NULL;
#ifdef DEBUG
	String logLevelString() { return getenv("WEBKIT_DEBUG"); }
#endif
}

static GLuint programObject;
static int width = 1024;
static int height = 768;

static GLint uniformZoom, uniformColor, uniformMVP;
static float zoom = 1.0;

static SDL_Window *window;
static SDL_GLContext context;

//Main loop flag
static bool quit = false;

//Event handler
static SDL_Event e;

static cairo_device_t* cairo_device;

static cairo_surface_t* cairo_surface;

static GLuint cairo_texture = 0;

bool makeCurrent() {
  SDL_GL_MakeCurrent(window, nullptr);

  return !SDL_GL_MakeCurrent(window, context);
}

bool makeCairoCurrent() {
  SDL_GL_MakeCurrent(window, nullptr);

  return !SDL_GL_MakeCurrent(window, context);
}

inline void cairo_draw(cairo_surface_t* surface) {
	static double s = 1.0;

	cairo_t* cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
	cairo_paint(cr);
	cairo_translate(cr, width / 2, height / 2);
	cairo_scale(cr, s, s);
	cairo_arc(cr, 0.0, 0.0, width / 4, 0.0, 2.0 * 3.14159);
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	cairo_fill(cr);
	cairo_surface_flush(surface);
	cairo_destroy(cr);

	s += 1.0 / 180.0;

	if(s >= 2.0) s = 1.0;
}

//The application time based timer
class LTimer
{
public:
	//Initializes variables
	LTimer();

	//The various clock actions
	void start();
	void stop();
	void pause();
	void unpause();

	//Gets the timer's time
	Uint32 getTicks();

	//Checks the status of the timer
	bool isStarted();
	bool isPaused();

private:
	//The clock time when the timer started
	Uint32 mStartTicks;

	//The ticks stored when the timer was paused
	Uint32 mPausedTicks;

	//The timer status
	bool mPaused;
	bool mStarted;
};

LTimer::LTimer()
{
	//Initialize the variables
	mStartTicks = 0;
	mPausedTicks = 0;

	mPaused = false;
	mStarted = false;
}

void LTimer::start()
{
	//Start the timer
	mStarted = true;

	//Unpause the timer
	mPaused = false;

	//Get the current clock time
	mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop()
{
	//Stop the timer
	mStarted = false;

	//Unpause the timer
	mPaused = false;

	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause()
{
	//If the timer is running and isn't already paused
	if (mStarted && !mPaused)
	{
		//Pause the timer
		mPaused = true;

		//Calculate the paused ticks
		mPausedTicks = SDL_GetTicks() - mStartTicks;
		mStartTicks = 0;
	}
}

void LTimer::unpause()
{
	//If the timer is running and paused
	if (mStarted && mPaused)
	{
		//Unpause the timer
		mPaused = false;

		//Reset the starting ticks
		mStartTicks = SDL_GetTicks() - mPausedTicks;

		//Reset the paused ticks
		mPausedTicks = 0;
	}
}

Uint32 LTimer::getTicks()
{
	//The actual timer time
	Uint32 time = 0;

	//If the timer is running
	if (mStarted)
	{
		//If the timer is paused
		if (mPaused)
		{
			//Return the number of ticks when the timer was paused
			time = mPausedTicks;
		}
		else
		{
			//Return the current time minus the start time
			time = SDL_GetTicks() - mStartTicks;
		}
	}

	return time;
}

bool LTimer::isStarted()
{
	//Timer is running and paused or unpaused
	return mStarted;
}

bool LTimer::isPaused()
{
	//Timer is running and paused
	return mPaused && mStarted;
}

//The frames per second timer
LTimer fpsTimer;

//Start counting frames per second
int countedFrames = 0;

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
		"attribute vec4 vPosition;                \n"
//		"attribute vec4 vTexCoord;                \n"
		"uniform mat4 uMVPMatrix; \n"
		"uniform float zoom;	\n"
		"void main()                              \n"
		"{                                        \n"
		"   gl_Position = uMVPMatrix * vPosition;              \n"
		"   gl_Position.x = gl_Position.x * zoom; \n"
		"   gl_Position.y = gl_Position.y * zoom; \n"
		"}                                        \n";

	char fShaderStr[] =
		"precision mediump float;\n"
		"uniform vec4 vColor;"
		"void main()                                  \n"
		"{                                            \n"
		"  gl_FragColor = vColor;        \n"
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
	glLinkProgram(programObject);
	uniformColor = glGetUniformLocation(programObject, "vColor");
	uniformZoom = glGetUniformLocation(programObject, "zoom");
	uniformMVP = glGetUniformLocation(programObject, "uMVPMatrix");
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	fpsTimer.start();

	return GL_TRUE;
}

///
// Draw using the shader pair created in Init()
//
void Draw()
{

  if(makeCairoCurrent()) {
    cairo_draw(cairo_surface);

    cairo_gl_surface_swapbuffers(cairo_surface);
  } else {
    printf("!makeCairoCurrent \n");
  }

  if(makeCurrent()) {
    int x = 0;
    int y = 0;

    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, cairo_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(GL_QUADS);

    // Bottom-Left
    glTexCoord2i(0, 1);
    glVertex2i(x, y);

    // Upper-Left
    glTexCoord2i(0, 0);
    glVertex2i(x, y + height);

    // Upper-Right
    glTexCoord2i(1, 0);
    glVertex2i(x + width, y + height);

    // Bottom-Right
    glTexCoord2i(1, 1);
    glVertex2i(x + width, y);

    glEnd();*/
  } else {
    printf("!makeCurrent \n");
  }

  // uncomment if you want to draw triangle >>>

	/*GLfloat vVertices[] = { 0.0f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f };
	// No clientside arrays, so do this in a webgl-friendly manner
	GLuint vertexPosObject;
	glGenBuffers(1, &vertexPosObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	glBufferData(GL_ARRAY_BUFFER, 9 * 4, vVertices, GL_STATIC_DRAW);
	
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexPosObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(0);

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
	glUniformMatrix4fv(uniformMVP, 1, 0, mat);

	glUniform1f(uniformZoom, zoom);
	float color[] = { 0.2f, 0.1f, 0.51f, 1.0f };
	glUniform4fv(uniformColor, 1, color);

	glDrawArrays(GL_TRIANGLES, 0, 3);*/
}

void tick() {
	//Calculate and correct fps
	float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);
	if (avgFPS > 2000000)
	{
		avgFPS = 0;
	}
#ifdef __EMSCRIPTEN__
	if (quit) emscripten_cancel_main_loop();
#endif
	while (SDL_PollEvent(&e) != 0)
	{

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
	Draw();
	//std::cout << " avgFPS = " << avgFPS << std::endl;
	++countedFrames;
}

extern "C" {
	void setHtml(char *html) { WebCore::mainView->setHtml(html,strlen(html)); }
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

	printf("createWebKit... \n");
  createWebKit(window, context, width, height, false);
  setHtml("<html><body style='background-color:red'>iiii</body></html>");
  
  //WebCore::mainView = new WebCore::WebView(window, context, width, height, /*accel*/ true);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(tick, 0, 1);
#else
	while (!quit) 
	{
		tick();
	}
#endif

  // TODO
	//cairo_surface_destroy(surface);
	//cairo_device_destroy(device);

	SDL_DestroyWindow(window);
	window = NULL;

	//Quit SDL subsystems
	SDL_Quit();
	return 0;
}



#ifdef NOPE

namespace WebCore {
	static WebCore::WebView* mainView = NULL;
#ifdef DEBUG
	String logLevelString() { return getenv("WEBKIT_DEBUG"); }
#endif
}


void tick() {
	if(!WebCore::mainView) {
    return;
  }
	// Note: do not add a  here, this funciton executes quite
	// a few times that it will cause the browser to slow down to a crawl
	// to finish writing console messages.
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		case SDL_QUIT:
			exit(1);
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		default:
			//WebKit::mainView->handleSDLEvent(event);
			break;
	}
}
extern "C" {
	void setHtml(char *html) { WebCore::mainView->setHtml(html,strlen(html)); }
	void setTransparent(bool transparent) { WebCore::mainView->setTransparent(transparent); }
	void scrollBy(int x, int y) { WebCore::mainView->scrollBy(x,y); }
	void resize(int w, int h) { WebCore::mainView->resize(w,h); }
	void scalefactor(float sf) { WebCore::mainView->scalefactor(sf); }
	void createWebKit(int width, int height, bool accel) { WebCore::mainView = new WebCore::WebView(width, height, accel); }
	int main(int argc, char** argv) {
		createWebKit(500,500,true);
		emscripten_set_main_loop(&tick, 0, false);
	}
}
#endif