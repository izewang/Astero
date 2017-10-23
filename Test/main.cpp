//
//  main.cpp
//  Test
//
//  Created by Yuzhe Wang on 10/23/17.
//  Copyright © 2017 Yuzhe Wang. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include "AsteroContainers.tpp"
#include "AsteroGeometry.h"
#include "AsteroResource.h"
#include "AsteroMesh.h"
#include "AsteroMeshLoader.h"
#include "AsteroDataStream.h"
#include "AsteroRenderSystem.h"
#include "AsteroHardwareBuffer.h"

using namespace Astero;

int main(int argc, const char * argv[]) {
	GLFWwindow* window;
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	
	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	
	window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // Windowed
	
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	
	printf("vertexBuffer = %u\n", vertexBuffer);
	printf("%s", glfwGetVersionString());
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		
		/* Poll for and process events */
		glfwPollEvents();
		
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
	}
	
	glfwTerminate();
	
    return 0;
}
