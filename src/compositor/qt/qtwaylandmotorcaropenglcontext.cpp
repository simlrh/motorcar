#include <qt/qtwaylandmotorcaropenglcontext.h>


qtmotorcar::QtWaylandMotorcarOpenGLContext::QtWaylandMotorcarOpenGLContext(QOpenGLWindow *window)
    :m_window(window)
{
}

glm::ivec2 qtmotorcar::QtWaylandMotorcarOpenGLContext::defaultFramebufferSize()
{
   return glm::ivec2(m_window->size().width(), m_window->size().height());
}

QOpenGLWindow *qtmotorcar::QtWaylandMotorcarOpenGLContext::window() {
	return m_window;
}

void qtmotorcar::QtWaylandMotorcarOpenGLContext::makeCurrent()
{
    m_window->makeCurrent();
}
