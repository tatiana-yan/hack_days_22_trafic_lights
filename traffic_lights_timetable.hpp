#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace hack_days {
namespace traffic_lights {
enum class RuleType { Normal, Disabled, Unknown };

struct Record {
  bool operator==(const Record &other) const {
    return std::make_tuple(start, stop, type, starts_with_green, green_duration,
                           red_duration) ==
           std::make_tuple(start, stop, type, starts_with_green, green_duration,
                           red_duration);
  }

  uint32_t start = 0;
  uint32_t stop = 0;
  RuleType type = RuleType::Unknown;
  std::optional<bool> starts_with_green = true;
  std::optional<uint16_t> green_duration = 0; // have for type normal
  std::optional<uint16_t> red_duration = 0;   // have for type normal
};

struct DayRecord {
  bool operator==(const DayRecord &other) const {
    return records == other.records;
  }

  std::vector<Record> records;
};

enum class Weekday : uint8_t {
  Mo = 1U << 0U,
  Tu = 1U << 1U,
  We = 1U << 2U,
  Th = 1U << 3U,
  Fr = 1U << 4U,
  Sa = 1U << 5U,
  Su = 1U << 6U
};

struct WeekdaysRecord {
  bool operator==(const WeekdaysRecord &other) const {
    return std::make_tuple(weekdays, record) ==
           std::make_tuple(other.weekdays, other.record);
  }

  std::vector<Weekday> weekdays;
  DayRecord record;
};

struct Timetable {
  enum class Status { Green, Red, Unknown, Disabled };

  Status GetStatus(bool holiday, Weekday weekday, uint32_t time_sec) const {
    return Status::Unknown;
  } // TODO

  bool operator==(const Timetable &other) const {
    return std::make_tuple(holidays_timetable, weekdays_timatable) ==
           std::make_tuple(other.holidays_timetable, other.weekdays_timatable);
  }

  std::optional<DayRecord> holidays_timetable;
  std::vector<WeekdaysRecord> weekdays_timatable;
};

struct RoadId {
  RoadId() = default;
  RoadId(uint64_t road_osm_id_, uint16_t segment_number_)
      : road_osm_id(road_osm_id_), segment_number(segment_number_) {}

  bool operator==(RoadId const &other) const {
    return std::make_tuple(road_osm_id, segment_number) ==
           std::make_tuple(other.road_osm_id, other.segment_number);
  };

  uint64_t road_osm_id = 0;
  uint16_t segment_number = 0;
};

using TrafficLightsId =
    uint64_t; // We have 1'571'105 osm nodes and <100 ways + relation. Let's use
              // node id and ignore ways and relations.

struct SingleDirectionTimetable {
  SingleDirectionTimetable() = default;

  SingleDirectionTimetable(RoadId &&from_, RoadId &&to_, Timetable &&timetable_)
      : from(from_), to(to_), timetable(timetable_) {}

  bool operator==(SingleDirectionTimetable const &other) const {
    return std::make_tuple(from, to, timetable) ==
           std::make_tuple(other.from, other.to, other.timetable);
  }

  RoadId from;
  RoadId to;
  Timetable timetable;
};

struct SingleTrafficLightsTimetable {
  bool operator==(SingleTrafficLightsTimetable const &other) const {
    return timetables == other.timetables;
  }
  // todo consider map instead or vector
  std::vector<SingleDirectionTimetable> timetables;
};

struct TrafficLightsTimetable {
  TrafficLightsTimetable() = default;
  TrafficLightsTimetable(std::string jsonFileName);

  bool operator==(TrafficLightsTimetable const &other) const {
    return timetables == other.timetables;
  }

  std::unordered_map<TrafficLightsId, SingleTrafficLightsTimetable> timetables;
};

} // namespace traffic_lights
} // namespace hack_days
