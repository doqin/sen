#pragma once
#include <string>

class FileReader {
private:
    std::string content;

    void readFile(const std::string& filename);
    static bool isValidUtf8(const std::string& str);
public:
    explicit FileReader(const std::string& filename) { readFile(filename); }
    const std::string& getContent() const { return content; }
};