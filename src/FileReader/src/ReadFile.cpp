#include <FileReader.hpp>
#include <fstream>
#include <iterator>
#include <stdexcept>

void FileReader::readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    throw std::runtime_error("FileReader::Failed to open file: " + filename);

  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());

  if (!isValidUtf8(content)) {
    throw std::runtime_error("FileReader::File is not valid UTF-8: " +
                             filename);
  }
}

bool FileReader::isValidUtf8(const std::string &str) {
  size_t i = 0;
  while (i < str.size()) {
    unsigned char c = static_cast<unsigned char>(str[i]);
    int bytes = 0;

    if ((c & 0x80) == 0) {
      bytes = 1; // ASCII
    } else if ((c & 0xE0) == 0xC0) {
      bytes = 2; // 2-byte UTF-8
    } else if ((c & 0xF0) == 0xE0) {
      bytes = 3; // 3-byte UTF-8
    } else if ((c & 0xF8) == 0xF0) {
      bytes = 4; // 4-byte UTF-8
    } else {
      return false;
    }

    if (i + bytes > str.size())
      return false;

    for (int j = 1; j < bytes; ++j) {
      if ((static_cast<unsigned char>(str[i + j]) & 0xC0) != 0x80) {
        return false; // Not a valid continuation byte
      }
    }
    i += bytes;
  }
  return true;
}