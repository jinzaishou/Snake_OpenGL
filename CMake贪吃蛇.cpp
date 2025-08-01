#define STB_IMAGE_IMPLEMENTATION
#include "./external/stb/stb_image.h"

#include <deque>
#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

const int gridWidth = 20;
const int gridHeight = 20;
const float cellSize = 2.0f / gridWidth;

struct Vec2i {
    int x, y;
    bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; }
};
std::deque<Vec2i> snake = { {10, 7} };
Vec2i direction = { 1, 0 };
Vec2i food = { 5, 5 };

float moveInterval = 0.2f;
float toRadians(float degree) { return degree * 3.14159265f / 180.0f; }

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    direction = { 0, 1 };
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  direction = { 0, -1 };
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  direction = { -1, 0 };
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) direction = { 1, 0 };
}

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
uniform sampler2D ourTexture;
void main() {
    FragColor = texture(ourTexture, texCoord);
}
)";

// 加载纹理函数
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
        glTexImage2D(GL_TEXTURE_2D, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB),
            width, height, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB),
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "纹理加载失败: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

int main() {
    srand((unsigned)time(0));
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Snake Game", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 着色器
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // 顶点数据
    float vertices[] = {
        // pos        // tex
        -1.0f / gridWidth, -1.0f / gridHeight, 0.0f, 0.0f,
         1.0f / gridWidth, -1.0f / gridHeight, 1.0f, 0.0f,
         1.0f / gridWidth,  1.0f / gridHeight, 1.0f, 1.0f,
        -1.0f / gridWidth, -1.0f / gridHeight, 0.0f, 0.0f,
         1.0f / gridWidth,  1.0f / gridHeight, 1.0f, 1.0f,
        -1.0f / gridWidth,  1.0f / gridHeight, 0.0f, 1.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 加载纹理
    GLuint snakeHeadTex = loadTexture("C:\\dev\\snake\\CMake贪吃蛇\\textures\\snake_head.png");
    GLuint snakeBodyTex = loadTexture("C:\\dev\\snake\\CMake贪吃蛇\\textures\\snake_body.png");
    GLuint foodTex = loadTexture("C:\\dev\\snake\\CMake贪吃蛇\\textures\\food.png");

    int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    int angleLoc = glGetUniformLocation(shaderProgram, "angle");

    float lastLogicTime = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        processInput(window);

        // 移动逻辑
        static std::deque<Vec2i> oldSnake = snake;
        if (currentTime - lastLogicTime >= moveInterval) {
            lastLogicTime = currentTime;
            oldSnake = snake;
            Vec2i newHead = { snake.front().x + direction.x, snake.front().y + direction.y };
            if (newHead.x < 0 || newHead.x >= gridWidth || newHead.y < 0 || newHead.y >= gridHeight) {
                std::cout << "Game Over\\n";
                break;
            }
            if (newHead == food) {
                snake.push_front(newHead);
                food = { rand() % gridWidth, rand() % gridHeight };
            }
            else {
                snake.push_front(newHead);
                snake.pop_back();
            }
        }

        float t = (currentTime - lastLogicTime) / moveInterval;
        if (t > 1.0f) t = 1.0f;

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // 渲染蛇
        for (size_t i = 0; i < snake.size(); ++i) {
            Vec2i oldPos = (i < oldSnake.size()) ? oldSnake[i] : snake[i];
            Vec2i newPos = snake[i];
            float x = -1.0f + (oldPos.x + (newPos.x - oldPos.x) * t) * cellSize + cellSize / 2.0f;
            float y = -1.0f + (oldPos.y + (newPos.y - oldPos.y) * t) * cellSize + cellSize / 2.0f;
            glUniform2f(offsetLoc, x, y);

            if (i == 0) { // 蛇头
                float angle = 0.0f;
                if (direction.x == 1) angle = 90.0f;
                else if (direction.x == -1) angle = -90.0f;
                else if (direction.y == 1) angle = 0.0f;
                else if (direction.y == -1) angle = 180.f;
                glUniform1f(angleLoc, toRadians(angle));
                glBindTexture(GL_TEXTURE_2D, snakeHeadTex);
            }
            else { // 身体
                glUniform1f(angleLoc, 0.0f);
                glBindTexture(GL_TEXTURE_2D, snakeBodyTex);
            }
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // 渲染食物
        float fx = -1.0f + food.x * cellSize + cellSize / 2.0f;
        float fy = -1.0f + food.y * cellSize + cellSize / 2.0f;
        glUniform2f(offsetLoc, fx, fy);
        glUniform1f(angleLoc, 0.0f);
        glBindTexture(GL_TEXTURE_2D, foodTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
