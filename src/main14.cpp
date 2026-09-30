#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include "shader.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- Game state enum ---
enum GameState {
    FLYING,
    LANDED,
    CRASHED 
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow* StartWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lunar Lander", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return GLFW_FALSE;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return GLFW_FALSE;
    }
    
    return window;
}


int main() {
    
    GLFWwindow* window = StartWindow();

    Shader shader("shaders/shape.vert", "shaders/shape.frag");

    // --- Ship geometry ---
    float shipVertices[] = {
         0.0f,   0.04f,     // Top
        -0.03f,  -0.03f,    // left bottom
         0.03f,  -0.03f    // right bottom
    };

    GLuint shipVAO, shipVBO;
    glGenVertexArrays(1, &shipVAO);
    glGenBuffers(1, &shipVBO);

    glBindVertexArray(shipVAO);
    glBindBuffer(GL_ARRAY_BUFFER, shipVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shipVertices), shipVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Landing pad geometry ---
    float padVertices[] = {
        -0.15f, -0.65f,
         0.15f, -0.65f,
         0.15f, -0.60f,
        -0.15f, -0.60f
    };

    GLuint padVAO, padVBO;
    glGenVertexArrays(1, &padVAO);
    glGenBuffers(1, &padVBO);

    glBindVertexArray(padVAO);
    glBindBuffer(GL_ARRAY_BUFFER, padVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(padVertices), padVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Ground geometry ---
    float groundVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f, -0.75f,
        -1.0f, -0.75f
    };

    GLuint groundVAO, groundVBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Ship state ---
    glm::vec2 shipPos(0.0f, 0.5f);
    glm::vec2 shipVel(0.0f, 0.0f);
    float shipRotation = 0.0f;
    float shipRotationSpeed = 180.0f;
    float thrustAccel = 0.8f;
    const float GRAVITY = -0.5f;

    // --- Landing pad state ---
    glm::vec2 padCenter(0.0f, -0.625f);
    float padWidth = 0.3f;
    float padHeight = 0.05f;

    // --- Ground collision bounds ---
    float groundTop = -0.75f;
    float groundBottom = -1.0f;

    // --- Game state ---
    GameState gameState = FLYING;
    float landingSpeed = 0.0f;  // store velocity magnitude when landed (for display)

    // --- Landing validation criteria ---
    const float MAX_LANDING_SPEED = 0.15f;   // max velocity to land safely
    const float MAX_LANDING_ANGLE = 15.0f;   // max rotation angle (in degrees) to land safely

    float lastFrame = 0.0f;
    bool keyRPressed = false;  // track if R was just pressed to avoid re-triggering

    while (!glfwWindowShouldClose(window)) 
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || 
            glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- Reset on R key ---
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) 
        {
            if (!keyRPressed) 
            {
                gameState = FLYING;
                shipPos = glm::vec2(0.0f, 0.5f);
                shipVel = glm::vec2(0.0f, 0.0f);
                shipRotation = 0.0f;
                keyRPressed = true;
            }
        } 
        else 
        {
            keyRPressed = false;
        }

        // --- Input (only allow control while flying) ---
        if (gameState == FLYING) 
        {
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                shipRotation += shipRotationSpeed * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                shipRotation -= shipRotationSpeed * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
            {
                float angleRad = glm::radians(shipRotation);
                float thrustX = std::sin(angleRad) * thrustAccel;
                float thrustY = std::cos(angleRad) * thrustAccel;
                shipVel.x += thrustX * deltaTime;
                shipVel.y += thrustY * deltaTime;
            }

            // --- Physics ---
            shipVel.y += GRAVITY * deltaTime;
            shipPos += shipVel * deltaTime;

            // --- Boundary wrapping ---
            if (shipPos.x > 1.0f) shipPos.x = -1.0f;
            if (shipPos.x < -1.0f) shipPos.x = 1.0f;
            
            // --- Collision detection ---
            // Check if ship crashed into ground
            if (shipPos.y < groundTop || shipPos.y > 1.0f) {
                gameState = CRASHED;
                std::cout << "CRASHED!\n";
            }

            // Check if ship is near landing pad
            float distX = std::abs(shipPos.x - padCenter.x);
            float distY = std::abs(shipPos.y - padCenter.y);

            if (distX < padWidth / 2.0f && distY < padHeight) {
                // Ship is on the landing pad — check if landing is safe
                float shipSpeed = glm::length(shipVel);
                float shipRotationAbs = std::abs(shipRotation);
                // Normalize rotation to [0, 180] (shortest angle)
                if (shipRotationAbs > 180.0f)
                    shipRotationAbs = 360.0f - shipRotationAbs;

                if (shipSpeed <= MAX_LANDING_SPEED && shipRotationAbs <= MAX_LANDING_ANGLE) {
                    gameState = LANDED;
                    landingSpeed = shipSpeed;
                    std::cout << "LANDED SAFELY! Speed: " << shipSpeed << "\n";
                } else {
                    gameState = CRASHED;
                    std::cout << "CRASH! Speed too high or angle too steep.\n";
                    std::cout << "  Speed: " << shipSpeed << " (max: " << MAX_LANDING_SPEED << ")\n";
                    std::cout << "  Angle: " << shipRotationAbs << "° (max: " << MAX_LANDING_ANGLE << "°)\n";
                }
            }
        }

        // --- Render ---
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        // --- Draw ground ---
        {
            glm::mat4 model = glm::mat4(1.0f);
            shader.setMat4("model", model);
            shader.setVec3("shapeColor", 0.3f, 0.25f, 0.2f);
            glBindVertexArray(groundVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            
        }

        // --- Draw landing pad ---
        {
            glm::mat4 model = glm::mat4(1.0f);
            shader.setMat4("model", model);
            shader.setVec3("shapeColor", 0.0f, 1.0f, 0.0f);
            glBindVertexArray(padVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        // --- Draw ship (color changes based on state) ---
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(shipPos, 0.0f));
            model = glm::rotate(model, glm::radians(shipRotation), glm::vec3(0.0f, 0.0f, 1.0f));
            shader.setMat4("model", model);

            // Color based on game state
            if (gameState == FLYING) {
                shader.setVec3("shapeColor", 1.0f, 1.0f, 1.0f);  // white
            } else if (gameState == LANDED) {
                shader.setVec3("shapeColor", 0.0f, 1.0f, 0.5f);  // cyan (success)
            } else if (gameState == CRASHED) {
                shader.setVec3("shapeColor", 1.0f, 0.0f, 0.0f);  // red (failure)
            }

            glBindVertexArray(shipVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &shipVAO);
    glDeleteBuffers(1, &shipVBO);
    glDeleteVertexArrays(1, &padVAO);
    glDeleteBuffers(1, &padVBO);
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glfwTerminate();
    return 0;
}
