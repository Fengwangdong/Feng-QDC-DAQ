#include "qdcmodule.h"
#include "qdebug.h"

QDCModule::QDCModule(int handleChef_):
    handleChef(handleChef_)
{
    //baseAddress = 0x22000000; // for V792
    baseAddress = 0x33000000; // for V792N
    headerType = 0x2;
    dataType = 0x0;
    eobType = 0x4;
    version = ReadAndOut16(handleChef, baseAddress + 0x8032) & 0xff;
    qDebug() << "version: " << version;
    qDebug() << "Board ID: "<< (ReadAndOut16(handleChef, baseAddress + 0x8036) & 0xff);
    qDebug() << "Board ID: "<< (ReadAndOut16(handleChef, baseAddress + 0x803A) & 0xff);
    qDebug() << "Board ID: "<< (ReadAndOut16(handleChef, baseAddress + 0x803E) & 0xff);

    if(version == 19){
        rightShiftBitForChannelIndex = 16;
        nBitForChannelIndex = 5;
        stepForRegisterChannelIndex = 2;
        nChannels = 32;
    }

    else if(version == 227){
        rightShiftBitForChannelIndex = 17;
        nBitForChannelIndex = 4;
        stepForRegisterChannelIndex = 4;
        nChannels = 16;
    }

}

int QDCModule::getNChannels(){
    return nChannels;
}

void QDCModule::ConfigureChannel(int channelN, bool enable, int threshold)
{
    qDebug() << "In config: " << channelN;
    int registerAddress = baseAddress + 0x1080 + channelN * stepForRegisterChannelIndex;
    uint16_t kThreshold = (uint16_t(!enable) << 8) + (threshold >> 4); // note that the bracket is
    // necessary since the priority of "+" is higher than ">>"! otherwise the readout will not work!
    WriteAndOut16(handleChef, registerAddress, kThreshold);
}

bool QDCModule::ReadChannelStatus(int channelN)
{
    qDebug() << "In read config: " << channelN;
    int registerAddress = baseAddress + 0x1080 + channelN * stepForRegisterChannelIndex;
    uint16_t kThreshold = ReadAndOut16(handleChef, registerAddress);
    bool status = bool(kThreshold>>8 & 0x1);
    return !status;
}

int QDCModule::ReadChannelThreshold(int channelN)
{
    int registerAddress = baseAddress + 0x1080 + channelN * stepForRegisterChannelIndex;
    uint16_t kThreshold = ReadAndOut16(handleChef, registerAddress);
    int threshold = (kThreshold & 0xff) << 4;
    return threshold;
}

uint16_t QDCModule::WriteAndOut16(int32_t handle, uint32_t address, uint16_t dataValue)
{
    CVErrorCodes ret;
    ret = CAENVME_WriteCycle(handle, address, &dataValue, cvA32_U_DATA, cvD16);

    qDebug() << "hello!";
    if (ret != cvSuccess) {
        qDebug() << "I cannot implement the writing with the module!";
        qDebug() << CAENVME_DecodeError(ret);
    }
    return 0;

}

uint16_t QDCModule::ReadAndOut16(int32_t handle, uint32_t address)
{
    uint16_t value16;
    CVErrorCodes ret;
    ret = CAENVME_ReadCycle(handle, address, &value16, cvA32_U_DATA, cvD16);

    if (ret != cvSuccess) {
        qDebug() << "Something fishy happened when reading (D16) address "<< hex << address;
        qDebug() << CAENVME_DecodeError(ret);
    }
    return value16;
}

uint32_t QDCModule::ReadAndOut32(int32_t handle, uint32_t address)
{
    uint32_t value32;
    CVErrorCodes ret;
    ret = CAENVME_ReadCycle(handle, address, &value32, cvA32_U_DATA, cvD32);
    if (ret != cvSuccess) {
        qDebug() << "Something fishy happened when reading (D32) address ";
        qDebug() << CAENVME_DecodeError(ret);
    }
    return value32;
}

void QDCModule::ClearChannels()
{
    ClearData(handleChef);
}

void QDCModule::ClearData(int32_t handle)
{
    int16_t value = 1 << 2;
    WriteAndOut16(handle, baseAddress + 0x1032, value);
    WriteAndOut16(handle, baseAddress + 0x1034, value);
}

// --- extract the required bits from the read value ---
int QDCModule::extractBits(uint32_t word, int rightShiftsBits, int nBits)
{
    int andNumber = pow(2,nBits) - 1;// nbits which the user wish to keep
    // --- right shift the read word by specialized number of bits ---
    // --- and fetch the bits from the shifted word ---
    return (word>>rightShiftsBits) & andNumber;
}

// --- extract the bits for judging the type of word (P45 in V792 manual) ---
int QDCModule::extractWordType(uint32_t word){
    // --- right shift the word by 24 bit and fetch the first three bits ---
    return extractBits(word, 24, 3);
}

// -- extract the size of data from the header (P45 in V792 manual) ---
int QDCModule::extractDataSize(uint32_t word){
    // --- right shift the header by 8 bits and fetch the first 6 bits ---
    return extractBits(word, 8, 6);
}

// --- extract the ADC bits from data word (P45) ---
int QDCModule::extractADC(uint32_t word){
    return extractBits(word, 0, 12);
}

// --- extract the index of channel from data word (P45) ---
int QDCModule::extractNChannel(uint32_t word){
    // --- right shift the data word by 16 bits and take the first 5 bits --- (V792)
    // --- right shift the data word by 17 bits and take the first 4 bits --- (V792N)
    return extractBits(word, rightShiftBitForChannelIndex, nBitForChannelIndex);
}

// --- read the data from the channel ---
void QDCModule::ReadChannelData(int channelN, uint32_t* data, bool* valid)
{
    int dataready = (ReadAndOut16(handleChef, baseAddress + 0x100E) & 0x1);
    // read status control register for data ready bit
    //int data = 0;
    if(dataready){
        *valid = true;
        *data = ProcessData(handleChef, baseAddress + 0x0000, channelN);
        qDebug() << "Finish processing on the event!";
    }
    else {
        *valid = false;
    }
}

// --- process the data word ---
int QDCModule::ProcessData(int32_t handle, uint32_t address, int channelN)
{
    uint32_t output = ReadAndOut32(handle, address);
    int wordType = extractWordType(output);
    int ADC[32] = {0};

    // --- fetch the header ---
    if(wordType == headerType){
        qDebug() << "Header: " << output;
        qDebug() << "Valid data!";

        // --- fetch the converted channels (data) from the header and read them in order ---
        int dataSize = extractDataSize(output);
        qDebug() << "datasize: " << dataSize;
        //if(channelN >= 0) dataSize = 1; // specify the channel for user!
        for(int i=0; i<dataSize; i++){
            output = ReadAndOut32(handle, address);
            wordType = extractWordType(output);
            if(wordType != dataType){
                qDebug() << "Something fishy with the data in this channel";
                return 0;
            }

            int index = extractNChannel(output);
            ADC[index] = extractADC(output); // numbers in the ADC with 12 bits
            qDebug() << " Converted value: " << ADC[extractNChannel(output)];

        }

        // --- retrieve the end of block ---
        output = ReadAndOut32(handle, address);
        wordType = extractWordType(output);
        if(wordType != eobType){
           qDebug() << "There is no end of block!!!";
           return 0;
        }
        else{
           qDebug() << "End of block: " << output;
        }

    }

    else{
       qDebug() << "Event without header or reserved event!";
       // --- the word is invalid data in this case ---
       return 0;
    }

    return ADC[channelN];

}

