#pragma once
#include "../extLibs/glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>

// ╭───────────────╮
// │ Window Params │
// ╰───────────────╯
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// ball params
float vy{0.0f}, vx{0.0f}, dt{0.016f};
float cx = WIDTH / 2.0f;
float cy = HEIGHT / 2.0f;
float lyPos = HEIGHT / 2.0f;
float ryPos = HEIGHT / 2.0f;
const float PADDLE_WIDTH = 20.0f;
const float PADDLE_HEIGHT = 100.0f;
const float PADDLE_X_OFFSET = 40.0f;
const float PADDLE_HALF_WIDTH = PADDLE_WIDTH / 2.0f;
const float PADDLE_HALF_HEIGHT = PADDLE_HEIGHT / 2.0f;

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window) {
  float paddleSpeed = 15.0f;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS &&
      (lyPos + PADDLE_HALF_HEIGHT) < HEIGHT)
    lyPos += paddleSpeed;

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS &&
      (lyPos - PADDLE_HALF_HEIGHT) > 0.0f)
    lyPos -= paddleSpeed;

  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS &&
      (ryPos + PADDLE_HALF_HEIGHT) < HEIGHT)
    ryPos += paddleSpeed;

  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS &&
      (ryPos - PADDLE_HALF_HEIGHT) > 0.0f)
    ryPos -= paddleSpeed;
}
// ╭──────────────────────────────────────────╮
// │ Ball Collision with top and bottom walls │
// ╰──────────────────────────────────────────╯
void CollisionCheck(float cx, float cy, float r) {
  const float MIN_BOUNCE_RATIO = 0.2f;
  if (cy + r >= HEIGHT) {
    vy = -fabs(vy);

    float speed = sqrt(vx * vx + vy * vy);
    if (fabs(vy) / speed < MIN_BOUNCE_RATIO) {
      float new_vy = -speed * MIN_BOUNCE_RATIO;
      float new_vx_mag = sqrt(speed * speed - new_vy * new_vy);

      vx = copysignf(new_vx_mag, vx);
      vy = new_vy;
    }
  }
  if (cy - r <= 0.0f) {
    vy = fabs(vy);

    float speed = sqrt(vx * vx + vy * vy);
    if (vy / speed < MIN_BOUNCE_RATIO) {
      float new_vy = speed * MIN_BOUNCE_RATIO;
      float new_vx_mag = sqrt(speed * speed - new_vy * new_vy);
      vx = copysignf(new_vx_mag, vx);
      vy = new_vy;
    }
  }
}
// ╭────────────────────────────╮
// │ Ball and Paddle Collision  │
// ╰────────────────────────────╯
void paddleCollisionCheck(float ball_r, float paddle_x, float paddle_y,
                          float paddle_half_width, float paddle_half_height,
                          bool isLeftPaddle) {

  // Paddle boundaries
  float paddle_left = paddle_x - paddle_half_width;
  float paddle_right = paddle_x + paddle_half_width;
  float paddle_bottom = paddle_y - paddle_half_height;
  float paddle_top = paddle_y + paddle_half_height;

  // AABB Collision Check
  bool collisionX = cx + ball_r >= paddle_left && cx - ball_r <= paddle_right;
  bool collisionY = cy + ball_r >= paddle_bottom && cy - ball_r <= paddle_top;

  if (collisionX && collisionY) {
    if ((isLeftPaddle && vx < 0) || (!isLeftPaddle && vx > 0)) {
      vx = -vx;
      vx *= 1.05f;
      float relativeIntersectY = (paddle_y - cy) / paddle_half_height;
      const float MAX_BOUNCE_VY = 600.0f; // bounciness adjustment
      vy = -relativeIntersectY * MAX_BOUNCE_VY;
      if (isLeftPaddle) {
        cx = paddle_right + ball_r;
      } else {
        cx = paddle_left - ball_r;
      }
    }
  }
}
void makeCircle(std::vector<float> &vertices, int numSegments, float radius,
                unsigned int &VAO, unsigned int &VBO) {
  vertices.clear();
  vertices.reserve((numSegments + 2) * 4);

  vertices.push_back(0.0f); // x
  vertices.push_back(0.0f); // y
  vertices.push_back(0.5f); // u
  vertices.push_back(0.5f); // v

  for (int i = 0; i <= numSegments; ++i) {
    float angle = 2.0f * M_PI * i / numSegments;
    float x = radius * cos(angle);
    float y = radius * sin(angle);

    // Mapping circle point to [0,1] texture coords
    float u = (x / radius + 1.0f) * 0.5f;
    float v = (y / radius + 1.0f) * 0.5f;

    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(u);
    vertices.push_back(v);
  }

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}
void genPaddle(unsigned int &paddleVao, unsigned int &paddleVbo,
               unsigned int &paddleEbo) {

  float paddleVertices[] = {
      -0.5f, -0.5f, 0.0f, 0.0f, // b-l
      0.5f,  -0.5f, 1.0f, 0.0f, // b-r
      0.5f,  0.5f,  1.0f, 1.0f, // u-r
      -0.5f, 0.5f,  0.0f, 1.0f  // u-l
  };
  unsigned int paddleIndices[] = {
      0, 1, 2, // 1
      2, 3, 0  // 2
  };
  glGenVertexArrays(1, &paddleVao);
  glGenBuffers(1, &paddleVbo);
  glGenBuffers(1, &paddleEbo);
  glBindVertexArray(paddleVao);
  glBindBuffer(GL_ARRAY_BUFFER, paddleVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(paddleVertices), paddleVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, paddleEbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(paddleIndices), paddleIndices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
}
float randFloat(float a, float b) {
  return a + (b - a) * (static_cast<float>(rand()) / RAND_MAX);
}
void scoreUpdate(float radius) {
  if (cx - radius < 0.0f) {
    std::cout << "Right Player Scored!" << std::endl;
    cx = WIDTH / 2.0f;
    cy = HEIGHT / 2.0f;
    vx = randFloat(-600.0f, 600.0f);
    vy = randFloat(-600.0f, 600.0f);
  } else if (cx + radius > WIDTH) {
    std::cout << "Left Player Scored!" << std::endl;
    cx = WIDTH / 2.0f;
    cy = HEIGHT / 2.0f;
    vx = randFloat(-600.0f, 600.0f);
    vy = randFloat(-600.0f, 600.0f);
  }
}
