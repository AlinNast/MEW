#include < GLFW/glfw3.h>
#include <iostream>

int main()
{
	// Initialize GLFW
    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
	}

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

	// Main loop
    while (!glfwWindowShouldClose(window))
    {
		// Render here
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