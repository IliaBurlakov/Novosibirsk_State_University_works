#include "SoundProcessor.h"
#include "WAVFile.h"
#include "MixConverter.h"
#include "MuteConverter.h"
#include "ReverseConverter.h"
#include <iostream>
#include <fstream>
#include <sstream>

SoundProcessor::SoundProcessor(const std::vector<std::string> &inputFiles, const std::string &outputFile, const std::string &commandFile)
    : inputFiles(inputFiles), outputFile(outputFile), commandFile(commandFile) {}

void SoundProcessor::process()
{
    WAVFile mainWavFile(inputFiles[0]);
    mainWavFile.read();

    parseCommands();

    executeCommands(mainWavFile);
    mainWavFile.write(outputFile);
}

void SoundProcessor::parseCommands()
{
    std::ifstream file(commandFile);
    if (!file.is_open())
    {
        std::cerr << "Failed to open command file: " << commandFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "mute")
        {
            int startSecond, endSecond;
            if (iss >> startSecond >> endSecond)
            {
                if (startSecond >= 0 && endSecond > startSecond)
                {
                    converters.push_back(std::make_unique<MuteConverter>(startSecond, endSecond));
                }
                else
                {
                    std::cerr << "Error: invalid arguments for mute.\n";
                }
            }
            else
            {
                std::cerr << "Error: mute requires two integer arguments.\n";
            }
        }
        else if (command == "mix")
        {
            int fileNumberToMix, startSecond;
            if (iss >> fileNumberToMix >> startSecond)
            {
                if (fileNumberToMix > 0 && startSecond >= 0)
                {
                    if (fileNumberToMix <= inputFiles.size())
                    {
                        std::string otherFilePath = inputFiles[fileNumberToMix - 1];
                        converters.push_back(std::make_unique<MixConverter>(otherFilePath, startSecond));
                    }
                    else
                    {
                        std::cerr << "Error: not enough files to apply mix converter for file number " << fileNumberToMix << ".\n";
                    }
                }
                else
                {
                    std::cerr << "Error: invalid arguments for mix.\n";
                }
            }
            else
            {
                std::cerr << "Error: mix requires two integer arguments.\n";
            }
        }
        else if (command == "reverse")
        {
            int startSecond, endSecond;
            if (iss >> startSecond >> endSecond)
            {
                if (startSecond >= 0 && endSecond > startSecond)
                {
                    converters.push_back(std::make_unique<ReverseConverter>(startSecond, endSecond));
                }
                else
                {
                    std::cerr << "Error: invalid arguments for reverse.\n";
                }
            }
            else
            {
                std::cerr << "Error: reverse requires two integer arguments.\n";
            }
        }
        else
        {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }

    file.close();
}

void SoundProcessor::executeCommands(WAVFile &wavFile)
{
    for (auto &converter : converters)
    {
        converter->apply(wavFile);
    }
}