#include "config.h"

#if USE(ACCELERATED_COMPOSITING)
#include "AcceleratedContext.h"
#include "CairoUtilities.h"
#include "DebuggerJS.h"
#include "FrameView.h"
#include "MainFrame.h"
#include "PlatformContextCairo.h"
#include "Settings.h"
#if USE(TEXTURE_MAPPER) && USE(TEXTURE_MAPPER_GL)
#include "TextureMapperGL.h"
#include "TextureMapperLayer.h"
#include "GraphicsLayerTextureMapper.h"
#endif
#include "GraphicsLayerClient.h"
#include "WebView.h"

#include <cairo.h>
#include <emscripten.h>
#include <wtf/CurrentTime.h>
#include "GL/glew.h"

// There seems to be a delicate balance between the main loop being flooded
// with motion events (that force flushes) and starving the main loop of events
// with flush callbacks. This delay is entirely empirical.
const double framesPerSecond = 60.0;
const double scheduleDelay = (1.0 / framesPerSecond);

namespace WebCore {

	AcceleratedContext::AcceleratedContext(WebView* webView)
		: m_view(webView)
		, m_layerFlushTimerCallbackId(0)
		, m_lastFlushTime(0)
		, m_redrawPendingTime(0)
		, m_needsExtraFlush(false)
	{
		webkitTrace();
    /// >>>
    //initialize();
	}

	void redirectedWindowDamagedCallback(void* data)
	{
		webkitTrace();
	}

	void AcceleratedContext::initialize()
	{
    printf("AcceleratedContext::initialize 1 ....\n");

		webkitTrace();
    if (m_rootLayer) {
      printf("AcceleratedContext::initialize m_rootLayer already exists\n");
      webkitTrace();
      /////return;
      return;
    }

		IntSize pageSize = roundedIntSize(m_view->positionAndSize().size());

		clearEverywhere();
		m_rootLayer = GraphicsLayer::create(0, this);
    if (!m_rootLayer) {
      printf("AcceleratedContext::initialize !m_rootLayer !!!!!!!!!!\n");
		  webkitTrace();
			return;
    }

		m_rootLayer->setDrawsContent(false);
		m_rootLayer->setMasksToBounds(false);
		m_rootLayer->setSize(FloatSize(pageSize.width(),pageSize.height()));

		webkitTrace();
    if (m_nonCompositedContentLayer) {
      printf("m_nonCompositedContentLayer already exists\n");
      webkitTrace();
      /////return;
      return;
    }

		// The non-composited contents are a child of the root layer.
		m_nonCompositedContentLayer = GraphicsLayer::create(0, this);
    if (!m_nonCompositedContentLayer) {
      printf("AcceleratedContext::initialize !m_nonCompositedContentLayer !!!!!!!!!!\n");
		  webkitTrace();
			return;
    }

		m_nonCompositedContentLayer->setDrawsContent(true);
		m_nonCompositedContentLayer->setContentsOpaque(false);
		m_nonCompositedContentLayer->setSize(pageSize);

		if (m_view->m_private->corePage->settings().acceleratedDrawingEnabled())
			m_nonCompositedContentLayer->setAcceleratesDrawing(true);

#ifndef NDEBUG
		m_rootLayer->setName("Root layer");
		m_nonCompositedContentLayer->setName("Non-composited content");
#endif

		m_rootLayer->addChild(m_nonCompositedContentLayer.get());
		m_nonCompositedContentLayer->setNeedsDisplay();

    if(!m_view->window_) {
      printf("!m_view->window_\n");
		  webkitTrace();
    }

		// The creation of the TextureMapper needs an active OpenGL context.
		GLContext* context = m_view->glWindowContext(m_view->window_);

    if(!context) {
      printf("!m_view->glWindowContext\n");
		  webkitTrace();
    }


		webkitTrace();
		context->makeContextCurrent();
		webkitTrace();

// FIX IT: TextureMapper::create ERROR: To use dlopen, you need to use Emscripten's linking support

    if (!context) {
        printf("INVALID  m_view->glWindowContext(m_view->window_)!!!\n");
		    webkitTrace();
        return;
    }

    // The creation of the TextureMapper needs an active OpenGL context.
    context->makeContextCurrent();

    printf("AcceleratedContext::initialize 2...\n");

    if(!m_rootLayer) {
      printf("AcceleratedContext::initialize 2 !m_rootLayer()\n");
		  webkitTrace();
    }
    
#if USE(TEXTURE_MAPPER) && USE(TEXTURE_MAPPER_GL)
		webkitTrace();
      printf("AcceleratedContext m_textureMapper = TextureMapper::create 1\n");
		m_textureMapper = TextureMapper::create(TextureMapper::OpenGLMode);
      printf("AcceleratedContext m_textureMapper = TextureMapper::create 2\n");
		webkitTrace();
    //m_textureMapper = TextureMapper::create(TextureMapper::SoftwareMode);
    if (!m_textureMapper) {
      printf("TextureMapper::create !m_textureMapper!!!!!!!!!!!!!!!!!!!!!!\n");
		  webkitTrace();
			return;
    }
		webkitTrace();
		static_cast<TextureMapperGL*>(m_textureMapper.get())->setEnableEdgeDistanceAntialiasing(true);
		webkitTrace();
		toTextureMapperLayer(m_rootLayer.get())->setTextureMapper(m_textureMapper.get());
#endif

		webkitTrace();
		scheduleLayerFlush();
		webkitTrace();

    printf("AcceleratedContext::initialize 3 ....\n");
	}

