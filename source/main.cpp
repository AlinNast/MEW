#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <map>


/// Structs -------------------------------------------------
/// ----------------   Snake game Code begins here
struct Vec2
{
	float x, y;
	Vec2() : x(0.0f), y(0.0f) {} // Default constructor initializes x and y to 0.0f
	Vec2(float x, float y) : x(x), y(y) {} // Parameterized constructor initializes x and y to the provided values
};

struct Vec2i 
{
	int x, y;
	Vec2i() : x(0), y(0) {} // Default constructor initializes x and y to 0
	Vec2i(int x, int y) : x(x), y(y) {} // Parameterized constructor initializes x and y to the provided values
	bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; } // Equality operator checks if two Vec2i objects are equal
};

struct Vec3
{
	float r, g, b;
	Vec3() : r(0.0f), g(0.0f), b(0.0f) {} // Default constructor initializes r, g, and b to 0
	Vec3(float r, float g, float b) : r(r), g(g), b(b) {} // Parameterized constructor initializes r, g, and b to the provided values
};


/// Game constants -------------------------------------------------

const int GRID_WIDTH = 20; // Width of the game grid
const int GRID_HEIGHT = 20; // Height of the game grid
const float UPDATE_INTERVAL = 0.2f; // Time interval for updating the game state (in seconds)
const float CELL_WIDTH = 2.0f / GRID_WIDTH; // Width of each cell in the grid (normalized to OpenGL coordinates)
const float CELL_HEIGHT = 2.0f / GRID_HEIGHT; // Height of each cell

/// Game state variables -------------------------------------------------

enum class Direction { UP, DOWN, LEFT, RIGHT }; // Enum to represent the direction of the snake
Direction currentDirection = Direction::RIGHT; // Current direction of the snake, initialized to RIGHT
std::vector<Vec2i> snake = { Vec2i(GRID_WIDTH / 2, GRID_HEIGHT / 2), Vec2i(GRID_WIDTH / 2 -1, GRID_HEIGHT / 2),Vec2i(GRID_WIDTH / 2 - 2, GRID_HEIGHT / 2) }; // Vector to store the positions of the snake segments, initialized with the starting position at the center of the grid
Vec2i food; // Position of the food on the grid
int score = 0; // Player's score, initialized to 0
bool gameOver = false; // Flag to indicate if the game is over, initialized to false
bool gameStarted = false; // Flag to indicate if the game has started, initialized to false
float lastUpdateTime = 0.0f; // Time of the last game state update, initialized to 0.0f
float snakeSpeed = UPDATE_INTERVAL; // Speed of the snake, initialized to the update interval


/// Shader variables -------------------------------------------------
std::string vertexShaderSource = R"(
	#version 330 core
	layout (location = 0) in vec2 aPos;
	uniform vec2 uOffset;
	uniform vec2 uScale;

	void main()
	{
		vec2 position = (aPos * uScale) + uOffset;
		gl_Position = vec4(position, 0.0, 1.0);
	}
)";

std::string fragmentShaderSource = R"(
	#version 330 core
	out vec4 FragColor;
	uniform vec3 uColor;
	void main()
	{
		FragColor = vec4(uColor, 1.0); 
	}
)";

/// OpenGL variables -------------------------------------------------

GLuint shaderProgram; // Variable to store the shader program ID
GLuint VAO, VBO; // Variables to store the Vertex Array Object and Vertex Buffer Object IDs
GLuint uOffsetLoc, uScaleLoc, uColorLoc; // Variables to store the uniform locations for offset, scale, and color in the shader program

/// Font variables -------------------------------------------------
const int FONT_WIDTH = 5; // Width of each character in the font texture
const int FONT_HEIGHT = 5; // Height of each character in the font texture
const int FONT_SPACING = 1; // Spacing between characters in the font texture

/// Character Deifnitions -------------------------------------------------

