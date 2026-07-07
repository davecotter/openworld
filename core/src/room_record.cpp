#include "sos/room_record.hpp"

#include <cstdlib>
#include <sstream>

namespace sos {

namespace {

std::string EscapeJson(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

bool ExtractString(const std::string& json, const char* key, std::string& out) {
  const std::string needle = std::string("\"") + key + "\":\"";
  const auto pos = json.find(needle);
  if (pos == std::string::npos) {
    return false;
  }
  const auto start = pos + needle.size();
  const auto end = json.find('"', start);
  if (end == std::string::npos) {
    return false;
  }
  out = json.substr(start, end - start);
  return true;
}

bool ExtractNumber(const std::string& json, const char* key, int64_t& out) {
  const std::string needle = std::string("\"") + key + "\":";
  const auto pos = json.find(needle);
  if (pos == std::string::npos) {
    return false;
  }
  const auto start = pos + needle.size();
  const auto end = json.find_first_of(",}", start);
  const auto token = json.substr(start, end - start);
  char* endptr = nullptr;
  out = std::strtoll(token.c_str(), &endptr, 10);
  return endptr != token.c_str();
}

bool ExtractDouble(const std::string& json, const char* key, double& out) {
  const std::string needle = std::string("\"") + key + "\":";
  const auto pos = json.find(needle);
  if (pos == std::string::npos) {
    return false;
  }
  const auto start = pos + needle.size();
  const auto end = json.find_first_of(",}", start);
  const auto token = json.substr(start, end - start);
  char* endptr = nullptr;
  out = std::strtod(token.c_str(), &endptr);
  return endptr != token.c_str();
}

bool ExtractFloat(const std::string& json, const char* key, float& out) {
  const std::string needle = std::string("\"") + key + "\":";
  const auto pos = json.find(needle);
  if (pos == std::string::npos) {
    return false;
  }
  const auto start = pos + needle.size();
  const auto end = json.find_first_of(",}", start);
  const auto token = json.substr(start, end - start);
  char* endptr = nullptr;
  out = std::strtof(token.c_str(), &endptr);
  return endptr != token.c_str();
}

bool ExtractBool(const std::string& json, const char* key, bool& out) {
  const std::string needle = std::string("\"") + key + "\":";
  const auto pos = json.find(needle);
  if (pos == std::string::npos) {
    return false;
  }
  const auto start = pos + needle.size();
  if (json.compare(start, 4, "true") == 0) {
    out = true;
    return true;
  }
  if (json.compare(start, 5, "false") == 0) {
    out = false;
    return true;
  }
  return false;
}

}  // namespace

std::string room_record_to_json(const RoomRecord& record) {
  std::ostringstream ss;
  ss << "{"
     << "\"swm_version\":" << RoomRecord::kSwmVersion << ","
     << "\"id\":\"" << EscapeJson(record.id) << "\","
     << "\"display_name\":\"" << EscapeJson(record.display_name) << "\","
     << "\"source_type\":\"" << EscapeJson(record.source_type) << "\","
     << "\"primary_asset_hash\":\"" << EscapeJson(record.primary_asset_hash) << "\","
     << "\"asset_kind\":\"" << EscapeJson(record.asset_kind) << "\","
     << "\"created_at_ns\":" << record.created_at_ns << ","
     << "\"updated_at_ns\":" << record.updated_at_ns << ","
     << "\"geometry_confidence\":" << record.geometry_confidence << ","
     << "\"pose\":{"
     << "\"tx\":" << record.pose[0] << ","
     << "\"ty\":" << record.pose[1] << ","
     << "\"tz\":" << record.pose[2] << ","
     << "\"qx\":" << record.pose[3] << ","
     << "\"qy\":" << record.pose[4] << ","
     << "\"qz\":" << record.pose[5] << ","
     << "\"qw\":" << record.pose[6] << "},"
     << "\"gps\":{"
     << "\"has_gps\":" << (record.has_gps ? "true" : "false") << ","
     << "\"latitude\":" << record.gps_latitude << ","
     << "\"longitude\":" << record.gps_longitude << ","
     << "\"accuracy_m\":" << record.gps_accuracy_m << "}}";
  return ss.str();
}

bool room_record_from_json(const std::string& json, RoomRecord& out) {
  RoomRecord record;
  if (!ExtractString(json, "id", record.id)) {
    return false;
  }
  ExtractString(json, "display_name", record.display_name);
  ExtractString(json, "source_type", record.source_type);
  ExtractString(json, "primary_asset_hash", record.primary_asset_hash);
  ExtractString(json, "asset_kind", record.asset_kind);
  ExtractNumber(json, "created_at_ns", record.created_at_ns);
  ExtractNumber(json, "updated_at_ns", record.updated_at_ns);
  ExtractFloat(json, "geometry_confidence", record.geometry_confidence);
  ExtractBool(json, "has_gps", record.has_gps);
  ExtractDouble(json, "latitude", record.gps_latitude);
  ExtractDouble(json, "longitude", record.gps_longitude);
  ExtractFloat(json, "accuracy_m", record.gps_accuracy_m);
  out = std::move(record);
  return true;
}

}  // namespace sos