	AcceleratedContext::~AcceleratedContext()
	{
		webkitTrace();
		stopAnyPendingLayerFlush();
	}

	void AcceleratedContext::stopAnyPendingLayerFlush()
	{
		webkitTrace();
	}

	bool AcceleratedContext::enabled()
	{
    printf("AcceleratedContext::enabled() 1 ....\n");

		webkitTrace();
    if(!m_rootLayer) {
      printf("AcceleratedContext::enabled !m_rootLayer()\n");
		  webkitTrace();
    }
    if(!m_textureMapper) {
      printf("AcceleratedContext::enabled !m_textureMapper()\n");
		  webkitTrace();
    }

    printf("AcceleratedContext::enabled() 2 ....\n");

#if USE(TEXTURE_MAPPER) && USE(TEXTURE_MAPPER_GL)
		return m_rootLayer && m_textureMapper;
#else
		return m_rootLayer != nullptr;
#endif
	}

	GLContext* AcceleratedContext::prepareForRendering()
	{
    printf("AcceleratedContext::prepareForRendering 1 ....\n");
		webkitTrace();
    if(!enabled()) {
      printf("prepareForRendering !enabled()\n");
		  webkitTrace();
			///////////////////////////return NULL;
      // return NULL;
    }

    printf("AcceleratedContext::prepareForRendering 2 ....\n");

		GLContext* context = GLContext::getCurrent();
		webkitTrace();

		if (!context || !context->makeContextCurrent())	{
    //	if (!context) {
      printf("!context makeContextCurrent\n");
		  webkitTrace();
			/////////////////////return NULL;
      // return NULL;
    }

    printf("AcceleratedContext::prepareForRendering 3 ....\n");
		return context;
	}

	void AcceleratedContext::compositeLayersToContext(CompositePurpose purpose)
	{
    printf("compositeLayersToContext....\n");
		webkitTrace();
		GLContext* context = prepareForRendering();

		if (!context) {
      printf("compositeLayersToContext !context\n");
		  webkitTrace();
			return;
    }

		IntSize windowSize = roundedIntSize(m_view->positionAndSize().size());
		glViewport(0, 0, windowSize.width(), windowSize.height());

		if (purpose == ForResize) {
		//glClearColor(1, 1, 1, 0);
			glClearColor(1.0, 0.5, 1.0, 0.5);
			glClear(GL_COLOR_BUFFER_BIT);
		}

#if USE(TEXTURE_MAPPER) && USE(TEXTURE_MAPPER_GL)
    if(!m_textureMapper) {
      printf("AcceleratedContext::compositeLayersToContext !m_textureMapper\n");
		  webkitTrace();
    }
		if (!m_rootLayer) {
      printf("AcceleratedContext::compositeLayersToContext !m_rootLayer\n");
		  webkitTrace();
			return;
    }
		m_textureMapper->beginPainting();
		toTextureMapperLayer(m_rootLayer.get())->paint();
		m_textureMapper->endPainting();
#else
		// TODO: Composite layers together, without texutre mapping this is an
		// unknown.
#endif

  // >>>>>>>
		context->swapBuffers();

#if USE(TEXTURE_MAPPER) && USE(TEXTURE_MAPPER_GL)
    printf("layerFlushTimerFired....\n");
    webkitTrace();
		// Without this call animations will not render, check to see if we have any existing
		// (pending) animations and callback a render every 1/60th a second.
		if(toTextureMapperLayer(m_rootLayer.get())->descendantsOrSelfHaveRunningAnimations()) {
		  webkitTrace();
			m_layerFlushTimerCallbackId = 1;
      printf("layerFlushTimerFired scheduleDelay %d\n", scheduleDelay);
			emscripten_async_call(&layerFlushTimerFiredCallback, this, scheduleDelay * 1000.0);
      //layerFlushTimerFiredCallback(this);
			//emscripten_async_call(&layerFlushTimerFiredCallback, this, 5000.0);
		}
#else
		// TODO: Determine if there's a running animation and call back for a composite.
#endif
	}

