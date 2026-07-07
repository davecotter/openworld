#include "sos/zip_io.hpp"

#include <cstring>
#include <fstream>

namespace sos {

namespace {

#pragma pack(push, 1)
struct LocalHeader {
  uint32_t signature = 0x04034b50;
  uint16_t version = 20;
  uint16_t flags = 0;
  uint16_t method = 0;
  uint16_t mod_time = 0;
  uint16_t mod_date = 0;
  uint32_t crc32 = 0;
  uint32_t compressed_size = 0;
  uint32_t uncompressed_size = 0;
  uint16_t name_len = 0;
  uint16_t extra_len = 0;
};

struct CentralHeader {
  uint32_t signature = 0x02014b50;
  uint16_t version_made = 20;
  uint16_t version_needed = 20;
  uint16_t flags = 0;
  uint16_t method = 0;
  uint16_t mod_time = 0;
  uint16_t mod_date = 0;
  uint32_t crc32 = 0;
  uint32_t compressed_size = 0;
  uint32_t uncompressed_size = 0;
  uint16_t name_len = 0;
  uint16_t extra_len = 0;
  uint16_t comment_len = 0;
  uint16_t disk_start = 0;
  uint16_t internal_attr = 0;
  uint32_t external_attr = 0;
  uint32_t local_offset = 0;
};

struct EndRecord {
  uint32_t signature = 0x06054b50;
  uint16_t disk = 0;
  uint16_t central_disk = 0;
  uint16_t entries_disk = 0;
  uint16_t entries_total = 0;
  uint32_t central_size = 0;
  uint32_t central_offset = 0;
  uint16_t comment_len = 0;
};
#pragma pack(pop)

uint32_t Crc32(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int j = 0; j < 8; ++j) {
      const uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

}  // namespace

bool zip_write_store(const std::string& path, const std::vector<ZipEntry>& entries) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    return false;
  }

  std::vector<CentralHeader> central;
  central.reserve(entries.size());
  uint32_t offset = 0;

  for (const auto& entry : entries) {
    LocalHeader local{};
    local.crc32 = Crc32(entry.data.data(), entry.data.size());
    local.compressed_size = static_cast<uint32_t>(entry.data.size());
    local.uncompressed_size = local.compressed_size;
    local.name_len = static_cast<uint16_t>(entry.name.size());

    out.write(reinterpret_cast<const char*>(&local), sizeof(local));
    out.write(entry.name.data(), static_cast<std::streamsize>(entry.name.size()));

    CentralHeader cent{};
    cent.crc32 = local.crc32;
    cent.compressed_size = local.compressed_size;
    cent.uncompressed_size = local.uncompressed_size;
    cent.name_len = local.name_len;
    cent.local_offset = offset;
    central.push_back(cent);

    offset += static_cast<uint32_t>(sizeof(local) + entry.name.size());
    out.write(reinterpret_cast<const char*>(entry.data.data()),
              static_cast<std::streamsize>(entry.data.size()));
    offset += local.compressed_size;
  }

  const uint32_t central_offset = offset;
  uint32_t central_size = 0;
  for (size_t i = 0; i < entries.size(); ++i) {
    const auto& entry = entries[i];
    out.write(reinterpret_cast<const char*>(&central[i]), sizeof(CentralHeader));
    out.write(entry.name.data(), static_cast<std::streamsize>(entry.name.size()));
    central_size += static_cast<uint32_t>(sizeof(CentralHeader) + entry.name.size());
  }

  EndRecord end{};
  end.entries_disk = static_cast<uint16_t>(entries.size());
  end.entries_total = end.entries_disk;
  end.central_size = central_size;
  end.central_offset = central_offset;
  out.write(reinterpret_cast<const char*>(&end), sizeof(end));
  return out.good();
}

bool zip_read_store(const std::string& path, std::vector<ZipEntry>& out_entries) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return false;
  }

  in.seekg(0, std::ios::end);
  const auto file_size = static_cast<size_t>(in.tellg());
  in.seekg(0, std::ios::beg);

  std::vector<uint8_t> file(file_size);
  in.read(reinterpret_cast<char*>(file.data()), static_cast<std::streamsize>(file.size()));
  if (!in) {
    return false;
  }

  size_t pos = 0;
  out_entries.clear();
  while (pos + sizeof(LocalHeader) <= file.size()) {
    LocalHeader local{};
    std::memcpy(&local, file.data() + pos, sizeof(local));
    if (local.signature != 0x04034b50) {
      break;
    }
    pos += sizeof(local);
    if (pos + local.name_len + local.extra_len > file.size()) {
      return false;
    }
    ZipEntry entry;
    entry.name.assign(reinterpret_cast<const char*>(file.data() + pos), local.name_len);
    pos += local.name_len + local.extra_len;
    if (pos + local.compressed_size > file.size()) {
      return false;
    }
    entry.data.assign(file.begin() + static_cast<long>(pos),
                      file.begin() + static_cast<long>(pos + local.compressed_size));
    pos += local.compressed_size;
    out_entries.push_back(std::move(entry));
  }
  return !out_entries.empty();
}

}  // namespace sos
