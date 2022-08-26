#pragma once

#include "traffic_lights_timetable.hpp"

#include "3party/omim/varint.hpp"
#include "3party/omim/write_to_sink.hpp"

#include <algorithm>
#include <vector>

namespace hack_days {
namespace traffic_lights {
class RecordSerDes {
public:
  template <typename Writer>
  static void Serialize(Writer &writer, Record const &record) {
    uint8_t typeAndMeta = PackTypeAndMeta(record);
    WriteToSink(writer, typeAndMeta);
    WriteVarUint(writer, record.start);
    WriteVarUint(writer, record.stop - record.start);
    if (record.type == RuleType::Normal) {
      WriteVarUint(writer, *record.green_duration);
      WriteVarUint(writer, *record.red_duration);
    }
  }

  template <typename Reader> static Record Deserialize(Reader &reader) {
    Record record;
    uint8_t typeAndMeta;
    reader.Read(&typeAndMeta, 1);
    UnPackTypeAndMeta(typeAndMeta, record);
    record.start = ReadVarUint<uint32_t>(reader);
    record.stop = record.start + ReadVarUint<uint32_t>(reader);
    if (record.type == RuleType::Normal) {
      record.green_duration = ReadVarUint<uint32_t>(reader);
      record.red_duration = ReadVarUint<uint32_t>(reader);
    }
    return record;
  }

private:
  static uint8_t PackTypeAndMeta(Record const &record);
  static void UnPackTypeAndMeta(uint8_t typeAndMeta, Record &record);
};

class DayRecordSerDes {
public:
  template <typename Writer>
  static void Serialize(Writer &writer, DayRecord const &dayRecord) {
    WriteVarUint(writer, static_cast<uint32_t>(dayRecord.records.size()));
    for (auto const &record : dayRecord.records) {
      RecordSerDes::Serialize(writer, record);
    }
  }

  template <typename Reader> static DayRecord Deserialize(Reader &reader) {
    DayRecord DayRecord;
    uint32_t count = ReadVarUint<uint32_t>(reader);
    DayRecord.records.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
      DayRecord.records.emplace_back(RecordSerDes::Deserialize(reader));
    }
    return DayRecord;
  }
};

class WeekdaysRecordSerDes {
public:
  template <typename Writer>
  static void Serialize(Writer &writer, WeekdaysRecord const &record) {
    WriteToSink(writer, PackWeekdays(record.weekdays));
    DayRecordSerDes::Serialize(writer, record.record);
  }

  template <typename Reader> static WeekdaysRecord Deserialize(Reader &reader) {
    WeekdaysRecord record;
    uint8_t packed;
    reader.Read(&packed, 1);
    UnPackWeekdays(packed, record.weekdays);
    record.record = DayRecordSerDes::Deserialize(reader);
    return record;
  }

private:
  static uint8_t PackWeekdays(std::vector<Weekday> const &weekdays);
  static void UnPackWeekdays(uint8_t packed, std::vector<Weekday> &weekdays);
};

class TimetableSerDes {
public:
  template <typename Writer>
  static void Serialize(Writer &writer, Timetable const &record) {
    if (record.holidays_timetable) {
      // consider bit streams to save 7 bits here
      WriteToSink(writer, static_cast<uint8_t>(1));
      DayRecordSerDes::Serialize(writer, *record.holidays_timetable);
    } else {
      WriteToSink(writer, static_cast<uint8_t>(0));
    }
    WriteToSink(writer, static_cast<uint8_t>(record.weekdays_timatable.size()));
    for (auto weekday : record.weekdays_timatable) {
      WeekdaysRecordSerDes::Serialize(writer, weekday);
    }
  }

  template <typename Reader> static Timetable Deserialize(Reader &reader) {
    Timetable timetable;
    uint8_t haveHoliday = 0;
    reader.Read(&haveHoliday, 1);
    if (haveHoliday) {
      timetable.holidays_timetable = DayRecordSerDes::Deserialize(reader);
    }
    uint8_t weekdaysCount;
    reader.Read(&weekdaysCount, 1);
    timetable.weekdays_timatable.resize(weekdaysCount);
    for (auto &weekdays : timetable.weekdays_timatable) {
      weekdays = WeekdaysRecordSerDes::Deserialize(reader);
    }
    return timetable;
  }
};

class SingleTrafficLightsTimetableSerDes {
public:
  template <typename Writer>
  static void Serialize(Writer &writer,
                        SingleTrafficLightsTimetable timetable) {
    // We assume we have same roads ID when we build data and on runtime. Also
    // we assume that on runtime we can get list of ingoing and outgoing roads
    // for traffic lights. With these information we can have stable order of
    // timetables and avoid road ids encoding.
    std::sort(timetable.timetables.begin(), timetable.timetables.end(),
              &lessCompare);
    WriteVarUint(writer, static_cast<uint32_t>(timetable.timetables.size()));
    for (auto const &singleDirectionTimetable : timetable.timetables) {
      TimetableSerDes::Serialize(writer, singleDirectionTimetable.timetable);
    }
  }

  template <typename Reader>
  static SingleTrafficLightsTimetable
  Deserialize(Reader &reader,
              std::vector<std::pair<RoadId, RoadId>> const &directions) {
    uint32_t count = ReadVarUint<uint32_t>(reader);
    // assert count == directions.size();
    SingleTrafficLightsTimetable timetable;
    timetable.timetables.reserve(directions.size());
    for (auto const &direction : directions) {
      SingleDirectionTimetable singleDirectionTimetable;
      singleDirectionTimetable.from = direction.first;
      singleDirectionTimetable.to = direction.second;
      timetable.timetables.emplace_back(std::move(singleDirectionTimetable));
    }
    std::sort(timetable.timetables.begin(), timetable.timetables.end(),
              &lessCompare);
    for (auto &singleDirectionTimetable : timetable.timetables) {
      singleDirectionTimetable.timetable = TimetableSerDes::Deserialize(reader);
    }
    return timetable;
  }

private:
  static bool lessCompare(SingleDirectionTimetable const &lhs,
                          SingleDirectionTimetable const &rhs);
};

class TrafficLightsTimatableSerDes {
public:
  using GetRoadsCallbacks =
      std::function<std::vector<std::pair<RoadId, RoadId>>(TrafficLightsId)>;

  template <typename Writer>
  static void Serialize(Writer &writer,
                        TrafficLightsTimetable const &timetable) {
    WriteVarUint(writer, static_cast<uint32_t>(timetable.timetables.size()));
    for (auto const &kv : timetable.timetables) {
      // todo: can save some bytes if sort by id and use delta encoding
      WriteVarUint(writer, kv.first);
      SingleTrafficLightsTimetableSerDes::Serialize(writer, kv.second);
    }
  }

  template <typename Reader>
  static TrafficLightsTimetable Deserialize(Reader &reader,
                                            GetRoadsCallbacks const &cb) {
    TrafficLightsTimetable timetable;
    uint32_t count = ReadVarUint<uint32_t>(reader);
    for (uint32_t i = 0; i < count; i++) {
      TrafficLightsId id = ReadVarUint<uint64_t>(reader);
      auto const directions = cb(id);
      timetable.timetables[id] =
          SingleTrafficLightsTimetableSerDes::Deserialize(reader, directions);
    }
    return timetable;
  }
};

} // namespace traffic_lights
} // namespace hack_days