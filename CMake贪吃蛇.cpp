#define STB_IMAGE_IMPLEMENTATION
#include "./external/stb/stb_image.h"
#include "MapBorder.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <deque>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>



struct Vec2i {
    int x, y;
    bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; }
};

float toRadians(float degree) {
    return degree * 3.14159265f / 180.0f;
}

void resetGame(); 

enum GameState { MENU, GAME, SETTINGS, EXIT };
GameState gameState = MENU;

const int gridWidth = 20;
const int gridHeight = 20;
const float cellSize = 2.0f / gridWidth;
float moveInterval = 0.1f;
float lastLogicTime = 0.0f;

// 蛇
std::deque<Vec2i> snake = { {10, 7} };
Vec2i direction = { 1, 0 };
std::deque<Vec2i> dirQueue;
Vec2i food = { 5, 5 };
std::deque<Vec2i> oldSnake;
bool keyState[4] = { false, false, false, false };
float headAngle = 0.0f;

// 顶点数据
float vertices[] = {
    -1.0f / (gridWidth + 1), -1.0f / (gridHeight + 1),  0.0f, 0.0f,
     1.0f / (gridWidth + 1), -1.0f / (gridHeight + 1),  1.0f, 0.0f,
     1.0f / (gridWidth + 1),  1.0f / (gridHeight + 1),  1.0f, 1.0f,
    -1.0f / (gridWidth + 1), -1.0f / (gridHeight + 1),  0.0f, 0.0f,
     1.0f / (gridWidth + 1),  1.0f / (gridHeight + 1),  1.0f, 1.0f,
    -1.0f / (gridWidth + 1),  1.0f / (gridHeight + 1),  0.0f, 1.0f
};

// 顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;
uniform vec2 offset;
uniform float angle;

void main() {
    float cosA = cos(angle);
    float sinA = sin(angle);
    mat2 rotation = mat2(cosA, -sinA, sinA, cosA);
    vec2 rotatedPos = rotation * aPos;
    gl_Position = vec4(rotatedPos + offset, 0.0, 1.0);
    texCoord = aTexCoord;
}
)";

// 片元着色器
const char* fragmentShaderSource = R"(
#version 330 core
in vec2 texCoord;
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D ourTexture;
uniform bool useTexture;

void main() {
    if(useTexture)
        FragColor = texture(ourTexture, texCoord);
    else
        FragColor = vec4(color, 1.0);
}
)";

// 纹理加载
GLuint loadTexture(const char* path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "纹理加载失败: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

// 输入处理
void processInput(GLFWwindow* window, Vec2i currentDir) {
    // 键盘与方向映射表
    struct KeyDir { int key; Vec2i dir; Vec2i opposite; };
    KeyDir keys[4] = {
        { GLFW_KEY_UP,    { 0,  1}, { 0, -1} },
        { GLFW_KEY_DOWN,  { 0, -1}, { 0,  1} },
        { GLFW_KEY_LEFT,  {-1,  0}, { 1,  0} },
        { GLFW_KEY_RIGHT, { 1,  0}, {-1,  0} }
    };

    if (gameState == GAME) {
        for (int i = 0; i < 4; i++) {
            if (glfwGetKey(window, keys[i].key) == GLFW_PRESS) {
                // “刚按下”时才触发
                if (!keyState[i]) {
                    // 防止反方向掉头
                    if (!(currentDir.x == keys[i].opposite.x && currentDir.y == keys[i].opposite.y)) {
                        dirQueue.push_back(keys[i].dir);
                    }
                    keyState[i] = true;
                }
            }
            else {
                // 松开时重置该按键状态
                keyState[i] = false;
            }
        }
    }
}

// 更新头部角度
void updateHeadAngle(Vec2i dir) {
    if (dir.x == 1 && dir.y == 0) headAngle = 90.0f;
    else if (dir.x == 0 && dir.y == 1) headAngle = 0.0f;
    else if (dir.x == -1 && dir.y == 0) headAngle = -90.0f;
    else if (dir.x == 0 && dir.y == -1) headAngle = 180.0f;
}

// 菜单
int menuIndex = 0;
const int menuCount = 3;
void processMenu(GLFWwindow* window) {
    static bool menuPressed = false;
    if (!menuPressed) {
        //向上切换
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            menuIndex = (menuIndex - 1 + menuCount) % menuCount;
            menuPressed = true;
        }
		//向下切换
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            menuIndex = (menuIndex + 1) % menuCount;
            menuPressed = true;
        }
		//选择菜单项
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            if (menuIndex == 0) { resetGame();gameState = GAME; lastLogicTime = glfwGetTime();}
            else if (menuIndex == 1) gameState = SETTINGS;
            else if (menuIndex == 2) gameState = EXIT;
            menuPressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        menuPressed = false;
    }
}

void resetGame() {
    snake.clear();
    snake.push_back({ gridWidth / 2, gridHeight / 2 });
    food = { rand() % (gridWidth - 2) + 1, rand() % (gridHeight - 2) + 1 };
    moveInterval = 0.1f;
}


