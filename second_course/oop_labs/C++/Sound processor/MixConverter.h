#pragma once
#include "Converter.h"
#include "WAVFile.h"
#include <string>

class MixConverter : public Converter {
public:
    MixConverter(const std::string& otherFilePath, int startSecond);
    void apply(WAVFile& wavFile) override;
    virtual ~MixConverter() = default;
private:
    std::string otherFilePath;
    int startSecond;
    int startSample;
    int sampleRate;
};