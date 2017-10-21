//
//  AsteroGpuProgram.h
//  Astero
//
//  Created by Yuzhe Wang on 10/10/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroGpuProgram_h
#define AsteroGpuProgram_h

namespace Astero {
	class GpuProgram {
	public:
		virtual bool isAdjacencyInfoRequired() const { return adjacency_info_required_; }
		
	protected:
		bool adjacency_info_required_;
		
	};
	class GLGpuProgram : public GpuProgram {
	public:
		virtual bool isAttributeValid(VertexElementSemantic semantic, unsigned int index);
		virtual GLuint getAttributeIndex(VertexElementSemantic semantic, unsigned int index);
	};
}

#endif // AsteroGpuProgram_h
