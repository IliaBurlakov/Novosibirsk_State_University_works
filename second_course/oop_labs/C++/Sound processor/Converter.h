#pragma once
#include "WAVFile.h"

class Converter {
public:
    virtual void apply(WAVFile& wavFile) = 0;
    virtual ~Converter() = default;
};