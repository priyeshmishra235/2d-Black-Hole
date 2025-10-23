#include "../extLibs/glad/glad.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

class Shader {
public:
  GLuint ID;
  Shader(const char *vertexPath, const char *fragmentPath) {
    std::ifstream vertexFile(vertexPath), fragFile(fragmentPath);
    if (!vertexFile || !fragFile) {
      std::cerr << "Failed to open shader file:" << vertexPath << "or"
                << fragmentPath << std::endl;
      ID = 0;
      return;
    }
    std::stringstream vStream, fStream;
    vStream << vertexFile.rdbuf();
    fStream << fragFile.rdbuf();

    std::string vCode = vStream.str();
    std::string fCode = fStream.str();

    const char *vSrc = vCode.c_str();
    const char *fSrc = fCode.c_str();

    ID = createShaderProgram(vSrc, fSrc);
  }
  void use() { glUseProgram(ID); }
  // I think it might be better to not fetch location each call when setting in
  // while loop so to fix that have a func. to fetch the location only and only
  // fetch one in main. I will implement it later
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }
  void setMat4(const std::string &name, const float *mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       mat);
  }
  void setVec2(const std::string &name, const glm::vec2 vec) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y);
  }

  void setVec3(const std::string &name, const glm::vec3 vec) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
  }
  ~Shader() {
    if (ID) {
      glDeleteProgram(ID);
    }
  }

private:
  static GLuint compileShader(GLenum type, const char *src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(id, 512, nullptr, infoLog);
      std::cerr << "shader compile error:" << infoLog << std::endl;
      return 0;
    }
    return id;
  }
  static GLuint createShaderProgram(const char *vCode, const char *fCode) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vCode);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fCode);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(program, 512, nullptr, infoLog);
      std::cerr << "Program link error: " << infoLog << std::endl;
      glDeleteShader(vs);
      glDeleteShader(fs);
      return 0;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
  }
};
