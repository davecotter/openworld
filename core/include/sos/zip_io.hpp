#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace sos {

struct ZipEntry {
  std::string name;
  std::vector<uint8_t> data;
};

bool zip_write_store(const std::string& path, const std::vector<ZipEntry>& entries);
bool zip_read_store(const std::string& path, std::vector<ZipEntry>& out_entries);

}  // namespace sos
