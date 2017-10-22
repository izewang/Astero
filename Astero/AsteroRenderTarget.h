//
//  AsteroRenderTarget.h
//  Astero
//
//  Created by Yuzhe Wang on 10/22/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroRenderTarget_h
#define AsteroRenderTarget_h

#include "AsteroPrerequisites.h"

namespace Astero {
	// Abstract class of a root to all targets or 'canvas' of render operations.
	class RenderTarget {
	public:
		enum StatFlags
		{
			SF_NONE           = 0,
			SF_FPS            = 1,
			SF_AVG_FPS        = 2,
			SF_BEST_FPS       = 4,
			SF_WORST_FPS      = 8,
			SF_TRIANGLE_COUNT = 16,
			SF_ALL            = 0xFFFF
		};
		enum FrameBuffer
		{
			FB_FRONT,
			FB_BACK,
			FB_AUTO
		};
		struct FrameStats
		{
			float lastFPS;
			float avgFPS;
			float bestFPS;
			float worstFPS;
			unsigned long bestFrameTime;
			unsigned long worstFrameTime;
			size_t triangleCount;
			size_t batchCount;
			int vBlankMissCount; // -1 means that the value is not applicable
		};
		
		RenderTarget();
		virtual ~RenderTarget();
		
		virtual const std::string & getName() const;
		virtual unsigned char getPriority() const;
		
	protected:
		
	};
} // namespace Astero

#endif // AsteroRenderTarget_h
