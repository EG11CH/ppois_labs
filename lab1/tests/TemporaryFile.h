#pragma once

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

class TemporaryFile {
  public:
    TemporaryFile(const std::string& name, const std::string& content)
        : path_(std::filesystem::temp_directory_path() / ("lab1_" + name + ".txt")) {
        std::ofstream output(path_);
        output << content;
        output.close();
        if (!output) {
            throw std::runtime_error("Cannot create test fixture");
        }
    }
    ~TemporaryFile() {
        std::error_code ignored;
        std::filesystem::remove(path_, ignored);
    }
    std::string path() const {
        return path_.string();
    }

  private:
    std::filesystem::path path_;
};
