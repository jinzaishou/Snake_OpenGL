#include "MapBorder.h"

MapBorder::MapBorder(float left, float right, float top, float bottom) {
    // 确保使用局部数组，这里是 NDC 坐标
    float vertices[8] = {
        left,  bottom,
        right, bottom,
        right, top,
        left,  top
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // layout(location = 0) in vec2 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

MapBorder::~MapBorder() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void MapBorder::Draw(GLuint shaderProgram) {
    // 画到最上层：禁用深度测试（如果你有启用）
    GLboolean depthEnabled = (glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);
    if (depthEnabled) glDisable(GL_DEPTH_TEST);

    glUseProgram(shaderProgram);
    // 可选：给 shader 传颜色 uniform（如果需要）
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
    if (colorLoc != -1) {
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // 白色
    }

    glBindVertexArray(VAO);
    glLineWidth(3.0f); // 有些平台可能忽略线宽
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindVertexArray(0);

    if (depthEnabled) glEnable(GL_DEPTH_TEST);
}
