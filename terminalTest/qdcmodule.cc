#include <CAENVMElib.h>
#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
#define HEADERTYPE 0x2
#define DATATYPE 0x0
#define EOBTYPE 0x4
#define BASEADDRESS 0x22000000

using namespace std;

// --- read the data in the specific address of the module (D16 read mode) ---
uint16_t ReadAndOut16(int32_t handle, uint32_t address){
	uint16_t value16;
	CVErrorCodes ret;
	ret = CAENVME_ReadCycle(handle, address, &value16, cvA32_U_DATA, cvD16);

	if (ret != cvSuccess) {
		cout << "Something fishy happened when reading (D16) address "<< hex << address  << endl;
		cout << CAENVME_DecodeError(ret) << endl;
	}
	return value16;
}

// --- write the data value set by user to the unit with specific address of the module ---
uint16_t WriteAndOut16(int32_t handle, uint32_t address, uint16_t dataValue){
	CVErrorCodes ret;
	ret = CAENVME_WriteCycle(handle, address, &dataValue, cvA32_U_DATA, cvD16);

	if (ret != cvSuccess) {
		cout << "I cannot implement the writing with the module!" << endl;
		cout << CAENVME_DecodeError(ret) << endl;
	}
	return 0;
}


// --- read the data in the specific address of the module (D32 read mode) ---
uint32_t ReadAndOut32(int32_t handle, uint32_t address){
	uint32_t value32;
	CVErrorCodes ret;
	ret = CAENVME_ReadCycle(handle, address, &value32, cvA32_U_DATA, cvD32);
	if (ret != cvSuccess) {
		cout << "Something fishy happened when reading (D32) address " << endl;
		cout << CAENVME_DecodeError(ret) << endl;
	}
	return value32;
}

// --- extract the required bits from the read value ---
int extractBits(uint32_t word, int rightShiftsBits, int nBits){
    int andNumber = pow(2,nBits) - 1; // nbits which the user wish to keep
	// --- right shift the read word by specialized number of bits ---
	// --- and fetch the bits from the shifted word ---
	return (word>>rightShiftsBits) & andNumber;
}

// --- extract the bits for judging the type of word (P45 in V792 manual) ---
int extractWordType(uint32_t word){
	// --- right shift the word by 24 bit and fetch the first three bits ---
	return extractBits(word, 24, 3);
}

// -- extract the size of data from the header (P45 in V792 manual) ---
int extractDataSize(uint32_t word){
	// --- right shift the header by 8 bits and fetch the first 6 bits ---
	return extractBits(word, 8, 6);
}

// --- extract the geographical address from the header (P45 in V792 manual) ---
int extractGEO(uint32_t word){
	// --- right shift the header by 27 bits and fetch the first 5 bits ---
	return extractBits(word, 27, 5);
}

// --- extract the number of channels from data word (P45) ---
int extractNChannel(uint32_t word){
	// --- right shift the data word by 16 bits and take the first 5 bits ---
	return extractBits(word, 16, 5);
}

// --- extract the under-threshold bit from the data word (P45) ---
int extractUnderThreshold(uint32_t word){
	return extractBits(word, 13, 1);
}

// --- extract the overflow bit from the data word (P45) ---
int extractOverThreshold(uint32_t word){
	return extractBits(word, 12, 1);
}

// --- extract the ADC bits from data word (P45) ---
int extractADC(uint32_t word){
	return extractBits(word, 0, 12);
}

// --- extract the PEDESTAL value from Iped register (P63) ---
int extractIped(uint16_t word){
    return extractBits(word, 0, 8);
}

// --- clear the data after device start up ---
void ClearData(int32_t handle){
    int baseAddress = BASEADDRESS;
    int16_t value = 1 << 2;
    WriteAndOut16(handle, baseAddress + 0x1032, value); 
    WriteAndOut16(handle, baseAddress + 0x1034, value); 
}

void DisableOverflow(int32_t handle){
    int baseAddress = BASEADDRESS;
    int16_t value = 1 << 3;
    WriteAndOut16(handle, baseAddress + 0x1032, value);
}

void EnableOverflow(int32_t handle){
    int baseAddress = BASEADDRESS;
    int16_t value = 1 << 3;
    WriteAndOut16(handle, baseAddress + 0x1034, value);
}

