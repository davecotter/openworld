#include "sos/splat_import.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "sos/hash.hpp"
#include "sos/splat_asset.hpp"

namespace sos {

namespace {

float DecodeSplatRot(uint8_t v) {
  return (static_cast<float>(v) / 255.f) * 2.f - 1.f;
}

bool ParseSplatBinary(const std::vector<uint8_t>& raw, std::vector<CanonicalSplat>& out) {
  if (raw.size() % 32 != 0) {
    return false;
  }
  const size_t count = raw.size() / 32;
  out.resize(count);
  for (size_t i = 0; i < count; ++i) {
    const auto* p = raw.data() + i * 32;
    std::memcpy(out[i].pos, p, sizeof(float) * 3);
    std::memcpy(out[i].scale, p + 12, sizeof(float) * 3);
    out[i].color[0] = p[24];
    out[i].color[1] = p[25];
    out[i].color[2] = p[26];
    out[i].color[3] = p[27];
    out[i].rot[0] = DecodeSplatRot(p[28]);
    out[i].rot[1] = DecodeSplatRot(p[29]);
    out[i].rot[2] = DecodeSplatRot(p[30]);
    out[i].rot[3] = DecodeSplatRot(p[31]);
    out[i].opacity = static_cast<float>(out[i].color[3]) / 255.f;
  }
  return true;
}

std::string Trim(const std::string& s) {
  size_t start = 0;
  while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
    ++start;
  }
  size_t end = s.size();
  while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
    --end;
  }
  return s.substr(start, end - start);
}

bool ParsePlyAscii(const std::string& text, std::vector<CanonicalSplat>& out) {
  std::istringstream in(text);
  std::string line;
  if (!std::getline(in, line) || Trim(line) != "ply") {
    return false;
  }

  bool ascii = false;
  size_t vertex_count = 0;
  std::vector<std::string> props;
  while (std::getline(in, line)) {
    line = Trim(line);
    if (line.rfind("format ascii", 0) == 0) {
      ascii = true;
    } else if (line.rfind("element vertex", 0) == 0) {
      vertex_count = static_cast<size_t>(std::stoul(line.substr(14)));
    } else if (line.rfind("property", 0) == 0) {
      props.push_back(line.substr(9));
    } else if (line == "end_header") {
      break;
    }
  }
  if (!ascii || vertex_count == 0) {
    return false;
  }

  auto find_index = [&](const char* name) -> int {
    for (size_t i = 0; i < props.size(); ++i) {
      if (props[i].find(name) != std::string::npos) {
        return static_cast<int>(i);
      }
    }
    return -1;
  };

  const int ix = find_index(" x");
  const int iy = find_index(" y");
  const int iz = find_index(" z");
  if (ix < 0 || iy < 0 || iz < 0) {
    return false;
  }
  const int is0 = find_index("scale_0");
  const int is1 = find_index("scale_1");
  const int is2 = find_index("scale_2");
  const int ir0 = find_index("rot_0");
  const int ir1 = find_index("rot_1");
  const int ir2 = find_index("rot_2");
  const int ir3 = find_index("rot_3");
  const int iop = find_index("opacity");

  out.clear();
  out.reserve(vertex_count);
  for (size_t v = 0; v < vertex_count; ++v) {
    if (!std::getline(in, line)) {
      return false;
    }
    std::istringstream row(line);
    std::vector<float> values;
    float val = 0.f;
    while (row >> val) {
      values.push_back(val);
    }
    if (values.size() < props.size()) {
      continue;
    }
    CanonicalSplat splat{};
    splat.pos[0] = values[static_cast<size_t>(ix)];
    splat.pos[1] = values[static_cast<size_t>(iy)];
    splat.pos[2] = values[static_cast<size_t>(iz)];
    if (is0 >= 0) {
      splat.scale[0] = std::exp(values[static_cast<size_t>(is0)]);
      splat.scale[1] = std::exp(values[static_cast<size_t>(is1)]);
      splat.scale[2] = std::exp(values[static_cast<size_t>(is2)]);
    } else {
      splat.scale[0] = splat.scale[1] = splat.scale[2] = 0.01f;
    }
    if (ir0 >= 0) {
      splat.rot[0] = values[static_cast<size_t>(ir0)];
      splat.rot[1] = values[static_cast<size_t>(ir1)];
      splat.rot[2] = values[static_cast<size_t>(ir2)];
      splat.rot[3] = values[static_cast<size_t>(ir3)];
    }
    if (iop >= 0) {
      splat.opacity = 1.f / (1.f + std::exp(-values[static_cast<size_t>(iop)]));
      splat.color[3] = static_cast<uint8_t>(std::clamp(splat.opacity * 255.f, 0.f, 255.f));
    } else {
      splat.opacity = 1.f;
      splat.color[3] = 255;
    }
    splat.color[0] = splat.color[1] = splat.color[2] = 200;
    out.push_back(splat);
  }
  return !out.empty();
}

}  // namespace

SplatImportResult import_splat_file(const std::string& path) {
  SplatImportResult result;
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    result.error = "failed to open file";
    return result;
  }

  std::vector<uint8_t> raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  if (raw.empty()) {
    result.error = "empty file";
    return result;
  }

  std::vector<CanonicalSplat> splats;
  const auto lower = path.size() >= 6 ? path.substr(path.size() - 6) : path;
  const bool is_splat = path.size() >= 6 && path.substr(path.size() - 6) == ".splat";
  const bool is_ply = path.size() >= 4 && path.substr(path.size() - 4) == ".ply";

  if (is_splat) {
    if (!ParseSplatBinary(raw, splats)) {
      result.error = "invalid .splat binary";
      return result;
    }
  } else if (is_ply || (raw.size() >= 3 && std::string(raw.begin(), raw.begin() + 3) == "ply")) {
    if (!ParsePlyAscii(std::string(raw.begin(), raw.end()), splats)) {
      result.error = "unsupported or invalid PLY";
      return result;
    }
  } else {
    result.error = "unsupported file extension";
    return result;
  }

  result.canonical_blob = encode_splat_blob(splats);
  result.content_hash = hash_to_hex(xxhash64(result.canonical_blob.data(), result.canonical_blob.size()));
  result.splat_count = static_cast<uint32_t>(splats.size());
  result.ok = true;
  return result;
}

}  // namespace sos
