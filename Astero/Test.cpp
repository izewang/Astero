//
//  Test.cpp
//  Astero
//
//  Created by Yuzhe Wang on 8/9/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#include <stdio.h>
#include "AsteroContainers.tpp"
#include "AsteroGeometry.h"
#include "AsteroResource.h"
#include "AsteroMesh.h"
#include "AsteroMeshLoader.h"
#include "AsteroDataStream.h"
#include "AsteroRenderSystem.h"
#include "AsteroHardwareBuffer.h"

using namespace Astero;

class Test {
public:
	Test() {
		printf("HelloWorld\n");
		HardwareBuffer * testptr;
		testptr->unlock();
	}
};

int main () {
	printf("Hello World!\n");
}
