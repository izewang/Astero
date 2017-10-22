//
//  AsteroGLSupport.h
//  Astero
//
//  Created by Yuzhe Wang on 10/22/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroGLSupport_h
#define AsteroGLSupport_h

#include "AsteroPrerequisites.h"

namespace Astero {
	class GLSupport {
	public:
		virtual RenderWindow * newWindow(const std::string & name, unsigned int width, unsigned int height, bool full_screen,
										 const NameValuePairList * misc_params);
	protected:
		
	};
} // namespace Astero

#endif // AsteroGLSupport_h
