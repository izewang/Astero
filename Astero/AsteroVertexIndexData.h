//
//  AsteroVertexIndexData.h
//  Astero
//
//  Created by Yuzhe Wang on 10/7/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroVertexIndexData_h
#define AsteroVertexIndexData_h

#include "AsteroPrerequisites.h"

namespace Astero {
	struct VertexData {
	public:
		VertexData();
		~VertexData();
		
		unsigned int vertex_count;
		VertexDeclaration * vertex_declaration;
		VertexBufferBinding * vertex_buffer_binding;
		size_t vertex_start;
		
	protected:
		HardwareBufferManager * manager_;
	};
	
	struct IndexData {
	public:
		HardwareIndexBufferPtr index_buffer;
		unsigned int index_count;
		size_t index_start;
	};
}


#endif // AsteroVertexIndexData_h