// --- process the data word ---
int ProcessData(int32_t handle, uint32_t address, ofstream *text){
	const int goodEvent = 1;
	uint32_t output = ReadAndOut32(handle, address);
    int wordType = extractWordType(output);
	// --- fetch the header ---
	if(wordType == HEADERTYPE){
		cout << "Header: " << std::hex << output << endl
			<< "Valid data: " << endl;

		// --- fetch the converted channels (data) from the header and read them in order ---
		int dataSize = extractDataSize(output);
		for(int i=0; i<dataSize; i++){
			output = ReadAndOut32(handle, address);
            wordType = extractWordType(output);
			if(wordType != DATATYPE){
				cout << "Something fishy with the data in channel" << i << endl;
			}
			int GEO = extractGEO(output); // --- geographical address ---
			int Nchannel = extractNChannel(output); // --- number of channels ---
			int UnderThreshold = extractUnderThreshold(output); // under-threshold bit
			int OverThreshold = extractOverThreshold(output); // overflow bit
			int ADC = extractADC(output); // numbers in the ADC with 12 bits

			cout << std::dec << "Geograph: " << GEO
				     << " Channel No. " << Nchannel
					 << " Underflow: " << UnderThreshold
					 << " Overflow: " << OverThreshold
					 << " Converted value: " << ADC << endl;

            // --- record the ADC values into the text files for offline histograms ---
            (*text) << ADC << endl;
		}


		// --- retrieve the end of block ---
		output = ReadAndOut32(handle, address);
        wordType = extractWordType(output);
		if(wordType != EOBTYPE){
			cout << "There is no end of block!!!" << endl;
			return !goodEvent;
		}
		else{
			cout << "End of block: " << std::hex << output << endl;
			return goodEvent;
		}
	}

	else{
		cout << "Event without header or reserved event!" << endl;
		return !goodEvent; 
		// --- the word is invalid data in this case ---
	}

}

// --- read the pedestal register of QDC module ---
void testPedestal(int32_t handle, int value) {
    uint32_t baseAddress = BASEADDRESS;
    // read the iped value of the register ---
    cout << "initial Pedestal value: " << std::dec << (ReadAndOut16(handle, baseAddress + 0x1060) & 0xff) << endl;
    WriteAndOut16(handle, baseAddress + 0x1060, value);
    cout << "updated Pedestal value: " << std::dec << (ReadAndOut16(handle, baseAddress + 0x1060) & 0xff) << endl;
}

// --- access the module and read the information inside it ---
void testQDCModule(int32_t handle, ofstream *text) {
	uint32_t baseAddress = BASEADDRESS;
	// Read Firmware Revision register at 0x1000
	// Print out the Firmware Revision
	cout << "Firmware Revision: " << std::hex << ReadAndOut16(handle, baseAddress + 0x1000) << endl;
	cout << "Data ready: " << std::dec << (ReadAndOut16(handle, baseAddress + 0x100E) & 0x1) << endl;

	int iter = 0; // an iterator which will increment until reach niter 
	int niter = 5000; // it decides how many good events needs to be aquired before abort
	int dataready = 0; // variable used to see if at least a datum have been aquired
	do {
		// read status control register for data ready bit
		dataready = (ReadAndOut16(handle, baseAddress + 0x100E) & 0x1);
		if(dataready){
			int goodEvent = ProcessData(handle, baseAddress + 0x0000, text);
			if(goodEvent){ 
				iter++;
				cout << "Finish processing on the " << std::dec << iter << " event!" << endl;
			}
			else{
				cout << "The event configuration has bugs!" << endl;
			}
		}
	/*	else{// no data in the output buffer
			cout << "No data!" << endl;
		    this_thread::sleep_for(chrono::seconds(1)); // read the buffer with sleep mode if no data
		}*/

		// --- read the data with frequency of one second ---
	} while(iter < niter);

}

// --- read the default/updated threshold with Bit Set 2 register (P65 of the manual) ---
void testThreshold(int32_t handle, uint32_t flagedChannels){
    // --- flagedChannels represent the bit combination of the 32 input channels
	// --- where the user can extract the channel which one would like to access --- 
	uint32_t baseAddress = BASEADDRESS;
	for (int i=0; i<32; i++){ // there are totally 32 channels of input
		int mustRead = extractBits(flagedChannels, i, 1); // extract each bit from the must read segments
		if(mustRead == 1){
			uint32_t registerAddress = 0x1080 + i*2; // access each register (P65)
			uint16_t thresholdRegister = ReadAndOut16(handle, baseAddress + registerAddress);
			int thresholdValue = extractBits(thresholdRegister, 0, 8); // the first 8 bits are the threshold arrow
			cout << "Threshold value of channel " << i << " : " << std::dec << thresholdValue << endl;	
		}
	}
}

