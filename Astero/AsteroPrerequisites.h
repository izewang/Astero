//
//  AsteroPrerequisites.h
//  Astero
//
//  Created by Yuzhe Wang on 10/3/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroPrerequisites_h
#define AsteroPrerequisites_h

#include "AsteroStdHeaders.h"

namespace Astero {
	class HardwareBuffer;
	class HardwareBufferManager;
	class HardwareVertexBuffer;
	class HardwareIndexBuffer;
	class RenderToVertexBuffer;
	class SceneManager;
	class Renderable;
	class Material;
	class VertexDeclaration;
	class VertexBufferBinding;
	class VertexData;
	class IndexData;
	class VertexElement;
	class RenderSystemCapabilities;
	class Matrix4;

	
	typedef std::shared_ptr<HardwareVertexBuffer> HardwareVertexBufferPtr;
	typedef std::shared_ptr<HardwareIndexBuffer> HardwareIndexBufferPtr;
	typedef std::shared_ptr<RenderToVertexBuffer> RenderToVertexBufferPtr;
	
	class GLSupport;
	class GLRenderSystem;
	class GLTexture;
	class GLTextureManager;
	class GLGpuProgram;
	class GLContext;
	class GLRTTManager;
	class GLFBOManager;
	class GLHardwarePixelBuffer;
	class GLRenderBuffer;
	class GLDepthBuffer;
	
	typedef std::shared_ptr<GLGpuProgram> GLGpuProgramPtr;
	typedef std::shared_ptr<GLTexture> GLTexturePtr;

}

#endif // AsteroPrerequisites_h
