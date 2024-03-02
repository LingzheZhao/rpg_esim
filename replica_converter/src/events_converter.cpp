#include <set>
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <ze/cameras/camera_impl.hpp>

#include "events_converter.hpp"

namespace events_converter_node {

EventsConverter::EventsConverter(fs::path input_dir,
                                 fs::path output_path)
: input_dir_(input_dir), output_path_(output_path)
{
  images_dir_ = input_dir / "Gray";
  // calib_path_ = input_dir / "calib.yaml";
  times_path_ = input_dir / "poses_ts.txt";
  events_path_ = input_dir / "event_threshold_0.1" / "gray_events_data.npy";

  rosbag_writer_ = std::make_shared<RosbagWriter>(output_path_, 1);

  // readCalibration(calib_path_);
  initCameraRig();

  readImages(images_dir_);
  readImagesTimestamps(times_path_);
  readEvents(events_path_);
  assert(images_vector_.size() == times_vector_.size());
  sliceEvents();
  run();
}

void EventsConverter::run()
{
  const size_t NUM_CHUNCKS = events_vector_.size();
  for(size_t i = 0; i < NUM_CHUNCKS; i++)
  {
    std::cout << "Exporting "
              << i << " / " << NUM_CHUNCKS
              << " data chuncks to rosbag...\n";

    EventsVector events_chunck;
    events_chunck.emplace_back(events_vector_[i]);

    ColorImagePtrVector image_chunck;
    image_chunck.emplace_back(images_vector_[i]);

    publishData(static_cast<Time>(times_vector_[i] * 1e9),
                events_chunck,
                image_chunck,
                camera_rig_);
  }  
}

void EventsConverter::publishData(const Time&                time,
                                  const EventsVector&        events,
                                  const ColorImagePtrVector& images,
                                  const ze::CameraRig::Ptr&  camera_rig)
{
  // Publish the new data (events, images, depth maps, poses, point clouds, etc.)
  if(!events.empty())
  {
    rosbag_writer_->eventsCallback(events);
  }
  if(!images.empty())
  {
    rosbag_writer_->imageCallback(images, time);
  }
  if(camera_rig)
  {
    rosbag_writer_->cameraInfoCallback(camera_rig, time);
  }
}

void EventsConverter::initCameraRig()
{
  ze::CameraVector cameras;
  cameras.emplace_back(std::make_shared<ze::PinholeCamera>(
    ze::createPinholeCamera(768, 480, 384.0, 384.0, 384.0, 240.0)));
  ze::TransformationVector extrinsics;
  extrinsics.push_back(ze::Transformation());
  camera_rig_ = std::make_shared<ze::CameraRig>(extrinsics, cameras, "camera");
}

// bool EventsConverter::readCalibration(fs::path calib_path)
// {
//   try {
//     camera_rig_ = ze::cameraRigFromYaml(calib_path);
//     return true;
//   } catch (const std::exception& e) {
//     std::cerr << "Error reading calibration: " << e.what() << std::endl;
//     return false;
//   }
// }

bool EventsConverter::readEvents(fs::path events_path)
{
  if(!fs::exists(events_path)) {
    std::cerr << "File does not exist: " << events_path << std::endl;
    return false;
  }
  std::cout << "Reading events data from " << events_path << "..." << std::endl;
  events_data_ = npy::read_npy<double>(events_path);
  std::cout << "OK!" << std::endl;

  size_t npy_dims = events_data_.shape.size();
  assert(npy_dims == 2);
  size_t npy_rows = events_data_.shape[0];
  size_t npy_cols = events_data_.shape[1];
  num_events_ = npy_rows;
  assert(npy_cols == 4);

  return true;
}

bool EventsConverter::readImages(fs::path images_dir)
{
  if(!fs::exists(images_dir)) {
    std::cerr << "Directory does not exist: " << images_dir << std::endl;
    return false;
  }

  std::set<fs::path> filesSorted;
  for (const auto& entry : fs::directory_iterator(images_dir))
  {
    if (entry.path().extension() == ".png")
    {
        filesSorted.insert(entry.path());
    }
  }
  std::cout << "Found " << filesSorted.size() << "images.\n";

  for (const auto& entry : filesSorted)
  {
    std::cout << "Reading image: " << entry << std::endl;
    cv::Mat image_8u = cv::imread(entry.string());
    ColorImage image_32f;
    image_8u.convertTo(image_32f, CV_32F);
    image_32f /= 255.0;

    cv::Scalar mean_val = cv::mean(image_32f);
    std::cout << "Mean value of image: " << mean_val[0] << std::endl;

    images_vector_.emplace_back(std::make_shared<ColorImage>(image_32f));
  }
  return true;
}

bool EventsConverter::readImagesTimestamps(fs::path timestamps_path)
{
  if(!fs::exists(timestamps_path)) {
    std::cerr << "File does not exist: " << timestamps_path << "\n";
    return false;
  }

  std::ifstream file(timestamps_path);
  std::string line;
  while (std::getline(file, line)) {
    try {
      double timestamp = std::stod(line);
      times_vector_.push_back(timestamp);
    } catch (const std::invalid_argument& ia) {
      std::cerr << "Invalid argument: " << ia.what() << '\n';
      return false;
    } catch (const std::out_of_range& oor) {
      std::cerr << "Out of Range error: " << oor.what() << '\n';
      return false;
    }
  }
  return true;
}

void EventsConverter::sliceEvents()
{
  size_t start_index = 0;
  for (double timestamp : times_vector_) {
    Events events;
    while (start_index < num_events_ &&
           events_data_.data[start_index * 4 + 2] < timestamp
    ) {
      auto  x = static_cast<uint16_t>(events_data_.data[start_index * 4]);
      auto  y = static_cast<uint16_t>(events_data_.data[start_index * 4 + 1]);
      auto  t = static_cast<Time>(events_data_.data[start_index * 4 + 2] * 1e9);
      bool  pol = events_data_.data[start_index * 4 + 3] == 1 ? true : false;
      events.emplace_back(x, y, t, pol);
      ++start_index;
    }
    events_vector_.emplace_back(events);
  }

  std::cout << "Events are sliced into " << events_vector_.size() << " chuncks\n";
  assert(events_vector_.size() == times_vector_.size() - 1);
}

}  // namespace events_converter_node
