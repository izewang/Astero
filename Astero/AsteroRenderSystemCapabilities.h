//
//  AsteroRenderSystemCapabilities.h
//  Astero
//
//  Created by Yuzhe Wang on 10/10/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#ifndef AsteroRenderSystemCapabilities_h
#define AsteroRenderSystemCapabilities_h

namespace Astero {
	enum Capabilities {
		RSC_FIXED_FUNCTION,
		RSC_VBO
	};
	
	class RenderSystemCapabilities {
	public:
		bool hasCapability(const Capabilities cap);
		unsigned short getTextureUnitNumber() const;
	};
}

#endif // AsteroRenderSystemCapabilities_h
