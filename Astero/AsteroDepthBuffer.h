//
//  AsteroDepthBuffer.h
//  Astero
//
//  Created by Yuzhe Wang on 10/13/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroDepthBuffer_h
#define AsteroDepthBuffer_h

#include "AsteroPrerequisites.h"

namespace Astero {
	//--------------------------------------------------------------------------------------------------------------------------------
	class DepthBuffer {
	public:
		enum PoolId {
			PI_NO_DEPTH = 0,
			PI_MANUAL_USAGE = 0,
			POOL_DEFAULT = 1
		};
		
		DepthBuffer(unsigned short pool_id, unsigned short bit_depth, unsigned int width, unsigned int height,
					unsigned int fsaa, const std::string & fsaa_hint, bool manual);
		virtual ~DepthBuffer();
		
		void setPoolId();
		
	protected:
		typedef std::unordered_set<RenderTarget *> RenderTargetSet;
		
		unsigned short pool_id_;
		unsigned short bit_depth_;
		unsigned int width_;
		unsigned int height_;
		unsigned int fsaa_;
		std::string fsaa_hint_;
		bool manual_;
		RenderTargetSet attached_render_targets_;
		
		void detachFromAllRenderTargets();
	};
	//--------------------------------------------------------------------------------------------------------------------------------
	class GLDepthBuffer : public DepthBuffer {
	public:
		GLDepthBuffer();
		~GLDepthBuffer();
		
	protected:
		unsigned int multi_sample_quality_;
		GLContext * context_;
		GLRenderBuffer * depth_buffer_;
		GLRenderBuffer * stencil_buffer_;
		GLRenderSystem * render_system_;
	};
}

#endif // AsteroDepthBuffer_h
