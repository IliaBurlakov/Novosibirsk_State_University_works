#include <vector>
#include <string>
#include <memory>
#include "Converter.h"

class SoundProcessor {
public:
    SoundProcessor(const std::vector<std::string>& inputFiles, const std::string& outputFile, const std::string& commandFile);
    void process();
    virtual ~SoundProcessor() = default;
private:
    std::vector<std::string> inputFiles;
    std::string outputFile;
    std::string commandFile;
    std::vector<std::unique_ptr<Converter>> converters;
    void parseCommands();
    void executeCommands(WAVFile& wavFile);
};