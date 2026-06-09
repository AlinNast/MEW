#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main()
{
	// Initialize GLFW
    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);

	// Check if the window was created successfully
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Main loop
    while (!glfwWindowShouldClose(window))
    {
		// Render here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// Swap front and back buffers
        glfwSwapBuffers(window);
		// Poll for and process events
        glfwPollEvents();
    }

	// Clean up and exit	
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}