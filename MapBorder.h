// MapBorder.h
#pragma once
#include <glad/glad.h>
#include <glm.hpp>

class MapBorder {
public:
    MapBorder(float left, float right, float top, float bottom);
    ~MapBorder();
    void Draw(GLuint shaderProgram);

private:
    GLuint VAO, VBO;
};
