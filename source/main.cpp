#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>


struct Vec2
{
	float x=0.0f;
	float y=0.0f;
};

Vec2 offset; // Define a 2D vector to store the offset values for the vertex shader


void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key) {
		case GLFW_KEY_UP:
			offset.y += 0.01f; // Move up by increasing the y offset
			break;
		case GLFW_KEY_DOWN:
			offset.y -= 0.01f; // Move down by decreasing the y offset
			break;
		case GLFW_KEY_LEFT:
			offset.x -= 0.01f; // Move left by decreasing the x offset
			break;
		case GLFW_KEY_RIGHT:
			offset.x += 0.01f; // Move right by increasing the x offset
			break;
		}
	}
}



int main()
{


	/// Initialization  -------------------------------------------------


	/// Initialize GLFW
    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
	}
	// Set GLFW window hints for OpenGL version and profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Request OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Request OpenGL version 3.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile

	// Create a windowed mode window and its OpenGL context
	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);

	// Check if the window was created successfully
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}


	glfwSetKeyCallback(window, keyCallBack);

	// Make the window's context current
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate(); // Clean up GLFW resources
		return -1;
	}

	/// Initialization end -------------------------------------------------



	/// Shader setup -------------------------------------------------

	// Define vertex shader source code
	std::string vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 color;
		uniform vec2 uOffset;

		out vec3 vColor;

		void main()
		{
			vColor = color;
			gl_Position = vec4(aPos.x + uOffset.x, aPos.y + uOffset.y, aPos.z, 1.0);
		}
	)";

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader object
	const char* vertexShaderSourceCStr = vertexShaderSource.c_str(); // Get a C-style string pointer to the vertex shader source code
	glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, nullptr); // Set the source code of the vertex shader
	glCompileShader(vertexShader); // Compile the vertex shader

	GLint success; // check for shader compilation success
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // Check if the vertex shader compiled successfully
	if (!success) 
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
	}

	// Define fragment shader source code
	std::string fragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		in vec3 vColor;
		uniform vec4 uColor;

		void main()
		{
			FragColor = vec4(vColor, 1.0) * uColor;
		}
	)";

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create a fragment shader object
	const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str(); // Get a C-style string pointer to the fragment shader source code
	glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr); // Set the source code of the fragment shader
	glCompileShader(fragmentShader); // Compile the fragment shader

	// Check if the fragment shader compiled successfully
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); 
	if (!success) 
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
	}

	// Link shaders into a shader program
	GLuint shaderProgram = glCreateProgram(); // Create a shader program object
	glAttachShader(shaderProgram, vertexShader); // Attach the vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); // Attach the fragment shader to the shader program
	glLinkProgram(shaderProgram); // Link the shader program

	// Check for shader program linking success
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) 
	{
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cerr << "Shader program linking failed: " << infoLog << std::endl;
	}

	// After linking the shader program, we can delete the individual shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	/// Shader setup end -------------------------------------------------

	/// Vertex data setup -------------------------------------------------
	// Define the vertices of a rectabgle
	std::vector<float> vertices = {
		0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 
		-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f
	};

	std::vector<unsigned int> indices = {
		0, 1, 2, // First triangle
		0, 2, 3  // Second triangle
	};

	// Create a Vertex Buffer Object (VBO) and upload the vertex data to the GPU
	GLuint vbo;
	glGenBuffers(1, &vbo); // Generate a buffer object and store its ID in the variable 'vbo'
	glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the buffer object to the GL_ARRAY_BUFFER target, making it the current active buffer for vertex data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); // Upload the vertex data to the GPU. The size of the data is calculated as the number of vertices multiplied by the size of a float. The pointer to the vertex data is obtained using vertices.data(). The usage hint GL_STATIC_DRAW indicates that the vertex data will not change frequently.
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO by binding to 0, which means no buffer is currently bound to the GL_ARRAY_BUFFER target

	GLuint ebo;
	glGenBuffers(1, &ebo); // Generate a buffer object for the element buffer and
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // Bind the buffer object to the GL_ELEMENT_ARRAY_BUFFER target, making it the current active buffer for index data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW); // Upload the index data to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind the EBO by binding to 0, which means no buffer is currently bound to the GL_ELEMENT_ARRAY_BUFFER target

	// Create a Vertex Array Object (VAO) to store the vertex attribute configuration
	GLuint vao;
	glGenVertexArrays(1, &vao); // Generate a vertex array object and store its ID in the variable 'vao'
	glBindVertexArray(vao); // Bind the vertex array object, making it the current active VAO
	glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the VBO again to the GL_ARRAY_BUFFER target, as the VAO needs to know which VBO to use for vertex attributes
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // Bind the EBO again to the GL_ELEMENT_ARRAY_BUFFER target, as the VAO needs to know which EBO to use for index data

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0); // Define the layout of the vertex data for the vertex shader. This function specifies how the vertex attributes are stored in the VBO. The first parameter '0' corresponds to the location of the vertex attribute in the vertex shader (layout(location = 0)). The second parameter '3' indicates that each vertex attribute consists of 3 components (x, y, z). The third parameter 'GL_FLOAT' specifies that the data type of each component is a float. The fourth parameter 'false' indicates that the data should not be normalized. The fifth parameter '3 * sizeof(float)' specifies the stride, which is the byte offset between consecutive vertex attributes. Since each vertex attribute consists of 3 floats, the stride is 3 times the size of a float. The last parameter '(void*)0' specifies the offset of the first component of the first vertex attribute in the VBO, which is 0 in this case since the vertex data starts at the beginning of the buffer.
	glEnableVertexAttribArray(0); // Enable the vertex attribute array at location 0, which allows the vertex shader to access the vertex data defined by the previous glVertexAttribPointer call

	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Define the layout of the vertex color attribute for the vertex shader. The first parameter '1' corresponds to the location of the vertex attribute in the vertex shader (layout(location = 1)). The second parameter '3' indicates that each vertex color attribute consists of 3 components (r, g, b). The third parameter 'GL_FLOAT' specifies that the data type of each component is a float. The fourth parameter 'false' indicates that the data should not be normalized. The fifth parameter '6 * sizeof(float)' specifies the stride, which is the byte offset between consecutive vertex attributes. Since each vertex consists of 6 floats (3 for position and 3 for color), the stride is 6 times the size of a float. The last parameter '(void*)(3 * sizeof(float))' specifies the offset of the first component of the first vertex color attribute in the VBO, which is 3 floats (the position data) from the start of each vertex.
	glEnableVertexAttribArray(1); // Enable the vertex attribute array at location 1,

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO by binding to 0, which means no buffer is currently bound to the GL_ARRAY_BUFFER target
	glBindVertexArray(0); // Unbind the VAO by binding to 0, which means no VAO is currently active

	GLuint uColorLoc = glGetUniformLocation(shaderProgram, "uColor"); // Get the location of the uniform variable 'uColor' in the shader program. This function returns an integer that represents the location of the uniform variable, which can be used to set its value later in the rendering loop.
	GLuint uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset"); // Get the location of the uniform variable 'uOffset' in the shader program


	/// Vertex data setup end -------------------------------------------------

	/// Main loop --------------------------------------------------
	/// 
	
    while (!glfwWindowShouldClose(window))
    {
		// Render here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);

		glUniform4f(uColorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Set the color to green
		glUniform2f(uOffsetLoc, offset.x, offset.y); // Set the offset values for the vertex shader

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// glBindVertexArray(0);

		// Swap front and back buffers
        glfwSwapBuffers(window);
		// Poll for and process events
        glfwPollEvents();
    }

	/// Main loop end --------------------------------------------------

	// Clean up and exit	
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}