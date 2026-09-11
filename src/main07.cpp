#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include "shader.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lunar Lander", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    Shader shader("shaders/shape.vert", "shaders/shape.frag");


    float padVertices[] = {
        -0.15f,  -0.65f,
         0.15f,  -0.65f,
         0.15f,  -0.60f,
        -0.15f,  -0.60f
    };

    GLuint padVAO, padVBO;

    glGenVertexArrays(1, &padVAO);
    glGenBuffers(1, &padVBO);

    glBindVertexArray(padVAO);
    glBindBuffer(GL_ARRAY_BUFFER, padVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(padVertices), padVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

   
    while(!glfwWindowShouldClose(window))
    {
        
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        shader.setVec3("shapeColor", 0.0f, 1.0f, 0.0f);

        glBindVertexArray(padVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &padVAO);
    glDeleteBuffers(1, &padVBO);
    
    glfwTerminate();

    return 0;
}
