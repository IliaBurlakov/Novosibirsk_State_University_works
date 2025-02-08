#include "Converter.h"
#include "WAVFile.h"

class MuteConverter : public Converter {
public:
    MuteConverter(int startSecond, int endSecond);
    void apply(WAVFile& wavFile) override;
    virtual ~MuteConverter() = default;
private:
    int startSecond;
    int endSecond;
    int startSample;
    int endSample;
};