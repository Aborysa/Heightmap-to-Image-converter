#include "heightMap.h"
#include "IOManager.h"
#include <iostream>
#include <fstream>
#include <math.h>

heightMap::heightMap(const std::string &filepath)
{
    load(filepath,false);
}
heightMap::heightMap(const std::string &filepath, int width, int length,int depth) : heightMap(width, length,depth)
{
    //Required for old hgt format
    load(filepath,true);
}

heightMap::heightMap(int width, int length, int depth)
{
    setDepth(depth);
    setDimensions(width,length);
    this->zones = new zone[this->width*this->length];
    for(int i = 0; i < this->width*this->length; i++){
        this->zones[i].heightData = new unsigned short[zone_width*zone_width];
    }

}

void heightMap::setDepth(int depth)
{
    this->depth = depth;
    this->zone_width = pow(2,depth);

}
void heightMap::setDimensions(int width, int length)
{
    this->width = width;
    this->length = length;
}
bool heightMap::isValid(){
    return valid;
}
bool heightMap::load(const std::string &filepath,bool old)
{
    std::cout << "Getting file buffer: \n";

    //const unsigned char * content = IOManager::getFileBuffer(filepath);
    long filesize = IOManager::getSizeOfFile(filepath);
    unsigned char * content = new unsigned char[filesize];
    IOManager::readFileToBuffer(filepath,content,filesize);
    int offset = 0;
    if(!old && filesize > 0xC){
        std::cout << "Reading header:\n";
        offset = 0xC;
        this->f_v = ((content[1]<<8) & 0xFF00) | (content[0]);
        setDepth(((content[3]<<8) & 0xFF00) | (content[2]));
        this->width = ((content[5]<<8) & 0xFF00) | (content[4]);
        this->length = ((content[7]<<8) & 0xFF00) | (content[6]);
        this->f_r = ((content[9]<<8) & 0xFF00) | (content[8]);
        this->valid = filesize == getWidth()*getLength()*2 + 0xC;
        if(isValid()){
            this->zones = new zone[this->width*this->length];
            for(int i = 0; i < this->width*this->length; i++){
                this->zones[i].heightData = new unsigned short[zone_width*zone_width];
            }
            std::cout << "\tWidth: " << width << "\n\tLength: " << length << "\n\tZone width: " << zone_width << "\n";
        }else{
            std::cout << "Invalid hg2!\n\tSize was: " << filesize << "\n\tExpected: " << (getWidth()*getLength()*2 + 0xC) << "\n";
        }
    }else{
        this->valid = (filesize > 0) && (filesize == getWidth()*getLength()*2);
    }
    if(isValid()){
        int zI = 0;
        int lI = 0;
        //std::cout << "\tLength: " << length << "\n\tWidth: " << width << "\n";
        for(int i=0; i < length*width*zone_width*zone_width*2;i+=2){
            zI = floor(i/(zone_width*zone_width*2));
            lI = (i/2)%(zone_width*zone_width);
            zones[zI].heightData[lI] = ((content[i+1+offset]<<8) & 0xFF00) | (content[i+offset]);
        }
    }
    return isValid();
}
int heightMap::getDepth(){
    return depth;
}
int heightMap::getZoneWidth(){
    return zone_width;
}
void heightMap::setRev(int rev){
    this->f_r = rev;
}
int heightMap::getRev(){
    return this->f_r;
}
bool heightMap::save(const std::string &filepath,bool old)
{
    std::ofstream ofile(filepath,std::ios::binary);
    if(!old){
        this->f_r++;
        /*Write header*/
        ofile << (char)(((int)this->f_v) & 0xFF)
              << (char)(((int)this->f_v >> 8) & 0xFF);
        ofile << (char)(((int)this->depth) & 0xFF)
              << (char)(((int)this->depth >> 8) & 0xFF);
        ofile << (char)(((int)this->width) & 0xFF)
              << (char)(((int)this->width >> 8) & 0xFF);
        ofile << (char)(((int)this->length) & 0xFF)
              << (char)(((int)this->length >> 8) & 0xFF);
        ofile << (char)(((int)this->f_r) & 0xFF)
              << (char)(((int)this->f_r >> 8) & 0xFF);
        ofile << (char)(0)
              << (char)(0);
    }

    int zI = 0;
    int lI = 0;
    for(int i=0; i < width*length*zone_width*zone_width;i++){
        zI = floor(i/(zone_width*zone_width));
        lI = i%(zone_width*zone_width);
        char hi = ((int)zones[zI].heightData[lI] >> 8) & 0xFF;
        char lo = ((int)zones[zI].heightData[lI]) & 0xFF;
        ofile << lo << hi;
    }
    ofile.close();
    return true;
}
void heightMap::setHeight(int x, int y,unsigned short height)
{
    if(inBounds(x,y)){
        zone * mZone = getZoneAt(x,y);
        int lx = x%zone_width;
        int ly = y%zone_width;
        mZone->heightData[lx + (ly*zone_width)] = height;
    }
}
int heightMap::getHeight(int x, int y)
{
    if(inBounds(x,y)){
        zone * mZone = getZoneAt(x,y);
        int lx = x%zone_width;
        int ly = y%zone_width;
        return mZone->heightData[lx + (ly*zone_width)];
    }
    return 0;
}
void heightMap::writeBuffer(int x, int y, int width,int bufferSize, const unsigned short * heightBuffer){
    for(int i=0; i < bufferSize;i++){
        setHeight(x + i%width,y + (int)(floor(i/width)*width),heightBuffer[i]);
    }
}

bool heightMap::inBounds(int x, int y){
    //x /= zone_width;
    //y /= zone_width;
    return x >= 0 && x < getWidth() && y >= 0 && y < getLength();
}
zone * heightMap::getZoneAt(int x, int y)
{
    if(inBounds(x,y)){
        return &zones[(int)(floor(x/zone_width) + floor(y/zone_width)*width)];
    }
    return nullptr;
}
int heightMap::getLength()
{
    return length*zone_width;
}
int heightMap::getWidth()
{
    return width*zone_width;
}
heightMap::~heightMap()
{
    if(zones != nullptr){
        delete[] zones;
    }
}