GLuint LoadShader(const char* vertexPath, const char* fragmentPath)
{
    // 1. 读取文件内容
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR: Shader file not successfully read: "
            << e.what() << std::endl;
        return 0;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. 编译着色器
    GLuint vertex, fragment;
    GLint success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "ERROR: Vertex shader compilation failed\n" << infoLog << std::endl;
    }

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "ERROR: Fragment shader compilation failed\n" << infoLog << std::endl;
    }

    // 3. 链接着色器程序
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader program linking failed\n" << infoLog << std::endl;
    }

    // 4. 删除中间着色器对象
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}


int main() {
    srand((unsigned)time(0));
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



    GLFWwindow* window = glfwCreateWindow(800, 800, "Snake with Menu", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint borderShader = LoadShader("C:/dev/snake/CMake贪吃蛇/shaders/border.vert", "C:/dev/snake/CMake贪吃蛇/shaders/border.frag");
    MapBorder border(-0.9f, 0.9f, 0.9f, -0.9f);

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint texHead = loadTexture("C:/dev/snake/CMake贪吃蛇/textures/snake_head.png");
    GLuint texBody = loadTexture("C:/dev/snake/CMake贪吃蛇/textures/snake_body1.png");
    GLuint texFood = loadTexture("C:/dev/snake/CMake贪吃蛇/textures/food.png");

    int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    int angleLoc = glGetUniformLocation(shaderProgram, "angle");
    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    int useTexLoc = glGetUniformLocation(shaderProgram, "useTexture");

    while (!glfwWindowShouldClose(window)) {

        if (snake.size() % 4==0) {
			moveInterval = 0.1f-(snake.size()/4)*0.01; // 加快速度
        }
        float currentTime = glfwGetTime();
        glClearColor(0.2f, 0.3f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        if (gameState == MENU) {
            processMenu(window);
            for (int i = 0; i < menuCount; ++i) {
                float y = 0.4f - i * 0.3f;
                glUniform2f(offsetLoc, 0.0f, y);
                glUniform1f(angleLoc, 0.0f);
                if (i == menuIndex)
                    glUniform3f(colorLoc, 1.0f, 1.0f, 0.0f);
                else
                    glUniform3f(colorLoc, 0.6f, 0.6f, 0.6f);
                glUniform1i(useTexLoc, 0);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        else if (gameState == GAME) {

            processInput(window, direction);
            if (currentTime - lastLogicTime >= moveInterval) {
                lastLogicTime = currentTime;
                oldSnake = snake;
                if (!dirQueue.empty()) {
                    Vec2i nextDir = dirQueue.front(); dirQueue.pop_front();
                    if (!(direction.x == -nextDir.x && direction.y == -nextDir.y))
                        direction = nextDir;
                }
                updateHeadAngle(direction);
                Vec2i newHead = { snake.front().x + direction.x, snake.front().y + direction.y };
                if (newHead.x < 1 || newHead.x >= gridWidth-1 || newHead.y < 1 || newHead.y >= gridHeight-1) {
                    std::cout << "撞墙，游戏结束！\n";
                    gameState = MENU; // 返回菜单
                }
                if (newHead == food) {
                    snake.push_front(newHead);
                    food = { rand() % (gridWidth - 2) + 1, rand() % (gridHeight-2) + 1 };
                }
                else {
                    snake.push_front(newHead);
                    snake.pop_back();
                }
            }
            float t = (currentTime - lastLogicTime) / moveInterval;
            if (t > 1.0f) t = 1.0f;

            for (size_t i = 0; i < snake.size(); ++i) {
                Vec2i oldPos = (i < oldSnake.size()) ? oldSnake[i] : snake[i];
                Vec2i newPos = snake[i];
                float x = -1.0f + (oldPos.x + (newPos.x - oldPos.x) * t) * cellSize + cellSize / 2.0f;
                float y = -1.0f + (oldPos.y + (newPos.y - oldPos.y) * t) * cellSize + cellSize / 2.0f;
                glUniform2f(offsetLoc, x, y);
                if (i == 0) {
                    glBindTexture(GL_TEXTURE_2D, texHead);
                    glUniform1i(useTexLoc, 1);
                    glUniform1f(angleLoc, toRadians(headAngle));
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, texBody);
                    glUniform1i(useTexLoc, 1);
                    glUniform1f(angleLoc, 0.0f);
                }
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            float fx = -1.0f + food.x * cellSize + cellSize / 2.0f;
            float fy = -1.0f + food.y * cellSize + cellSize / 2.0f;
            glUniform2f(offsetLoc, fx, fy);
            glBindTexture(GL_TEXTURE_2D, texFood);
            glUniform1i(useTexLoc, 1);
            glUniform1f(angleLoc, 0.0f);
            glUseProgram(shaderProgram);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glUseProgram(borderShader);
            border.Draw(borderShader);
        }
        else if (gameState == SETTINGS) {
            glUniform2f(offsetLoc, 0.0f, 0.0f);
            glUniform1f(angleLoc, 0.0f);
            glUniform3f(colorLoc, 0.2f, 0.7f, 1.0f);
            glUniform1i(useTexLoc, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) gameState = MENU;
        }
        else if (gameState == EXIT) {
            glfwSetWindowShouldClose(window, true);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
