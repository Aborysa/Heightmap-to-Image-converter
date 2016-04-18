#include "IOManager.h"
#include <fstream>
#include <math.h>
#include <iostream>

std::unordered_map<std::string,unsigned char *> IOManager::fileCache;

bool IOManager::readFileToBuffer(const std::string &filePath, std::vector<unsigned char> &buffer,int start,int end){
    int fileSize = IOManager::getSizeOfFile(filePath);
    buffer.resize(fileSize);
    return IOManager::readFileToBuffer(filePath,&(buffer[0]),buffer.size(),start,end);

}
bool IOManager::readFileToBuffer(const std::string &filePath, unsigned char* buffer,int bufferSize,int start, int end){
    std::ifstream file(filePath,std::ios::binary | std::ios::ate);
    if(file.fail()){
        std::cout << "Error: could not open file: " << filePath << "\n";
        perror(filePath.c_str());
        return false;
    }
    if(start < 0){
        std::cout << "Start pointer can not be less than 0, was: " << start << "\n";
        return false;
    }
    int fileSize = file.tellg();
    file.seekg(0,(std::ios_base::seekdir)start);
    int bToRead = end < 0 ? fileSize : std::min(end,fileSize);
    bToRead = std::min(bToRead,bufferSize);
    file.read((char*)buffer,bToRead);
    file.close();
    return true;
}
const unsigned char * const IOManager::getFileBuffer(const std::string &filePath){
    if(IOManager::fileCache.count(filePath) == 0){
        int size = getSizeOfFile(filePath);
        unsigned char * cacheBuffer = new unsigned char[size];
        IOManager::fileCache.insert(std::make_pair<std::string,unsigned char *>((std::string)filePath,(unsigned char *)cacheBuffer));
        if(!readFileToBuffer(filePath,cacheBuffer,size)){
            std::cout << "Could not read file, false returned by IOManager::readFileToBuffer\n";
            IOManager::fileCache.erase(filePath);
            delete cacheBuffer;
            return cacheBuffer;
        }
    }
    return IOManager::fileCache.at(filePath);
}
int IOManager::getSizeOfFile(const std::string &filePath){
    std::ifstream file(filePath,std::ios::binary | std::ios::ate);
    if(file.fail()){
        return 0;
    }
    return file.tellg();
}
