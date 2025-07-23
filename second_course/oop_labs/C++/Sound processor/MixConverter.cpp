#include "MixConverter.h"
#include <iostream>


MixConverter::MixConverter(const std::string& otherFilePath, int startSecond) : otherFilePath(otherFilePath), startSecond(startSecond) {}


void MixConverter::apply(WAVFile& wavFile) {

    WAVFile otherWavFile(otherFilePath);
    otherWavFile.read();
    startSample = startSecond * wavFile.getSampleRate();
    auto& mainSamples = wavFile.getSamples();
    const auto& otherSamples = otherWavFile.getSamples();
    
    int mainSize = mainSamples.size();
    int otherSize = otherSamples.size();

    for (int i = 0; i < otherSize && (startSample + i) < mainSize; ++i) {
        mainSamples[startSample + i] = (mainSamples[startSample + i] + otherSamples[i]) / 2;
    }
    std::cout << "MixConverter: mixed file " << wavFile.getFilePath() << " with file " << otherFilePath << " from "<< startSecond<< " sec"<<std::endl;
}