	void AcceleratedContext::clearEverywhere()
	{
		webkitTrace();

		GLContext* context = prepareForRendering();

		if (!context) {
      printf("clearEverywhere !context()\n");
		  webkitTrace();
			return;
    }

		IntSize windowSize = roundedIntSize(m_view->positionAndSize().size());
		glViewport(0, 0, windowSize.width(), windowSize.height());
		//glClearColor(1, 1, 1, 0);
			glClearColor(0.0, 0.5, 1.0, 0.5);
		glClear(GL_COLOR_BUFFER_BIT);

  // >>>>>>>
		context->swapBuffers();
	}

	void AcceleratedContext::setRootCompositingLayer(GraphicsLayer* graphicsLayer)
	{
    printf("AcceleratedContext::setRootCompositingLayer 1....\n");
    printf("AcceleratedContext::setRootCompositingLayer !graphicsLayer\n");
    
		webkitTrace();
		// Clearing everywhere when turning on or off the layer tree prevents us from flashing
		// old content before the first flush.
		clearEverywhere();

		if (!graphicsLayer) {
      printf("AcceleratedContext::setRootCompositingLayer !graphicsLayer !!!!!!!!!!!!!!!!!!!\n");
		  webkitTrace();

			stopAnyPendingLayerFlush();

			// Shrink the offscreen window to save memory while accelerated compositing is turned off.
			m_rootLayer = nullptr;
			m_nonCompositedContentLayer = nullptr;
#if USE(TEXTURE_MAPPER) && USE(TEXTURE_MAPPER_GL)
			m_textureMapper = nullptr;
#endif
			return;
		}

    printf("AcceleratedContext::setRootCompositingLayer 2 ....\n");

		// Add the accelerated layer tree hierarchy.
		initialize();

		if (!m_nonCompositedContentLayer) {
      printf("AcceleratedContext::setRootCompositingLayer !m_nonCompositedContentLayer()\n");
		  webkitTrace();
			return;
    }

		m_nonCompositedContentLayer->removeAllChildren();
		m_nonCompositedContentLayer->addChild(graphicsLayer);

		stopAnyPendingLayerFlush();
		scheduleLayerFlush();
	}

	void AcceleratedContext::setNonCompositedContentsNeedDisplay(const IntRect& rect)
	{
		webkitTrace();

    printf("AcceleratedContext::setNonCompositedContentsNeedDisplay 1....\n");

		if (!m_rootLayer) {
      printf("AcceleratedContext::setNonCompositedContentsNeedDisplay !m_rootLayer()\n");
		  webkitTrace();
			return;
    }

		if (rect.isEmpty()) {
			m_rootLayer->setNeedsDisplay();
			return;
		}

    printf("AcceleratedContext::setNonCompositedContentsNeedDisplay 2....\n");

		m_nonCompositedContentLayer->setNeedsDisplayInRect(rect);
		scheduleLayerFlush();
	}

	void AcceleratedContext::resizeRootLayer(const IntSize& newSize)
	{
		webkitTrace();

		if (!enabled()) {
      printf("resizeRootLayer !enabled()\n");
		  webkitTrace();
			return;
    }

		if (!m_nonCompositedContentLayer) {
      printf("!m_nonCompositedContentLayer\n");
		  webkitTrace();
			return;
    }

		FloatSize oldSize = m_nonCompositedContentLayer->size();

		if (m_rootLayer->size() == newSize) {
			return;
    }

		// If the newSize exposes new areas of the non-composited content a setNeedsDisplay is needed
		// for those newly exposed areas.
		m_rootLayer->setSize(newSize);
		m_nonCompositedContentLayer->setSize(newSize);

		if (newSize.width() > oldSize.width()) {
			float height = std::min(static_cast<float>(newSize.height()), oldSize.height());
			m_nonCompositedContentLayer->setNeedsDisplayInRect(FloatRect(oldSize.width(), 0, newSize.width() - oldSize.width(), height));
		}

		if (newSize.height() > oldSize.height())
			m_nonCompositedContentLayer->setNeedsDisplayInRect(FloatRect(0, oldSize.height(), newSize.width(), newSize.height() - oldSize.height()));

		m_nonCompositedContentLayer->setNeedsDisplayInRect(IntRect(IntPoint(), newSize));
		compositeLayersToContext(ForResize);
		scheduleLayerFlush();
	}

