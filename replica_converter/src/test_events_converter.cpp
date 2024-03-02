#include <iostream>
#include <filesystem>
#include "events_converter.hpp"

int main(int argc, char** argv)
{
    using namespace events_converter_node;
    namespace fs = std::filesystem;

    if(argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input_dir> <output_path> <max_events_rate>\n";
        return 1;
    }

    fs::path input_dir = argv[1];
    fs::path output_path = argv[2];
    size_t max_events_rate = std::stoul(argv[3]);

    if(!fs::exists(input_dir))
    {
        std::cerr << "Directory does not exist: " << input_dir << "\n";
        return 1;
    }
    else if(!fs::is_directory(input_dir))
    {
        std::cerr << "Not a directory:" << input_dir << "\n";
        return 1;
    }

    if(!fs::exists(output_path.parent_path()))
    {
        fs::create_directories(output_path.parent_path());
    }
    else if(fs::exists(output_path))
    {
        std::cerr << "Path not empty! " << output_path << "\n";
        return 1;
    }

    std::cout << "Input dir: " << input_dir << std::endl
              << "Output path: " << output_path << std::endl
              << "Max events rate: " << max_events_rate << std::endl;

    EventsConverter converter(input_dir, output_path, max_events_rate);

    return 0;
}
