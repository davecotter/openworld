#pragma once

#include <GLES3/gl3.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "sos/splat_decode.hpp"

namespace sos::quest {

class GlesSplatRenderer {
 public:
  GlesSplatRenderer();
  ~GlesSplatRenderer();

  bool loadFromBlob(const uint8_t* data, size_t size);
  void draw(const float viewProjection[16], const float cameraPos[3]);

  size_t splat_count() const { return splats_.size(); }

 private:
  bool initShaders();
  void uploadInstances();

  std::vector<SplatInstance> splats_;
  std::vector<float> sorted_depths_;
  std::vector<uint32_t> sorted_indices_;
  GLuint program_ = 0;
  GLuint vao_ = 0;
  GLuint instance_vbo_ = 0;
  bool ready_ = false;
};

}  // namespace sos::quest
