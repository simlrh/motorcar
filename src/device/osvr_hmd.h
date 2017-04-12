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

#ifndef OSVRHMD_H
#define OSVRHMD_H

#include <scenegraph/output/display/rendertotexturedisplay.h>
#include <scenegraph/input/singlebonetracker.h>

#include <scenegraph/output/viewpoint.h>

#include <osvr/ClientKit/Context.h>
#include <osvr/RenderKit/RenderManager.h>

#include <osvr/ClientKit/Interface.h>
#include <osvr/ClientKit/InterfaceStateC.h>

#include <glm/gtc/quaternion.hpp>

#include <algorithm>

namespace motorcar {

	class OsvrHMD : public RenderToTextureDisplay
	{
		public:

			OsvrHMD(Skeleton *skeleton, OpenGLContext *glContext, PhysicalNode *parent);
			~OsvrHMD();

			virtual void prepareForDraw() override;
			virtual void finishDraw() override;

			virtual glm::ivec2 size() override;

		private:
			osvr::clientkit::ClientContext m_context;

			osvr::renderkit::RenderManager* m_renderManager;
			osvr::renderkit::RenderManager::RenderParams m_params;

			GLuint frameBuffer;
			std::vector<osvr::renderkit::RenderBuffer> colorBuffers;
			std::vector<GLuint> depthBuffers;
			std::vector<osvr::renderkit::RenderInfo> m_renderInfo;

			osvr::clientkit::Interface m_interface;

			glm::ivec2 m_renderTargetSize;

			bool SetupRendering(osvr::renderkit::GraphicsLibrary library);

			void setViewpointProjection(ViewPoint *viewpoint, osvr::renderkit::OSVR_ProjectionMatrix &projectionToUse);
			void setViewpointPose(ViewPoint *viewpoint, OSVR_PoseState &pose);
	};
}


#endif // OSVRHMD_H
