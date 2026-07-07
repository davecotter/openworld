#include "render/GlesSplatRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace sos::quest {

namespace {

const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec2 aCorner;
layout(location = 1) in vec3 iPos;
layout(location = 2) in vec3 iScale;
layout(location = 3) in vec4 iRot;
layout(location = 4) in vec4 iColor;
uniform mat4 uViewProjection;
out vec4 vColor;
void main() {
  vec3 world = iPos + vec3(aCorner.x * iScale.x, aCorner.y * iScale.y, 0.0);
  gl_Position = uViewProjection * vec4(world, 1.0);
  vColor = iColor;
})";

const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec4 vColor;
out vec4 fragColor;
void main() {
  fragColor = vColor;
})";

GLuint Compile(GLenum type, const char* src) {
  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  return shader;
}

}  // namespace

GlesSplatRenderer::GlesSplatRenderer() {
  initShaders();
}

GlesSplatRenderer::~GlesSplatRenderer() {
  if (instance_vbo_) {
    glDeleteBuffers(1, &instance_vbo_);
  }
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
  }
  if (program_) {
    glDeleteProgram(program_);
  }
}

bool GlesSplatRenderer::initShaders() {
  const GLuint vs = Compile(GL_VERTEX_SHADER, kVertexShader);
  const GLuint fs = Compile(GL_FRAGMENT_SHADER, kFragmentShader);
  program_ = glCreateProgram();
  glAttachShader(program_, vs);
  glAttachShader(program_, fs);
  glLinkProgram(program_);
  glDeleteShader(vs);
  glDeleteShader(fs);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  const float quad[8] = {-1, -1, 1, -1, -1, 1, 1, 1};
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glGenBuffers(1, &instance_vbo_);
  return program_ != 0;
}

bool GlesSplatRenderer::loadFromBlob(const uint8_t* data, size_t size) {
  ready_ = decode_splat_instances(data, size, splats_);
  if (ready_) {
    uploadInstances();
  }
  return ready_;
}

void GlesSplatRenderer::uploadInstances() {
  std::vector<float> packed;
  packed.reserve(splats_.size() * 14);
  for (const auto& s : splats_) {
    packed.insert(packed.end(), s.pos, s.pos + 3);
    packed.insert(packed.end(), s.scale, s.scale + 3);
    packed.insert(packed.end(), s.rot, s.rot + 4);
    packed.insert(packed.end(), s.color, s.color + 4);
  }
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glBufferData(GL_ARRAY_BUFFER, packed.size() * sizeof(float), packed.data(), GL_STATIC_DRAW);
  const GLsizei stride = static_cast<GLsizei>(14 * sizeof(float));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
  glVertexAttribDivisor(1, 1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));
  glVertexAttribDivisor(3, 1);
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(10 * sizeof(float)));
  glVertexAttribDivisor(4, 1);
}

void GlesSplatRenderer::draw(const float viewProjection[16], const float cameraPos[3]) {
  if (!ready_ || splats_.empty()) {
    return;
  }
  sorted_indices_.resize(splats_.size());
  for (size_t i = 0; i < splats_.size(); ++i) {
    sorted_indices_[i] = static_cast<uint32_t>(i);
  }
  std::sort(sorted_indices_.begin(), sorted_indices_.end(), [&](uint32_t a, uint32_t b) {
    const auto& pa = splats_[a].pos;
    const auto& pb = splats_[b].pos;
    const float da = (pa[0] - cameraPos[0]) * (pa[0] - cameraPos[0]) +
                     (pa[1] - cameraPos[1]) * (pa[1] - cameraPos[1]) +
                     (pa[2] - cameraPos[2]) * (pa[2] - cameraPos[2]);
    const float db = (pb[0] - cameraPos[0]) * (pb[0] - cameraPos[0]) +
                     (pb[1] - cameraPos[1]) * (pb[1] - cameraPos[1]) +
                     (pb[2] - cameraPos[2]) * (pb[2] - cameraPos[2]);
    return da > db;
  });

  glUseProgram(program_);
  glUniformMatrix4fv(glGetUniformLocation(program_, "uViewProjection"), 1, GL_FALSE, viewProjection);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindVertexArray(vao_);
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(splats_.size()));
}

}  // namespace sos::quest
