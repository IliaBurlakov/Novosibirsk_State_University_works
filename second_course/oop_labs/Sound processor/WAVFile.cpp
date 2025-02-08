#pragma once
#include "WAVFile.h"
#include <stdexcept>
#include <cstring>
#include <iostream>

WAVFile::WAVFile(const std::string &filePath) : filePath(filePath), dataSize(0) {}

void WAVFile::read()
{
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile)
    {
        throw std::runtime_error("Cant open file: " + filePath);
    }

    readHeader(inFile);

    samples.resize(dataSize / sizeof(short));
    inFile.read(reinterpret_cast<char *>(samples.data()), dataSize);

    if (inFile.fail())
    {
        std::cerr << "Cant read samples from file " << filePath <<" !\n";
    }   
    inFile.close();
}

bool checkChunkId(const char *id, const char *expected)
{
    return std::strncmp(id, expected, 4) == 0;
}

void WAVFile::readHeader(std::ifstream &inFile)
{
    char riff[4];
    int chunkSize;
    char wave[4];

    inFile.read(riff, 4);
    inFile.read(reinterpret_cast<char *>(&chunkSize), 4);
    inFile.read(wave, 4);
    if (!checkChunkId(riff, "RIFF") || !checkChunkId(wave, "WAVE"))
    {
        std::cerr << "Invalid WAV file format!\n";
        return;
    }

    char chunkId[4];    
    while (inFile.read(chunkId, 4))
    {
        inFile.read(reinterpret_cast<char *>(&chunkSize), 4);
        if (checkChunkId(chunkId, "data"))
        {
            dataSize = chunkSize;
            break;
        }
        else
        {
            inFile.seekg(chunkSize, std::ios::cur);
        }

        if (inFile.eof() || inFile.fail())
        {
            std::cerr << "Cant find data chunk!\n";
            break;
        }
    }
}

void WAVFile::write(const std::string &outputPath)
{
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile)
    {
        throw std::runtime_error("Cant open output file: " + outputPath);
    }


    writeHeader(outFile);

    outFile.write(reinterpret_cast<const char *>(samples.data()), samples.size() * sizeof(short));
    outFile.close();
}

std::vector<short> &WAVFile::getSamples()
{
    return samples;
}

std::string WAVFile::getFilePath() const
{
    return filePath;
}

int WAVFile::getSampleRate() const
{
    return sampleRate;
}
void WAVFile::writeHeader(std::ofstream &outFile)
{
    char buffer[44] = {0};
    std::memcpy(buffer, "RIFF", 4);
    int fileSize = 36 + samples.size() * sizeof(short);
    std::memcpy(buffer + 4, &fileSize, 4);
    std::memcpy(buffer + 8, "WAVE", 4);

    std::memcpy(buffer + 12, "fmt ", 4);
    int subchunk1Size = 16;
    std::memcpy(buffer + 16, &subchunk1Size, 4);
    short audioFormat = 1;
    std::memcpy(buffer + 20, &audioFormat, 2);
    std::memcpy(buffer + 22, &numChannels, 2);
    std::memcpy(buffer + 24, &sampleRate, 4);
    int byteRate = sampleRate * numChannels * sizeof(short);
    std::memcpy(buffer + 28, &byteRate, 4);
    short blockAlign = numChannels * sizeof(short);
    std::memcpy(buffer + 32, &blockAlign, 2);
    short bitsPerSample = 16;
    std::memcpy(buffer + 34, &bitsPerSample, 2);

    std::memcpy(buffer + 36, "data", 4);
    int dataSize = samples.size() * sizeof(short);
    std::memcpy(buffer + 40, &dataSize, 4);

    outFile.write(buffer, 44);
}
