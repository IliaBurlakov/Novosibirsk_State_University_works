#pragma once
#include <string>
#include <vector>
#include <fstream>

class WAVFile {
public:
    WAVFile(const std::string& filePath);
    virtual ~WAVFile() = default;
    void read();

    void write(const std::string& outputPath);

    std::vector<short>& getSamples() ;
    std::string getFilePath() const;
    int getSampleRate() const;
private:
    std::string filePath;
    std::vector<short> samples;    
    const int sampleRate = 44100;
    const int numChannels = 1;          
    int dataSize;

    void readHeader(std::ifstream& inFile);
    void writeHeader(std::ofstream& outFile);
};
