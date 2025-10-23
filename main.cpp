#include "glad/glad.h"
#include "helpers.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

int main() {
  srand(static_cast<unsigned int>(time(0)));

  // ╭────────────────╮
  // │ Initialization │
  // ╰────────────────╯
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "GL Ping Pong", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // ╭──────────────╮
  // │ Shaders Load │
  // ╰──────────────╯
  Shader paddleShader("shaders/paddle.vert", "shaders/paddle.frag");
  Shader midlineShader("shaders/midLine.vert", "shaders/midLine.frag");
  Shader ballShader("shaders/ball.vert", "shaders/ball.frag");

  // ╭────────────╮
  // │ Projection │
  // ╰────────────╯
  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WIDTH), 0.0f,
                                    static_cast<float>(HEIGHT), -1.0f, 1.0f);

  // Passing projection to all shaders
  paddleShader.use();
  paddleShader.setMat4("projection", glm::value_ptr(projection));
  midlineShader.use();
  midlineShader.setMat4("projection", glm::value_ptr(projection));
  ballShader.use();
  ballShader.setMat4("projection", glm::value_ptr(projection));

  // ╭────────╮
  // │ Paddle │
  // ╰────────╯
  unsigned int paddleVao, paddleVbo, paddleEbo;
  genPaddle(paddleVao, paddleVbo, paddleEbo);

  // ╭────────╮
  // │ Circle │
  // ╰────────╯
  unsigned int circleVao, circleVbo;
  int numSegments = 100;
  float radius = 25.0f;
  std::vector<float> circleVertices;
  makeCircle(circleVertices, numSegments, radius, circleVao, circleVbo);

  // random velocity
  vx = (rand() % 2 == 0 ? 1 : -1) * randFloat(300.0f, 600.0f);
  vy = randFloat(-300.0f, 300.0f);

  // ╭─────────╮
  // │ Texture │
  // ╰─────────╯
  unsigned int texture = loadTexture("lines.png");

  // ╭─────────────╮
  // │ Render Loop │
  // ╰─────────────╯
  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float dt = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;
    cx += vx * dt;
    cy += vy * dt;

    // collision check
    CollisionCheck(cx, cy, radius);
    paddleCollisionCheck(radius, PADDLE_X_OFFSET + PADDLE_HALF_WIDTH, lyPos,
                         PADDLE_HALF_WIDTH, PADDLE_HALF_HEIGHT, true);

    // Right Paddle
    paddleCollisionCheck(radius, WIDTH - PADDLE_X_OFFSET - PADDLE_HALF_WIDTH,
                         ryPos, PADDLE_HALF_WIDTH, PADDLE_HALF_HEIGHT, false);

    // Check if anyone scored
    scoreUpdate(radius);
    // ╭──────────────╮
    // │ Right Paddle │
    // ╰──────────────╯
    paddleShader.use();
    paddleShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(
        model,
        glm::vec3(WIDTH - PADDLE_X_OFFSET - PADDLE_WIDTH / 2.0f, ryPos, 0.0f));
    model = glm::scale(model, glm::vec3(PADDLE_WIDTH, PADDLE_HEIGHT, 1.0f));
    paddleShader.setMat4("model", glm::value_ptr(model));
    glBindVertexArray(paddleVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // ╭─────────────╮
    // │ Left Paddle │
    // ╰─────────────╯
    paddleShader.setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::mat4(1.0f);
    model = glm::translate(
        model, glm::vec3(PADDLE_X_OFFSET + PADDLE_WIDTH / 2.0f, lyPos, 0.0f));
    model = glm::scale(model, glm::vec3(PADDLE_WIDTH, PADDLE_HEIGHT, 1.0f));
    paddleShader.setMat4("model", glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // ╭────────╮
    // │ Circle │
    // ╰────────╯
    ballShader.use();
    ballShader.setVec3("color", glm::vec3(1.0f));
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(cx, cy, 0.0f));
    ballShader.setMat4("model", glm::value_ptr(model));
    glBindVertexArray(circleVao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 2);

    // ╭──────────╮
    // │ Mid Line │
    // ╰──────────╯
    midlineShader.use();
    midlineShader.setFloat("repeatY", HEIGHT / 32.0f);
    midlineShader.setInt("tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(10.0f, HEIGHT, 1.0f));
    midlineShader.setMat4("model", glm::value_ptr(model));
    glBindVertexArray(paddleVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteTextures(1, &texture);
  glDeleteBuffers(1, &paddleVbo);
  glDeleteBuffers(1, &paddleEbo);
  glDeleteVertexArrays(1, &paddleVao);
  glDeleteBuffers(1, &circleVbo);
  glDeleteVertexArrays(1, &circleVao);
  glfwTerminate();
  return 0;
}
