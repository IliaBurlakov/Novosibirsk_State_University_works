#include "MuteConverter.h"
#include <iostream>
MuteConverter::MuteConverter(int startSecond, int endSecond) : startSecond(startSecond), endSecond(endSecond) {}

void MuteConverter::apply(WAVFile& wavFile) {
    auto& samples = wavFile.getSamples();
    int numSamples = samples.size();
    startSample = startSecond * wavFile.getSampleRate();
    endSample = endSecond * wavFile.getSampleRate();
    for (int i = startSample; i < endSample && i < numSamples; ++i) {
        samples[i] = 0;
    }
    std::cout << "MuteConverter: muted file " << wavFile.getFilePath() << " from " << startSecond << " to "<< endSecond<< " sec" <<std::endl;
}