// --- change the default threshold with Bit Set 2 register (P20 of the manual) ---
void changeThreshold(int32_t handle, uint32_t flagedChannels, int effectiveThreshold){
  // Note: the effective threshold is given by user,  while not equivalent to the threshold in the register!(see P20 of the manual)	
	uint32_t baseAddress = BASEADDRESS;
	for (int i=0; i<32; i++){ // there are totally 32 input channels
		int disableNumber = extractBits(flagedChannels, i, 1);
		if(disableNumber == 1){ // if this number is 0, it means this channel will not been taken into account
			uint32_t registerAddress = 0x1080 + i*2;
			uint16_t thresholdRegister = ReadAndOut16(handle, baseAddress + registerAddress);
			int thresholdValue = extractBits(thresholdRegister, 0, 8);
			if((effectiveThreshold>>4) != thresholdValue){ // right shift the effective threshold to get the threshold value for the register; firstly need to check if it is the same as the orginal one
				thresholdRegister |= 0xff;
				thresholdRegister &= ((0x1<<8) | (effectiveThreshold>>4)); // this two rows are aimed to disable the channel without changing the orginal threshold, while not disabling it again if this channel has already been disabled 
				WriteAndOut16(handle, baseAddress + registerAddress, thresholdRegister);
			}
		}
	}
}


// --- disable the one/several channels with Bit Set 2 register (P65 of the manual) ---
// --- quite similar to the last function ---
void disableChannels(int32_t handle, uint32_t flagedChannels){
	
	uint32_t baseAddress = BASEADDRESS;
	for (int i=0; i<32; i++){
		int disableNumber = extractBits(flagedChannels, i, 1);
		if(disableNumber == 1){
			uint32_t registerAddress = 0x1080 + i*2;
			uint16_t thresholdValue = ReadAndOut16(handle, baseAddress + registerAddress);
			int disable = extractBits(thresholdValue, 8, 1);
			if(disable == 0){
				thresholdValue |= (1<<8);// this two rows are aimed to disable the channel without changing the orginal threshold, while not disabling it again if this channel has already been disabled
				WriteAndOut16(handle, baseAddress + registerAddress, thresholdValue);
			}
		}
	}
}

int main(int argc, char *argv[]){

	// handleChef is the connection with the master
	int32_t handleChef;

	// Initialise the connection
	if (CAENVME_Init(cvV1718, 0, 0, &handleChef) == cvSuccess) { 
		cout << "Connection with master established" << endl;
	}
	else {
		cout << "Ooops, unable to connect!" << endl;
		return 1;
	}

    // --- clear the initial random data after the module starting up ---
    ClearData(handleChef);
	cout << "The threshold of all the channels" << endl;
	testThreshold(handleChef, 0xffffffff);
    changeThreshold(handleChef, 0xffffffff, 1);
	testThreshold(handleChef, 0xffffffff);

	cout << "Disable the channels ..." << endl;
	disableChannels(handleChef, 0xfffffffe);

    string currentArg = argv[1];
    if(argc > 1){
        if(currentArg == "iped"){
            for(int i=0;i<=255;i=i+5){
                cout << "Start with the pedestal: " << i << endl; 
                testPedestal(handleChef, i);

                cout << "Test QDC module ..." << endl;
                string fileName = "Iped_" + to_string(i) + ".txt";
                cout << "output: " << fileName << endl;
                ofstream text(fileName, ios::out);
                ClearData(handleChef);
                testQDCModule(handleChef, &text);
                text.close();
            }
        }
        else if(currentArg == "overflow"){
            testPedestal(handleChef, 180);
            cout << "Test QDC module ..." << endl;
            string fileName = "Iped_" + to_string(180) + ".txt";
            ofstream text(fileName, ios::out);
            DisableOverflow(handleChef);
            ClearData(handleChef);
            testQDCModule(handleChef, &text);
            text.close();
            EnableOverflow(handleChef);
        }
        else{
            testPedestal(handleChef, 180);
            cout << "Test QDC module ..." << endl;
            string fileName = "Iped_" + to_string(180) + ".txt";
            ofstream text(fileName, ios::out);
            ClearData(handleChef);
            testQDCModule(handleChef, &text);
            text.close();
        }
    }
    // Closing the connection
    CAENVME_End(handleChef);
    cout << "Connection closed" << endl;
    return 0;
}
