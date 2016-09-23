#ifndef QDCMODULE_H
#define QDCMODULE_H
#include <CAENVMElib.h>

class QDCModule
{
public:

    QDCModule(int handleChef_);
    void ConfigureChannel(int channelN, bool enable, int threshold);
    bool ReadChannelStatus(int channelN);
    int ReadChannelThreshold(int channelN);
    void ClearChannels();
    void ReadChannelData(int channelN, uint32_t* data, bool* valid);
    int getNChannels();

private:
    int32_t handleChef;
    int baseAddress;
    int headerType;
    int dataType;
    int eobType;
    int version;
    int rightShiftBitForChannelIndex;
    int nBitForChannelIndex;
    int stepForRegisterChannelIndex;
    int nChannels;
    uint32_t adcWord;
    uint16_t ReadAndOut16(int32_t handle, uint32_t address);
    uint32_t ReadAndOut32(int32_t handle, uint32_t address);
    uint16_t WriteAndOut16(int32_t handle, uint32_t address, uint16_t dataValue);
    void ClearData(int32_t handle);
    int extractBits(uint32_t word, int rightShiftsBits, int nBits);
    int extractWordType(uint32_t word);
    int extractDataSize(uint32_t word);
    int extractADC(uint32_t word);
    int extractNChannel(uint32_t word);
    int ProcessData(int32_t handle, uint32_t address, int channelN);
};

#endif // QDCMODULE_H
