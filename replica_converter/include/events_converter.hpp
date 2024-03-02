#pragma once

#include <filesystem>

#include <esim/visualization/rosbag_writer.hpp>

#include "npy.hpp"

namespace events_converter_node {

namespace fs = std::filesystem;
using event_camera_simulator::ColorImage;
using event_camera_simulator::ColorImagePtr;
using event_camera_simulator::ColorImagePtrVector;
using event_camera_simulator::Event;
using event_camera_simulator::Events;
using event_camera_simulator::EventsVector;
using event_camera_simulator::RosbagWriter;
using event_camera_simulator::Time;
using npy::npy_data;

class EventsConverter
{
public:
  EventsConverter(fs::path input_dir, fs::path output_path, size_t max_events_rate);
  ~EventsConverter() = default;

private:
  bool readEvents(fs::path events_path);
  bool findImages(fs::path images_dir);
  // bool readCalibration(fs::path calib_path);
  bool readImagesTimestamps(fs::path timestamps_path);
  void initCameraRig();
  void sliceEvents();
  void run();
  void publishData(const Time&                time,
                   const EventsVector&        events,
                   const ColorImagePtrVector& images,
                   const ze::CameraRig::Ptr&  camera_rig);

private:
  fs::path  input_dir_;
  fs::path  images_dir_;
  fs::path  output_path_;

  // fs::path  calib_path_;
  fs::path  times_path_;
  fs::path  events_path_;

  std::vector<fs::path>  images_paths_vector_;
  std::vector<double> times_vector_;

  npy_data<double>    events_data_;
  size_t              num_events_;
  size_t              max_events_rate_;

  EventsVector events_vector_;

  ze::CameraRig::Ptr  camera_rig_;
  RosbagWriter::Ptr   rosbag_writer_;
};

}  // namespace events_converter_node
