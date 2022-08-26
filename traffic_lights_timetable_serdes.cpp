#include "traffic_lights_timetable_serdes.hpp"

namespace hack_days {
namespace traffic_lights {
// static
uint8_t RecordSerDes::PackTypeAndMeta(Record const &record) {
  uint8_t res = 0;
  uint8_t const typeNormal = 1U << 0U;
  uint8_t const typeDisabled = 1U << 1U;
  uint8_t const typeUnknown = 1U << 2U;
  uint8_t const startsWithGreen = 1U << 3U;

  if (record.type == RuleType::Normal) {
    res |= typeNormal;
  } else if (record.type == RuleType::Disabled) {
    res |= typeDisabled;
  } else if (record.type == RuleType::Unknown) {
    // todo: consider not to save unknown to save some bits
    // need special processing for correct DayRecord records size for this case
    // & special processing for empty days
    res |= typeUnknown;
  }

  if (record.type == RuleType::Normal && *record.starts_with_green) {
    res |= startsWithGreen;
  }

  return res;
}

// static
void RecordSerDes::UnPackTypeAndMeta(uint8_t typeAndMeta, Record &record) {
  uint8_t const typeNormal = 1U << 0U;
  uint8_t const typeDisabled = 1U << 1U;
  uint8_t const typeUnknown = 1U << 2U;
  uint8_t const startsWithGreen = 1U << 4U;
  if (typeAndMeta & typeNormal) {
    record.type = RuleType::Normal;
  } else if (typeAndMeta & typeDisabled) {
    record.type = RuleType::Disabled;
  } else if (typeAndMeta & typeUnknown) {
    record.type = RuleType::Unknown;
  }

  if (record.type == RuleType::Normal && typeAndMeta & startsWithGreen) {
    record.starts_with_green = true;
  }
}

// static
uint8_t
WeekdaysRecordSerDes::PackWeekdays(std::vector<Weekday> const &weekdays) {
  uint8_t res = 0;
  for (auto wd : weekdays) {
    res |= static_cast<uint8_t>(wd);
  }
  return res;
}

// static
void WeekdaysRecordSerDes::UnPackWeekdays(uint8_t packed,
                                          std::vector<Weekday> &weekdays) {
  for (size_t i = 0; i < 7; i++) {
    uint8_t candidate = 1 << i;
    if (packed & candidate)
      weekdays.push_back(static_cast<Weekday>(candidate));
  }
}

// static
bool SingleTrafficLightsTimetableSerDes::lessCompare(
    SingleDirectionTimetable const &lhs, SingleDirectionTimetable const &rhs) {
  return std::make_tuple(lhs.from.road_osm_id, lhs.from.segment_number,
                         lhs.to.road_osm_id, lhs.to.segment_number) <
         std::make_tuple(rhs.from.road_osm_id, rhs.from.segment_number,
                         rhs.to.road_osm_id, rhs.to.segment_number);
}
} // namespace traffic_lights
} // namespace hack_days
