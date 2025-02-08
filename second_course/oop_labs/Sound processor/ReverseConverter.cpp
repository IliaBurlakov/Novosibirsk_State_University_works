#include "ReverseConverter.h"
#include <iostream>

ReverseConverter::ReverseConverter(int startSecond, int endSecond) : startSecond(startSecond), endSecond(endSecond) {}

void ReverseConverter::apply(WAVFile& wavFile) {
    auto& samples = wavFile.getSamples();
    int numSamples = samples.size();
    startSample = startSecond * wavFile.getSampleRate();
    endSample = endSecond * wavFile.getSampleRate();
    for (int i = 0; i < ((endSample-startSample) / 2) && i < numSamples; ++i) {
        short t = samples[startSample + i];
        samples[startSample + i] = samples[endSample-i];
        samples[endSample-i] = t;
    }
    std::cout << "ReverseConverter: reversed file " << wavFile.getFilePath() << " from " << startSecond << " to "<< endSecond<< " sec" <<std::endl;
}
