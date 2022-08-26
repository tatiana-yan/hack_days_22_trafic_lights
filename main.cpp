#include "3party/omim/byte_stream.hpp"
//#include "varint.hpp"
#include "traffic_lights_timetable.hpp"
#include "traffic_lights_timetable_serdes.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace hack_days::traffic_lights;

namespace {
size_t filesize(const char *filename) {
  std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
  return in.tellg();
}
} // namespace

int main(int argc, char **argv) {
  TrafficLightsTimetable timetable("data/traffic_lights_timetable.json");
  vector<unsigned char> data;
  PushBackByteSink<vector<uint8_t>> dst(data);
  TrafficLightsTimatableSerDes::Serialize(dst, timetable);

  // we should have graph data in runtime, otherwise weneed to save road ids
  auto const cb =
      [](TrafficLightsId id) -> std::vector<std::pair<RoadId, RoadId>> {
    if (id == 2496113387) {
      RoadId from(204056329, 4);
      RoadId to(204056329, 5);
      return {{from, to}};
    }
    return {};
  };

  ArrayByteSource src(&data[0]);
  TrafficLightsTimetable deserialized =
      TrafficLightsTimatableSerDes::Deserialize(src, cb);

  cout << boolalpha << "data equals: " << (timetable == deserialized) << endl;

  cout << "json size: " << filesize("data/traffic_lights_timetable.json")
       << endl
       << "compressed size: " << data.size() << endl;
}
