#include "traffic_lights_timetable.hpp"

#include "3party/omim/stl_helpers.hpp"

#include <fstream>
#include <json/json.h>

using namespace std;

namespace hack_days {
namespace traffic_lights {
namespace {
Record ParseRecord(Json::Value const &record) {
  Record res;
  res.start = record.get("start", 0).asInt();
  res.stop = record.get("stop", 0).asInt();
  // todo assert stop > start
  string type = record.get("type", "").asString();
  base::ToLower(type);
  if (type == "disabled") {
    res.type = RuleType::Disabled;
  } else if (type == "unknown") {
    res.type = RuleType::Unknown;
  } else if (type == "normal") {
    res.type = RuleType::Normal;
    res.starts_with_green = record.get("starts_with_green", true).asBool();
    res.green_duration = record.get("green_duration", 0).asInt();
    // todo assert > 0
    res.red_duration = record.get("red_duration", 0).asInt();
    // todo assert > 0
  } else {
    // todo assert
  }
  return res;
}

DayRecord ParseDayRecord(Json::Value const &records) {
  DayRecord res;
  res.records.reserve(records.size());
  for (auto const &record : records) {
    res.records.push_back(ParseRecord(record));
  }
  return res;
}

WeekdaysRecord ParseWeekdaysRecord(Json::Value const &record) {
  WeekdaysRecord res;
  Json::Value weekdays = record["weekdays"];
  for (auto const &weekday : weekdays) {
    string wd = weekday.asString();
    base::ToLower(wd);
    if (wd == "mo") {
      res.weekdays.push_back(Weekday::Mo);
    } else if (wd == "tu") {
      res.weekdays.push_back(Weekday::Tu);
    } else if (wd == "we") {
      res.weekdays.push_back(Weekday::We);
    } else if (wd == "th") {
      res.weekdays.push_back(Weekday::Th);
    } else if (wd == "fr") {
      res.weekdays.push_back(Weekday::Fr);
    } else if (wd == "sa") {
      res.weekdays.push_back(Weekday::Sa);
    } else if (wd == "su") {
      res.weekdays.push_back(Weekday::Su);
    } else {
      // todo assert
    }
  }
  res.record = ParseDayRecord(record["day_record"]);
  return res;
}

} // namespace

TrafficLightsTimetable::TrafficLightsTimetable(std::string jsonFileName) {
  // We may have too big json for all world here but it's ok for MVP
  std::ifstream jsonFile(jsonFileName);
  Json::Value root;
  jsonFile >> root;

  const Json::Value records = root["traffic_lights_timetables"];
  for (auto const &record : records) {
    TrafficLightsId id = record.get("id", 0).asUInt64();
    if (id == 0) {
      // TODO warning
      return;
    }

    SingleTrafficLightsTimetable singleTimetable;
    const Json::Value direction_rules = record["direction_rules"];
    for (auto const &rule : direction_rules) {
      RoadId from;
      Json::Value fromId = rule["from_id"];
      from.road_osm_id = fromId.get("road", 0).asUInt64();
      from.segment_number = fromId.get("segment", 0).asUInt64();
      RoadId to;
      Json::Value toId = rule["to_id"];
      to.road_osm_id = toId.get("road", 0).asUInt64();
      to.segment_number = toId.get("segment", 0).asUInt64();

      Timetable directionTimetable;
      Json::Value timetable = rule["timetable"];
      Json::Value holidayRecord = timetable["holiday_record"];
      if (!holidayRecord.empty()) {
        directionTimetable.holidays_timetable = ParseDayRecord(holidayRecord);
      }
      Json::Value weekdaysRecords = timetable["weekdays_records"];
      directionTimetable.weekdays_timatable.reserve(weekdaysRecords.size());
      for (auto const &weekdaysRecord : weekdaysRecords) {
        directionTimetable.weekdays_timatable.push_back(
            ParseWeekdaysRecord(weekdaysRecord));
      }

      singleTimetable.timetables.emplace_back(std::move(from), std::move(to),
                                              std::move(directionTimetable));
    }
    timetables.emplace(id, std::move(singleTimetable));
  }
}
} // namespace traffic_lights
} // namespace hack_days