/****************************************************************************
**This file is part of the Motorcar 3D windowing framework
**
**
**Copyright (C) 2015 Forrest Reiling
**
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#include <osvr/RenderKit/GraphicsLibraryOpenGL.h>
#include <osvr/RenderKit/RenderManagerOpenGLC.h>

#include <qt/qtwaylandmotorcaropenglcontext.h>
#include <QList>
#include <QWindow>
#include <QRect>
#include <QSurfaceFormat>

#include <vector>
#include <algorithm>

#include "osvr_hmd.h"

using namespace motorcar;
using namespace qtmotorcar;

class Qt5ToolkitImpl {
	OSVR_OpenGLToolkitFunctions toolkit;

	static void createImpl(void* data) {
	}
	static void destroyImpl(void* data) {
		delete ((Qt5ToolkitImpl*)data);
	}
	static OSVR_CBool addOpenGLContextImpl(void* data, const OSVR_OpenGLContextParams* p) {
		return ((Qt5ToolkitImpl*)data)->addOpenGLContext(p);
	}
	static OSVR_CBool removeOpenGLContextsImpl(void* data) {
		return ((Qt5ToolkitImpl*)data)->removeOpenGLContexts();
	}
	static OSVR_CBool makeCurrentImpl(void* data, size_t display) {
		return ((Qt5ToolkitImpl*)data)->makeCurrent(display);
	}
	static OSVR_CBool swapBuffersImpl(void* data, size_t display) {
		return ((Qt5ToolkitImpl*)data)->swapBuffers(display);
	}
	static OSVR_CBool setVerticalSyncImpl(void* data, OSVR_CBool verticalSync) {
		return ((Qt5ToolkitImpl*)data)->setVerticalSync(verticalSync);
	}
	static OSVR_CBool handleEventsImpl(void* data) {
		return ((Qt5ToolkitImpl*)data)->handleEvents();
	}
	static OSVR_CBool getDisplayFrameBufferImpl(void* data, size_t display, GLuint* displayFrameBufferOut) {
		return ((Qt5ToolkitImpl*)data)->getDisplayFrameBuffer(display, displayFrameBufferOut);
	}
	static OSVR_CBool getDisplaySizeOverrideImpl(void* data, size_t display, int* width, int* height) {
		return ((Qt5ToolkitImpl*)data)->getDisplaySizeOverride(display, width, height);
	}

	QList<QOpenGLWindow*> windows;
	QOpenGLWindow *m_window;

	public:
	Qt5ToolkitImpl(QOpenGLWindow *window) {
		m_window = window;

		memset(&toolkit, 0, sizeof(toolkit));
		toolkit.size = sizeof(toolkit);
		toolkit.data = this;
		toolkit.create = createImpl;
		toolkit.destroy = destroyImpl;
		toolkit.addOpenGLContext = addOpenGLContextImpl;
		toolkit.removeOpenGLContexts = removeOpenGLContextsImpl;
		toolkit.makeCurrent = makeCurrentImpl;
		toolkit.swapBuffers = swapBuffersImpl;
		toolkit.setVerticalSync = setVerticalSyncImpl;
		toolkit.handleEvents = handleEventsImpl;
		toolkit.getDisplaySizeOverride = getDisplaySizeOverrideImpl;
	}

	~Qt5ToolkitImpl() {
	}

	const OSVR_OpenGLToolkitFunctions* getToolkit() const { return &toolkit; }

	bool addOpenGLContext(const OSVR_OpenGLContextParams* p) {

		QRect screenGeometry(p->xPos, p->yPos, p->width, p->height);

		QSurfaceFormat format;
		format.setDepthBufferSize(8);
		format.setStencilBufferSize(8);
		format.setSwapInterval(1);
		format.setStencilBufferSize(8);

		QOpenGLWindow *window;

		if (windows.size()) {
			window = new QOpenGLWindow(m_window->context(), format, screenGeometry);
		} else {
			window = m_window;
			window->setGeometry(screenGeometry);
			window->setFormat(format);
		}

		window->hide();
		if (p->fullScreen) {
			window->showFullScreen();
		} else {
			window->showNormal();
		}

		windows.push_back(window);

		return true;
	}
	bool removeOpenGLContexts() {
		for (auto window : windows) {
			window->deleteLater();
		}
		windows.clear();

		return true;
	}
	bool makeCurrent(size_t display) {
		windows.at(static_cast<int>(display))->makeCurrent();
		return true;
	}
	bool swapBuffers(size_t display) {
		windows.at(static_cast<int>(display))->swapBuffers();
		return true;
	}
	bool setVerticalSync(bool verticalSync) {
		return true;
	}
	bool handleEvents() {
		return true;
	}
	bool getDisplayFrameBuffer(size_t display, GLuint* displayFrameBufferOut) {
		*displayFrameBufferOut = 0;
		return true;
	}
	bool getDisplaySizeOverride(size_t display, int* width, int* height) {
		// we don't override the display. Use default behavior.
		return false;
	}
};

OsvrHMD::OsvrHMD(Skeleton *skeleton, OpenGLContext *glContext, PhysicalNode *parent)
	: RenderToTextureDisplay(glContext, glm::vec2(0.126, 0.0706), parent, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.10f)))
	  , m_context("com.motorcar")
{
	m_interface = m_context.getInterface("/me/head");

	auto toolkit = new Qt5ToolkitImpl(reinterpret_cast<QtWaylandMotorcarOpenGLContext *>(glContext)->window());

	osvr::renderkit::GraphicsLibrary myLibrary;
	myLibrary.OpenGL = new osvr::renderkit::GraphicsLibraryOpenGL;
	myLibrary.OpenGL->toolkit = toolkit->getToolkit();

	m_renderManager = osvr::renderkit::createRenderManager(m_context.get(), "OpenGL", myLibrary);

	if ((m_renderManager == nullptr) || (!m_renderManager->doingOkay())) {
		std::cerr << "Could not create RenderManager" << std::endl;
	}

	// Open the display and make sure this worked.
	osvr::renderkit::RenderManager::OpenResults ret = m_renderManager->OpenDisplay();
	if (ret.status == osvr::renderkit::RenderManager::OpenStatus::FAILURE) {
		std::cerr << "Could not open display" << std::endl;
	}

	// Set up the rendering state we need.
	SetupRendering(ret.library);

	// Do a call to get the information we need to construct our
	// color and depth render-to-texture buffers.
	m_context.update();

	m_renderInfo = m_renderManager->GetRenderInfo();

	// Construct the buffers we're going to need for our render-to-texture
	// code.
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	int renderTargetWidth = 0,
		renderTargetHeight = 0;

	for (size_t i = 0; i < m_renderInfo.size(); i++) {
		GLuint colorBufferName = 0;
		glGenTextures(1, &colorBufferName);
		osvr::renderkit::RenderBuffer rb;
		rb.OpenGL = new osvr::renderkit::RenderBufferOpenGL;
		rb.OpenGL->colorBufferName = colorBufferName;

		colorBuffers.push_back(rb);

		// "Bind" the newly created texture : all future texture
		// functions will modify this texture glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBufferName);

		// Determine the appropriate size for the frame buffer to be used for
		// this eye.
		int width = static_cast<int>(m_renderInfo[i].viewport.width);
		int height = static_cast<int>(m_renderInfo[i].viewport.height);

		// Give an empty image to OpenGL ( the last "0" means "empty" )
		// Note that whether or not the second GL_RGBA is turned into
		// GL_BGRA, the first one should remain GL_RGBA -- it is specifying
		// the size.  If the second is changed to GL_RGB or GL_BGR, then
		// the first should become GL_RGB.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, 0);

		// Bilinear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// The depth buffer
		GLuint depthrenderbuffer;
		glGenRenderbuffers(1, &depthrenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width,
				height);
		depthBuffers.push_back(depthrenderbuffer);

		renderTargetWidth += width;
		renderTargetHeight = height > renderTargetHeight ? height : renderTargetHeight;
	}

	m_renderTargetSize = glm::ivec2(renderTargetWidth, renderTargetHeight);
	setRenderTargetSize(m_renderTargetSize);

	int offsetX = 0;
	for (size_t i = 0; i < m_renderInfo.size(); i++) {
		int width = static_cast<int>(m_renderInfo[i].viewport.width);
		int height = static_cast<int>(m_renderInfo[i].viewport.height);

		ViewPoint *vp = new ViewPoint(
				m_renderInfo[i].projection.nearClip,
				m_renderInfo[i].projection.farClip,
				this,
				this,
				glm::mat4(),
				glm::vec4(
					(offsetX / (float) renderTargetWidth),
					0,
					(width / (float) renderTargetWidth),
					(height / (float) renderTargetHeight)
					)
				);

		setViewpointProjection(vp, m_renderInfo[i].projection);
		addViewpoint(vp);

		offsetX += width;
	}

	// Register our constructed buffers so that we can use them for
	// presentation.
	if (!m_renderManager->RegisterRenderBuffers(colorBuffers)) {
		std::cerr << "RegisterRenderBuffers() returned false, cannot continue"
			<< std::endl;
	}

}

bool OsvrHMD::SetupRendering(osvr::renderkit::GraphicsLibrary library) {
	// Make sure our pointers are filled in correctly.  The config file selects
	// the graphics library to use, and may not match our needs.
	if (library.OpenGL == nullptr) {
		std::cerr << "SetupRendering: No OpenGL GraphicsLibrary, this should "
			"not happen"
			<< std::endl;
		return false;
	}

	osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

	// Turn on depth testing, so we get correct ordering.
	//glEnable(GL_DEPTH_TEST);

	return true;
}

glm::ivec2 OsvrHMD::size()
{
	return m_renderTargetSize;
}

OsvrHMD::~OsvrHMD()
{
}


void OsvrHMD::prepareForDraw()
{
	m_context.update();

	// Tidy this up
	OSVR_PoseState state;
	OSVR_TimeValue timestamp;
	OSVR_ReturnCode ret = osvrGetPoseState(m_interface.get(), &timestamp, &state);
	if (OSVR_RETURN_SUCCESS != ret) {
		std::cout << "No pose state!" << std::endl;
		return;
	}

	glm::vec3 position = glm::vec3(state.translation.data[0], state.translation.data[1], state.translation.data[2]);
	glm::quat orientation;
	orientation.x = osvrQuatGetX(&(state.rotation));
	orientation.y = osvrQuatGetY(&(state.rotation));
	orientation.z = osvrQuatGetZ(&(state.rotation));
	orientation.w = osvrQuatGetW(&(state.rotation));
	glm::mat4 transform = glm::translate(glm::mat4(), position) * glm::mat4_cast(orientation);
	this->setTransform(transform);

	m_renderInfo = m_renderManager->GetRenderInfo();

	std::vector<ViewPoint *> vps = viewpoints();

	for (size_t i = 0; i < m_renderInfo.size(); i++) {
		setViewpointPose(vps[i], m_renderInfo[i].pose);
	}

	RenderToTextureDisplay::prepareForDraw();
}

void OsvrHMD::finishDraw()
{
	std::vector<ViewPoint *> vps = viewpoints();

	for (size_t i = 0; i < m_renderInfo.size(); i++) {
		GLuint colorBuffer = colorBuffers[i].OpenGL->colorBufferName;
		GLuint depthBuffer = depthBuffers[i];

		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		// Set color and depth buffers for the frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_RENDERBUFFER, depthBuffer);

		// Set the list of draw buffers.
		GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		glBindFramebuffer(GL_READ_FRAMEBUFFER, activeFrameBuffer());

		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE
				|| glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "RenderView: Incomplete Framebuffer" << std::endl;
			return;
		}

		// Clear the screen to white and clear depth
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int width = static_cast<int>(m_renderInfo[i].viewport.width);
		int height = static_cast<int>(m_renderInfo[i].viewport.height);

		ViewPort *viewport = vps[i]->viewport();

		glBlitFramebuffer(
				(GLint) viewport->offsetX(),
				(GLint) viewport->offsetY(),
				(GLint) viewport->offsetX() + viewport->width(),
				(GLint) viewport->offsetY() + viewport->height(),
				(GLint) 0,
				(GLint) 0,
				(GLint) width,
				(GLint) height,
				GL_COLOR_BUFFER_BIT,
				GL_NEAREST
				);
	}

	// Send the rendered results to the screen
	if (!m_renderManager->PresentRenderBuffers(colorBuffers, m_renderInfo)) {
		std::cerr << "PresentRenderBuffers() returned false, maybe because "
			"it was asked to quit"
			<< std::endl;
	}
}

void OsvrHMD::setViewpointProjection(ViewPoint *viewpoint, osvr::renderkit::OSVR_ProjectionMatrix &projectionToUse)
{
	GLdouble glProjection[16];
	osvr::renderkit::OSVR_Projection_to_OpenGL(glProjection, projectionToUse);

	glm::mat4 glmProjection = glm::make_mat4(glProjection);

	viewpoint->overrideProjectionMatrix(glmProjection);
}

void OsvrHMD::setViewpointPose(ViewPoint *viewpoint, OSVR_PoseState &pose)
{
	GLdouble glModelView[16];
	osvr::renderkit::OSVR_PoseState_to_OpenGL(glModelView, pose);

	glm::mat4 transform = glm::inverse(glm::make_mat4(glModelView));

	viewpoint->setWorldTransform(transform);
}