	void AcceleratedContext::scrollNonCompositedContents(const IntRect& scrollRect, const IntSize& scrollOffset)
	{
		webkitTrace();
		if (!m_nonCompositedContentLayer) {
      printf("!m_nonCompositedContentLayer\n");
		  webkitTrace();
			return;
    }
		m_nonCompositedContentLayer->setNeedsDisplayInRect(scrollRect);
		scheduleLayerFlush();
	}

	void AcceleratedContext::layerFlushTimerFiredCallback(void* context)
	{
		webkitTrace();
		AcceleratedContext *contexts = (AcceleratedContext *)context;
    if (!contexts) {
      printf("layerFlushTimerFiredCallback !contexts\n");
      webkitTrace();
      return;
    }
		contexts->layerFlushTimerFired();
	}

	void AcceleratedContext::scheduleLayerFlush()
	{
		webkitTrace();

    if (!enabled()) {
      printf("scheduleLayerFlush !enabled\n");
      webkitTrace();
      ///////////return;
      //return;
    }

		if (m_layerFlushTimerCallbackId) {
      printf("scheduleLayerFlush m_layerFlushTimerCallbackId\n");
      webkitTrace();
			/////////return;
      return;
    }

		m_layerFlushTimerCallbackId = 1;
		double nextFlush = std::max(scheduleDelay - (currentTime() - m_lastFlushTime), 1.0);
    printf("nextFlush in %f", nextFlush);
		emscripten_async_call(&layerFlushTimerFiredCallback, this, nextFlush * 5000.0);
    //emscripten_async_call(&layerFlushTimerFiredCallback, this, 5000.0);
    //layerFlushTimerFiredCallback(this);
	}

	bool AcceleratedContext::flushPendingLayerChanges()
	{
		webkitTrace();


    if (!enabled()) {
      printf("flushPendingLayerChanges !enabled\n");
      webkitTrace();
      return false;
    }

    if (!m_rootLayer) {
      printf("AcceleratedContext::flushPendingLayerChanges !m_rootLayer\n");
      webkitTrace();
      return false;
    }

		m_rootLayer->flushCompositingStateForThisLayerOnly();
		m_nonCompositedContentLayer->flushCompositingStateForThisLayerOnly();
		return m_view->p()->mainFrame->coreFrame()->view()->flushCompositingStateIncludingSubframes();
	}

	void AcceleratedContext::flushAndRenderLayers()
	{
		webkitTrace();

		if (!enabled()) {
      printf("flushAndRenderLayers 1 !enabled\n");
		  webkitTrace();
			return;
    }

		Frame* frame = m_view->m_private->mainFrame->coreFrame();

		ASSERT(frame->isMainFrame());

		if (!frame->contentRenderer() || !frame->view()) {
      printf("!frame->contentRenderer\n");
		  webkitTrace();
			return;
    }

		frame->view()->updateLayoutAndStyleIfNeededRecursive();

		if (!enabled()) {
      printf("flushAndRenderLayers 2 !enabled\n");
		  webkitTrace();
			return;
    }

		GLContext* context = GLContext::getCurrent();
		webkitTrace();
	
		if (!context) {
      printf("flushAndRenderLayers !context\n");
		  webkitTrace();
			return;
    }
	
		if (context && !context->makeContextCurrent()) {
      printf("flushAndRenderLayers !context->makeContextCurrent()\n");
		  webkitTrace();
			return;
    }

		if (!flushPendingLayerChanges()) {
			return;
    }

		m_lastFlushTime = currentTime();
		compositeLayersToContext();

		if (!m_redrawPendingTime)
			m_redrawPendingTime = currentTime();
	}

	void AcceleratedContext::layerFlushTimerFired()
	{
		webkitTrace();
		if(!m_layerFlushTimerCallbackId) {
			return;
    }
		m_layerFlushTimerCallbackId = 0;
		flushAndRenderLayers();
	}

	void AcceleratedContext::notifyAnimationStarted(const GraphicsLayer*, double time) {
		notImplemented();
	}

	void AcceleratedContext::notifyFlushRequired(const GraphicsLayer*) {
		webkitTrace();
		scheduleLayerFlush();
	}

	void AcceleratedContext::paintContents(const GraphicsLayer* layer, GraphicsContext& context, GraphicsLayerPaintingPhase phase, const IntRect& rectToPaint)
	{
		webkitTrace();
		Frame& frame = (m_view->m_private->mainFrame->coreFrame()->mainFrame());
		ASSERT(frame.isMainFrame());
		context.save();
		context.clip(rectToPaint);
		if(m_view->m_private->transparent)
			context.clearRect(rectToPaint);
		frame.view()->paint(&context, rectToPaint);
		context.restore();
	}
}
#endif