#include "Converter.h"
#include "WAVFile.h"

class ReverseConverter : public Converter {
public:
    ReverseConverter(int startSecond, int endSecond);
    virtual ~ReverseConverter() = default;
    void apply(WAVFile& wavFile) override;
private:
    int startSecond;
    int endSecond;
    int startSample;
    int endSample;
};