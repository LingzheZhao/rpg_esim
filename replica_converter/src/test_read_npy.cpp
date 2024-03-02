#include <iostream>
#include <iomanip>
#include <filesystem>
#include "npy.hpp"

int main(int argc, char** argv)
{
    namespace fs = std::filesystem;

    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <numpy_file_path>\n";
        return 1;
    }

    fs::path numpy_file_path = argv[1];

    if(!fs::exists(numpy_file_path)) {
        std::cerr << "File does not exist: " << numpy_file_path << "\n";
        return 1;
    }

    auto data = npy::read_npy<double>(numpy_file_path);

    std::cout << "data.shape: (";
    for(size_t i = 0; i < data.shape.size(); ++i) {
        std::cout << data.shape[i];
        if(i != data.shape.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ")" << std::endl;

    std::cout << "First 20 items:\n" << std::setprecision(16);
    for(size_t i = 0; i < 20; ++i) {
        std::cout << data.data[i] << '\n';
    }
    std::cout << std::endl;

    return 0;
}
