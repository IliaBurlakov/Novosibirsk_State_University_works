#include <iostream> 
#include "SoundProcessor.h"
int main(int argc, char** argv){
    if(argc < 4){
        std::cerr<<"Bad number of arguments (must be >= 4)"<< std::endl;
        return 1;
    }
    std::vector<std::string> inputFiles;
    for (int i = 1; i < argc - 2; ++i) {
        inputFiles.push_back(argv[i]);
    }
    std::string outputFile = argv[argc - 2];
    std::string commandFile = argv[argc - 1];

    SoundProcessor processor(inputFiles, outputFile, commandFile);
    processor.process();
    return 0;
}