std::map<char, std::vector<int>> fontMap =
{
	{' ' ,{0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}},
	{'A' ,{0,1,1,1,0, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1}},
	{'B' ,{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,1, 1,1,1,0,0}},
	{'C' ,{0,1,1,1,1, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 0,1,1,1,1}},
	{'D' ,{1,1,1,0,0, 1,0,0,1,0, 1,0,0,0,1, 1,0,0,1,0, 1,1,1,0,0}},
	{'E' ,{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,1,1,1,1}},
	{'F' ,{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0}},
	{'G' ,{0,1,1,1,1, 1,0,0,0,0, 1,0,0,1,1, 1,0,0,0,1, 0,1,1,1,1}},
	{'H' ,{1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1}},
	{'I' ,{1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1}},
	{'J' ,{0,0,0,1,1, 0,0,0,0,1, 0,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
	{'K' ,{1,0,0,0,1, 1,0,0,1,0, 1,1,0,0,0, 1,0,1,0,0, 1,0,0,1,0}},
	{'L' ,{1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,0,0,0,0, 1,1,1,1,1}},
	{'M' ,{1,0,0,0,1, 1,1,0,1,1, 1,0,1,0,1, 1,0,0,0,1, 1,0,0,0,1}},
	{'N' ,{1,0,0,0,1, 1,0,1,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1}},
	{'O' ,{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
	{'P' ,{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,0,0,0, 1,0,0,0,0}},
	{'Q' ,{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,1,0,1, 0,1,1,1,0}},
	{'R' ,{1,1,1,1,0, 1,0,0,0,1, 1,1,1,1,0, 1,0,1,0,0, 1,0,0,1,0}},
	{'S' ,{0,1,1,1,1, 1,0,0,0,0, 0,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
	{'T' ,{1,1,1,1,1, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}},
	{'U' ,{1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
	{'V' ,{1,0,0,0,1, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,0,1,0,0}},
	{'W' ,{1,0,0,0,1, 1,0,0,0,1, 1,0,1,0,1, 1,1,0,1,1, 1,0,0,0,1}},
	{'X' ,{1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,1,0,1,0, 1,0,0,0,1}},
	{'Y' ,{1,0,0,0,1, 0,1,0,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}},
	{'Z' ,{1,1,1,1,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 1,1,1,1,1}},
	{'0' ,{0,1,1,1,0, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 0,1,1,1,0}},
	{'1' ,{0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}},
	{'2' ,{0,1,1,1,0, 1,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 1,1,1,1,1}},
	{'3' ,{1,1,1,1,0, 0,0,0,0,1, 0,0,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
	{'4' ,{0,0,1,1,0, 0,1,0,1,0, 1,0,0,1,0, 1,1,1,1,1, 0,0,0,1,0}},
	{'5' ,{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,0, 0,0,0,0,1, 1,1,1,1,0}},
	{'6' ,{0,1,1,1,0, 1,0,0,0,0, 1,1,1,1,0, 1,0,0,0,1, 0,1,1,1,0}},
	{'7' ,{1,1,1,1,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0}},
	{'8' ,{0,1,1,1,0, 1,0,0,0,1, 0,1,1,1,0, 1,0,0,0,1, 0,1,1,1,0}},
	{'9' ,{0,1,1,1,0, 1,0,0,0,1, 0,1,1,1,1, 0,0,0,0,1, 0,1,1,1,0}},
	{':' ,{0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0}},
	{'-' ,{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}},
	{'.' ,{0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0}}
};


/// Functions -------------------------------------------------

void SpawnFruit();
void InitGame();
void ResetGame();
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void DrawCell(const Vec2i& position, const Vec3& color);
void DrawChar(char c, float x, float y, float scale, const Vec3& color);
void DrawText(const std::string& text, float x, float y, float scale, const Vec3& color);
void RenderGame(GLFWwindow* window);
void UpdateGame(float deltaTime);
void DrawBorder();
void DrawSnake();
void DrawScore();
void DrawGameOver();
void DrawStartScreen();
void DrawFruit();

int main()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// Configure GLFW 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create Window
	GLFWwindow* window = glfwCreateWindow(800, 800, "Snake Game", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, KeyCallback);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Compile shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
	glCompileShader(vertexShader);

	// Check for shader compile errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		glfwTerminate();
		return -1;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
	glCompileShader(fragmentShader);

	// Check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		glfwTerminate();
		return -1;
	}

	//  Create shader program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		glfwTerminate();
		return -1;
	}

	// Cleanup shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Get uniform locations
	uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
	uScaleLoc = glGetUniformLocation(shaderProgram, "uScale");
	uColorLoc = glGetUniformLocation(shaderProgram, "uColor");

	// Setup VAO and VBO
	const float vertices[] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		-0.5f,  0.5f,
		 0.5f,  0.5f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// Initialize game
	InitGame();

	// Main loop
	auto lastTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window)) {
		// Calculate delta time
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		// process input
		glfwPollEvents();

		// update game
		UpdateGame(deltaTime);

		// Render game
		RenderGame(window);
	}

	// Cleanup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();

}


void InitGame()
{
	ResetGame();
	SpawnFruit();
}

void ResetGame()
{
	snake = { Vec2i(GRID_WIDTH / 2, GRID_HEIGHT / 2), Vec2i(GRID_WIDTH / 2 - 1, GRID_HEIGHT / 2) };
	currentDirection = Direction::RIGHT;
	gameOver = false;
	gameStarted = false;
	score = 0;
	lastUpdateTime = 0.0f;
	snakeSpeed = UPDATE_INTERVAL;
}

void SpawnFruit()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> disX(0, GRID_WIDTH - 1);
	std::uniform_int_distribution<> disY(0, GRID_HEIGHT - 1);
	while (true)
	{
		Vec2i newFood(disX(gen), disY(gen));
		// Check if the new food position is not on the snake
		bool validPosition = true;
		for (const auto& segment : snake)
		{
			if (segment == newFood)
			{
				validPosition = false;
				break;
			}

		}
		if (validPosition)
		{
			food = newFood;
			return;
		}
	}
}

void UpdateGame(float deltaTime)
{
	if (gameStarted && !gameOver) {

		// Update the last update time
		lastUpdateTime += deltaTime;

		// Game update
		if (lastUpdateTime >= snakeSpeed)
		{
			lastUpdateTime = 0.0f;

			// Move the snake
			Vec2i newHead = snake.front();

			switch (currentDirection)
			{
			case Direction::UP:    newHead.y += 1; break;
			case Direction::DOWN:  newHead.y -= 1; break;
			case Direction::LEFT:  newHead.x -= 1; break;
			case Direction::RIGHT: newHead.x += 1; break;
			}

			// Check for collisions with walls
			if (newHead.x < 0 || newHead.x >= GRID_WIDTH || newHead.y < 0 || newHead.y >= GRID_HEIGHT)
			{
				gameOver = true;
				return;
			}

			// Check for collisions with itself
			for (const auto& segment : snake)
			{
				if (segment == newHead)
				{
					gameOver = true;
					return;
				}
			}

			// Add the new head to the snake
			snake.insert(snake.begin(), newHead);

			// Check if the snake has eaten the food
			if (newHead == food)
			{
				score++;
				SpawnFruit();

				// Increase the snake's speed every 5 points
				if (score % 5 == 0 && snakeSpeed > 0.05f)
				{
					snakeSpeed -= 0.01f; // Increase speed by decreasing the interval
				}
			}
			else
			{
				snake.pop_back(); // Remove the tail segment if no food is eaten
			}
		}

	}
}

void RenderGame(GLFWwindow* window)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);

	DrawBorder();

	if (!gameStarted)
	{
		DrawStartScreen();
	}
	else if (gameOver)
	{
		DrawGameOver();
	}
	else
	{
		DrawSnake();
		DrawFruit();
		DrawScore();
	}

	glBindVertexArray(0);

	glfwSwapBuffers(window);
}

void DrawFruit() {
	DrawCell(food, Vec3(1.0f, 0.2f, 0.2f));
}

void DrawStartScreen()
{
	// Draw a snake and a fruit
	DrawSnake();

	DrawText("SNAKE GAME", 0.0f, 0.3f, 0.025f, Vec3(0.2f, 0.8f, 0.3f));
}

void DrawGameOver()
{
	// semi transparent overlay
	for (int x = 0; x < GRID_WIDTH; x++) {
		for (int y = 0; y < GRID_HEIGHT; y++) {
			DrawCell(Vec2i(x, y), Vec3(0.2f, 0.1f, 0.1f));
		}
	}

	DrawText("GAME OVER", 0.0f, 0.3f, 0.025f, Vec3(0.2f, 0.8f, 0.3f));
}

void DrawCell(const Vec2i& position, const Vec3& color) {
	// Calculate position in NDC
	Vec2 offset(
		-1 + position.x * CELL_WIDTH + CELL_WIDTH * 0.5f,
		-1 + position.y * CELL_HEIGHT + CELL_HEIGHT * 0.5f
	);

	// Apply scaling
	Vec2 scale(CELL_WIDTH * 0.9f, CELL_HEIGHT * 0.9f);

	glUniform3f(uColorLoc, color.r, color.g, color.b);
	glUniform2f(uOffsetLoc, offset.x, offset.y);
	glUniform2f(uScaleLoc, scale.x, scale.y);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void DrawChar(char c, float x, float y, float scale, const Vec3& color) {
	// Convert to upper
	c = std::toupper(c);

	// find char in font map
	auto it = fontMap.find(c);
	if (it == fontMap.end()) {
		it = fontMap.find(' ');
	}

	const std::vector<int>& bitmap = it->second;

	// Draw each pixel
	float charWidth = FONT_WIDTH * scale;
	float charHeight = FONT_HEIGHT * scale;

	glUniform3f(uColorLoc, color.r, color.g, color.b);
	glUniform2f(uScaleLoc, scale * 0.9f, scale * 0.9f); // scale of a font pixel

	for (int i = 0; i < FONT_HEIGHT; i++) {
		for (int j = 0; j < FONT_WIDTH; j++) {
			if (bitmap[i * FONT_WIDTH + j]) {
				Vec2 offset(
					x + j * scale - charWidth / 2.0f,
					y + j * scale + charHeight / 2.0f
				);

				glUniform2f(uOffsetLoc, offset.x, offset.y);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
	}
}

void DrawText(const std::string& text, float x, float y, float scale, const Vec3& color) {
	float charWidth = FONT_WIDTH * scale;
	float spacing = FONT_SPACING * scale;
	float totalWidth = text.size() * (charWidth + spacing) - spacing;

	float startX = x - totalWidth / 2.0f;

	for (size_t i = 0; i < text.size(); i++) {
		DrawChar(text[i], startX + i * (charWidth + spacing), y, scale, color);
	}
}

void DrawBorder() {
	Vec3 borderColor(0.3f, 0.3f, 0.5f);

	for (int x = -1; x <= GRID_WIDTH; x++) {
		DrawCell(Vec2i(x, GRID_HEIGHT), borderColor);
	}

	for (int x = -1; x <= GRID_WIDTH; x++) {
		DrawCell(Vec2i(x, -1), borderColor);
	}

	for (int y = -1; y <= GRID_HEIGHT; y++) {
		DrawCell(Vec2i(GRID_WIDTH, y), borderColor);
	}
	for (int y = -1; y <= GRID_HEIGHT; y++) {
		DrawCell(Vec2i(-1, y), borderColor);
	}

	Vec3 gridColor(0.15f, 0.17f, 0.2f);
	for (int x = 0; x < GRID_WIDTH; x++) {
		for (int y = 0; y < GRID_HEIGHT; y++) {
			DrawCell(Vec2i(x, y), gridColor);
		}
	}
}

void DrawSnake() {
	Vec3 headColor(0.0f, 0.1f, 0.3f);
	Vec3 bodyColor(0.0f, 0.7f, 0.1f);

	for (size_t i = 0; i < snake.size(); i++) {
		float factor = static_cast<float>(i) / snake.size();
		Vec3 segmentColor(
			bodyColor.r * (1 - factor) + 0.1 * factor,
			bodyColor.g * (1 - factor) + 0.8 * factor,
			bodyColor.b * (1 - factor)
		);
		DrawCell(snake[i], segmentColor);
	}
}

void DrawScore() {
	std::string scoreText = "SCORE: " + std::to_string(score);
	DrawText(scoreText, 0.0f, 0.9f, 0.02f, Vec3(0.9f, 0.9f,0.9f));
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (!gameStarted && key == GLFW_KEY_R) {
			gameStarted = true;
			currentDirection = Direction::UP;
			SpawnFruit();
			return;
		}
		if (gameStarted && key == GLFW_KEY_R) {
			ResetGame();
			SpawnFruit();
		}

		if (!gameOver && gameStarted) {
			switch (key) {
			case GLFW_KEY_UP:
			{
				if (currentDirection != Direction::DOWN) {
					currentDirection = Direction::UP;

				}
			}
			break;
			case GLFW_KEY_DOWN:
			{
				if (currentDirection != Direction::UP) {
					currentDirection = Direction::DOWN;

				}
			}
			break;
			case GLFW_KEY_LEFT:
			{
				if (currentDirection != Direction::RIGHT) {
					currentDirection = Direction::LEFT;

				}
			}
			break;
			case GLFW_KEY_RIGHT:
			{
				if (currentDirection != Direction::LEFT) {
					currentDirection = Direction::RIGHT;

				}
			}
			break;
			}
		}
	}
}

/// <summary>
///  OLD RENDER TRIANGEL ENGINE IS COMMENTED HERE, IT IS NOT USED IN THE SNAKE GAME, BUT I AM KEEPING IT FOR FUTURE REFERENCE
/// </summary>

//Vec2 offset; // Define a 2D vector to store the offset values for the vertex shader
//
//
//void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//	if (action == GLFW_PRESS)
//	{
//		switch (key) {
//		case GLFW_KEY_UP:
//			offset.y += 0.01f; // Move up by increasing the y offset
//			break;
//		case GLFW_KEY_DOWN:
//			offset.y -= 0.01f; // Move down by decreasing the y offset
//			break;
//		case GLFW_KEY_LEFT:
//			offset.x -= 0.01f; // Move left by decreasing the x offset
//			break;
//		case GLFW_KEY_RIGHT:
//			offset.x += 0.01f; // Move right by increasing the x offset
//			break;
//		}
//	}
//}
//
//
//
//int main()
//{
//
//
//	/// Initialization  -------------------------------------------------
//
//
//	/// Initialize GLFW
//    if(!glfwInit())
//    {
//        std::cerr << "Failed to initialize GLFW" << std::endl;
//        return -1;
//	}
//	// Set GLFW window hints for OpenGL version and profile
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Request OpenGL version 3.3
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Request OpenGL version 3.3
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use the core profile
//
//	// Create a windowed mode window and its OpenGL context
//	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
//
//	// Check if the window was created successfully
//	if (window == nullptr)
//	{
//		std::cerr << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//
//
//	glfwSetKeyCallback(window, keyCallBack);
//
//	// Make the window's context current
//    glfwMakeContextCurrent(window);
//
//	// Initialize GLEW
//	if (glewInit() != GLEW_OK)
//	{
//		std::cerr << "Failed to initialize GLEW" << std::endl;
//		glfwTerminate(); // Clean up GLFW resources
//		return -1;
//	}
//
//	/// Initialization end -------------------------------------------------
//
//
//
//	/// Shader setup -------------------------------------------------
//
//	// Define vertex shader source code
//	std::string vertexShaderSource = R"(
//		#version 330 core
//		layout (location = 0) in vec3 aPos;
//		layout (location = 1) in vec3 color;
//		uniform vec2 uOffset;
//
//		out vec3 vColor;
//
//		void main()
//		{
//			vColor = color;
//			gl_Position = vec4(aPos.x + uOffset.x, aPos.y + uOffset.y, aPos.z, 1.0);
//		}
//	)";
//
//	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create a vertex shader object
//	const char* vertexShaderSourceCStr = vertexShaderSource.c_str(); // Get a C-style string pointer to the vertex shader source code
//	glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, nullptr); // Set the source code of the vertex shader
//	glCompileShader(vertexShader); // Compile the vertex shader
//
//	GLint success; // check for shader compilation success
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // Check if the vertex shader compiled successfully
//	if (!success) 
//	{
//		char infoLog[512];
//		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
//		std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
//	}
//
//	// Define fragment shader source code
//	std::string fragmentShaderSource = R"(
//		#version 330 core
//		out vec4 FragColor;
//		in vec3 vColor;
//		uniform vec4 uColor;
//
//		void main()
//		{
//			FragColor = vec4(vColor, 1.0) * uColor;
//		}
//	)";
//
//	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create a fragment shader object
//	const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str(); // Get a C-style string pointer to the fragment shader source code
//	glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr); // Set the source code of the fragment shader
//	glCompileShader(fragmentShader); // Compile the fragment shader
//
//	// Check if the fragment shader compiled successfully
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); 
//	if (!success) 
//	{
//		char infoLog[512];
//		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
//		std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
//	}
//
//	// Link shaders into a shader program
//	GLuint shaderProgram = glCreateProgram(); // Create a shader program object
//	glAttachShader(shaderProgram, vertexShader); // Attach the vertex shader to the shader program
//	glAttachShader(shaderProgram, fragmentShader); // Attach the fragment shader to the shader program
//	glLinkProgram(shaderProgram); // Link the shader program
//
//	// Check for shader program linking success
//	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//	if (!success) 
//	{
//		char infoLog[512];
//		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
//		std::cerr << "Shader program linking failed: " << infoLog << std::endl;
//	}
//
//	// After linking the shader program, we can delete the individual shader objects
//	glDeleteShader(vertexShader);
//	glDeleteShader(fragmentShader);
//
//	/// Shader setup end -------------------------------------------------
//
//	/// Vertex data setup -------------------------------------------------
//	// Define the vertices of a rectabgle
//	std::vector<float> vertices = {
//		0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 
//		-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 
//		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
//		0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f
//	};
//
//	std::vector<unsigned int> indices = {
//		0, 1, 2, // First triangle
//		0, 2, 3  // Second triangle
//	};
//
//	// Create a Vertex Buffer Object (VBO) and upload the vertex data to the GPU
//	GLuint vbo;
//	glGenBuffers(1, &vbo); // Generate a buffer object and store its ID in the variable 'vbo'
//	glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the buffer object to the GL_ARRAY_BUFFER target, making it the current active buffer for vertex data
//	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); // Upload the vertex data to the GPU. The size of the data is calculated as the number of vertices multiplied by the size of a float. The pointer to the vertex data is obtained using vertices.data(). The usage hint GL_STATIC_DRAW indicates that the vertex data will not change frequently.
//	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO by binding to 0, which means no buffer is currently bound to the GL_ARRAY_BUFFER target
//
//	GLuint ebo;
//	glGenBuffers(1, &ebo); // Generate a buffer object for the element buffer and
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // Bind the buffer object to the GL_ELEMENT_ARRAY_BUFFER target, making it the current active buffer for index data
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW); // Upload the index data to the GPU
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind the EBO by binding to 0, which means no buffer is currently bound to the GL_ELEMENT_ARRAY_BUFFER target
//
//	// Create a Vertex Array Object (VAO) to store the vertex attribute configuration
//	GLuint vao;
//	glGenVertexArrays(1, &vao); // Generate a vertex array object and store its ID in the variable 'vao'
//	glBindVertexArray(vao); // Bind the vertex array object, making it the current active VAO
//	glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind the VBO again to the GL_ARRAY_BUFFER target, as the VAO needs to know which VBO to use for vertex attributes
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // Bind the EBO again to the GL_ELEMENT_ARRAY_BUFFER target, as the VAO needs to know which EBO to use for index data
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0); // Define the layout of the vertex data for the vertex shader. This function specifies how the vertex attributes are stored in the VBO. The first parameter '0' corresponds to the location of the vertex attribute in the vertex shader (layout(location = 0)). The second parameter '3' indicates that each vertex attribute consists of 3 components (x, y, z). The third parameter 'GL_FLOAT' specifies that the data type of each component is a float. The fourth parameter 'false' indicates that the data should not be normalized. The fifth parameter '3 * sizeof(float)' specifies the stride, which is the byte offset between consecutive vertex attributes. Since each vertex attribute consists of 3 floats, the stride is 3 times the size of a float. The last parameter '(void*)0' specifies the offset of the first component of the first vertex attribute in the VBO, which is 0 in this case since the vertex data starts at the beginning of the buffer.
//	glEnableVertexAttribArray(0); // Enable the vertex attribute array at location 0, which allows the vertex shader to access the vertex data defined by the previous glVertexAttribPointer call
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Define the layout of the vertex color attribute for the vertex shader. The first parameter '1' corresponds to the location of the vertex attribute in the vertex shader (layout(location = 1)). The second parameter '3' indicates that each vertex color attribute consists of 3 components (r, g, b). The third parameter 'GL_FLOAT' specifies that the data type of each component is a float. The fourth parameter 'false' indicates that the data should not be normalized. The fifth parameter '6 * sizeof(float)' specifies the stride, which is the byte offset between consecutive vertex attributes. Since each vertex consists of 6 floats (3 for position and 3 for color), the stride is 6 times the size of a float. The last parameter '(void*)(3 * sizeof(float))' specifies the offset of the first component of the first vertex color attribute in the VBO, which is 3 floats (the position data) from the start of each vertex.
//	glEnableVertexAttribArray(1); // Enable the vertex attribute array at location 1,
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO by binding to 0, which means no buffer is currently bound to the GL_ARRAY_BUFFER target
//	glBindVertexArray(0); // Unbind the VAO by binding to 0, which means no VAO is currently active
//
//	GLuint uColorLoc = glGetUniformLocation(shaderProgram, "uColor"); // Get the location of the uniform variable 'uColor' in the shader program. This function returns an integer that represents the location of the uniform variable, which can be used to set its value later in the rendering loop.
//	GLuint uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset"); // Get the location of the uniform variable 'uOffset' in the shader program
//
//
//	/// Vertex data setup end -------------------------------------------------
//
//	/// Main loop --------------------------------------------------
//	/// 
//	
//    while (!glfwWindowShouldClose(window))
//    {
//		// Render here
//        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//		glUseProgram(shaderProgram);
//
//		glUniform4f(uColorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Set the color to green
//		glUniform2f(uOffsetLoc, offset.x, offset.y); // Set the offset values for the vertex shader
//
//		glBindVertexArray(vao);
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//		// glBindVertexArray(0);
//
//		// Swap front and back buffers
//        glfwSwapBuffers(window);
//		// Poll for and process events
//        glfwPollEvents();
//    }
//
//	/// Main loop end --------------------------------------------------
//
//	// Clean up and exit	
//    glfwDestroyWindow(window);
//    glfwTerminate();
//
//    return 0;
//}

