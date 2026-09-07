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

    float shipVertices[] = {
        0.0f,   0.04f,
       -0.03f, -0.03f,
        0.03f, -0.03f
    };

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shipVertices), shipVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Ship state
    glm::vec2 shipPos(0.0f, 0.5f);
    glm::vec2 shipVel(0.0f, 0.0f);   // Velocity

    float shipRotation = 0.0f;
    float shipRotationSpeed = 180.0f;
    
    float lastFrame = 0.0f;
    
    float thrustAccel = 0.8f;
    const float GRAVITY = -0.5f;

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            shipRotation = shipRotation + (shipRotationSpeed * deltaTime);
        }
        
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            shipRotation = shipRotation - (shipRotationSpeed * deltaTime);
        }
        
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            float angleRad = glm::radians(shipRotation);
            float thrustX = std::sin(angleRad) * thrustAccel;
            float thrustY = std::cos(angleRad) * thrustAccel;

            shipVel.x = shipVel.x + (thrustX * deltaTime);
            shipVel.y = shipVel.y + (thrustY * deltaTime);
            
        }   
        
        // Gravity pulls down
        shipVel.y = shipVel.y + (GRAVITY * deltaTime);

        // Update position based on velocity
        shipPos = shipPos + (shipVel * deltaTime);
       

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        // Build model matrix: translate first, then rotate
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(shipPos, 0.0f));
        model = glm::rotate(model, glm::radians(shipRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        

        shader.setMat4("model", model);
        shader.setVec3("shapeColor", 1.0f, 1.0f, 1.0f);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

    return 0;
}
