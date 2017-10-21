//
//  AsteroRenderOperation.h
//  Astero
//
//  Created by Yuzhe Wang on 9/29/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroRenderOperation_h
#define AsteroRenderOperation_h

#include "AsteroPrerequisites.h"
#include "AsteroVertexIndexData.h"

namespace Astero {
	struct RenderOperation {
	public:
		// Render operation type to perform.
		enum OperationType {
			// A list of points, 1 vertex per point.
			OT_POINT_LIST = 1,
			// A list of lines, 2 vertices per line.
			OT_LINE_LIST = 2,
			// A strip of connected lines, 2 vertices for first line, 1 per additional line segament.
			OT_LINE_STRIP = 3,
			// A list of triangles, 3 vertices per triangle.
			OT_TRIANGLE_LIST = 4,
			// A stirp of triangles, 3 vertices for first triangle, 1 per additional triangle.
			OT_TRIANGLE_STRIP = 5,
			// A fan of triangles, 3 vertices for first triangle, 1 per additional triangle.
			OT_TRIANGLE_FAN = 6
		};
		
		RenderOperation();
		
		VertexData * vertex_data;
		OperationType operation_type;
		// Specifies whether to use indeces
		bool use_indices;
		IndexData * index_data;
		unsigned int instance_number;
		bool render_to_vertex_buffer;
		// A flag to indicate that it is possible to use global instance vertex buffer.
		bool use_global_instance_vertex_buffer;
	};
}

#endif // AsteroRenderOperation_h
