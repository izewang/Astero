//
//  AsteroGLDefaultHardwareBuffer.h
//  Astero
//
//  Created by Yuzhe Wang on 10/11/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroGLDefaultHardwareBuffer_h
#define AsteroGLDefaultHardwareBuffer_h

#include "AsteroPrerequisites.h"
#include "AsteroHardwareBuffer.h"

namespace Astero {
	class GLDefaultHardwareVertexBuffer : public DefaultHardwareVertexBuffer {
	public:
		GLDefaultHardwareVertexBuffer(size_t vertex_size,
									  size_t vertex_num,
									  HardwareBuffer::Usage usage);
		~GLDefaultHardwareVertexBuffer();
		
		void * getData(size_t offset) const {
			return (void *)(data_ + offset);
		}
	protected:
		
	};
	
	class GLDefaultHardwareIndexBuffer : public DefaultHardwareIndexBuffer {
	public:
		GLDefaultHardwareIndexBuffer(size_t vertex_size,
									  size_t vertex_num,
									  HardwareBuffer::Usage usage);
		~GLDefaultHardwareIndexBuffer();
		
		void * getData(size_t offset) const {
			return (void *)(data_ + offset);
		}
	protected:
		
	};
}

#endif // AsteroGLDefaultHardwareBuffer_h
