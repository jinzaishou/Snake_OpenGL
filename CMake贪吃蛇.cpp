#define STB_IMAGE_IMPLEMENTATION
#include "CMake贪吃蛇.h"
#include "./external/stb/stb_image.h"

#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


float y_offset = 0.0f;
float x_offset = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 处理输入
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		y_offset += 0.01f; // 向上移动
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		y_offset -= 0.01f; // 向下移动
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		x_offset -= 0.01f; // 向左移动
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		x_offset += 0.01f; // 向右移动
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		y_offset = 0.0f; // 重置 Y 偏移
		x_offset = 0.0f; // 重置 X 偏移
	}
}

// ========== 顶点着色器（增加颜色传递） ==========
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec3 aColor;
    layout(location = 2) in vec2 aTexCoord;

    out vec3 vertexColor;
    out vec2 texCoord;

    uniform vec2 offset;

    void main() {
        gl_Position = vec4(aPos + offset, 0.0, 1.0);
        vertexColor = aColor;
        texCoord = aTexCoord;
    }
)";

// ========== 片元着色器（接收颜色） ==========
const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 texCoord;
    in vec3 vertexColor;

    out vec4 FragColor;

    uniform sampler2D ourTexture;

    void main() {
        FragColor = texture(ourTexture, texCoord);
    }
)";


int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW 初始化失败\n";
        return -1;
    }

    // 设置 OpenGL 版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Green Box", nullptr, nullptr);
    if (!window) {
        std::cerr << "窗口创建失败\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 加载 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD 初始化失败\n";
        return -1;
    }

    // ========== 1. 创建顶点着色器 ==========
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // 检查编译错误
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "顶点着色器编译失败:\n" << infoLog << '\n';
    }

    // ========== 2. 创建片元着色器 ==========
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "片元着色器编译失败:\n" << infoLog << '\n';
    }

    // ========== 3. 链接着色器程序 ==========
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "着色器程序链接失败:\n" << infoLog << '\n';
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ========== 4. 顶点数据（一个矩形由两个三角形组成） ==========
    float vertices[] = {
        // 位置       // 颜色         // 纹理坐标
        -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f,

        -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f
    };

    // ========== 5. 创建 VAO & VBO ==========
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 顶点位置: layout(location = 1)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 顶点颜色: layout(location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // ========== 6. 加载纹理 ==========

    // 生成纹理对象
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载图像数据
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // OpenGL 的 Y 坐标是反的
    std::cout << "当前工作目录：" << std::filesystem::current_path() << std::endl;
    unsigned char* data = stbi_load("C:/dev/snake/CMake贪吃蛇/textures/snake.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            nrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "纹理加载失败！" << std::endl;
    }
    stbi_image_free(data);




    // ========== 7. 渲染循环 ==========
    while (!glfwWindowShouldClose(window)) {
		processInput(window);//获取x,y偏移

		int offsetLocation = glGetUniformLocation(shaderProgram, "offset");//获取 uniform 位置
        int textureLocation = glGetUniformLocation(shaderProgram, "ourTexture");

        glUniform1i(textureLocation, 0);
        glUniform2f(offsetLocation, x_offset, y_offset);//

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture); // 绑定纹理
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ========== 7. 清理资源 ==========
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}