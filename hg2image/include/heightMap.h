#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#define DEFAULT_DEPTH 8
#define FORMAT_V 1
#define FILE_REV 10

#include <string>

struct zone{
    unsigned short * heightData = nullptr;
    virtual ~zone(){
        if(heightData != nullptr){
            delete[] heightData;
        }
    };
};

class heightMap
{
    public:

        heightMap(unsigned short * data);
        heightMap(int width, int length,int depth = DEFAULT_DEPTH);
        heightMap(unsigned short * data,int width, int length,int depth = DEFAULT_DEPTH);
        heightMap(const std::string & filepath);
        heightMap(const std::string & filepath,int width, int length,int depth = DEFAULT_DEPTH);
        bool save(const std::string & filepath,bool old = false);
        bool load(const std::string & filepath,bool old = false);
        void setDepth(int depth);
        void setRev(int rev);
        int getRev();
        int getDepth();
        int getZoneWidth();
        void setDimensions(int width, int length);
        void setHeight(int x, int y, unsigned short height);
        bool inBounds(int x, int y);
        void writeBuffer(int x, int y, int width,int bufferSize, const unsigned short * heightBuffer);
        //void writeHeightMap(int x, int y,const & heightMap);
        //heightMap * getCroped(int x, int y, int width, int length);
        zone * getZoneAt(int x, int y);
        int getHeight(int x, int y);
        bool isValid();
        int getWidth();
        int getLength();
        virtual ~heightMap();
    protected:
    private:
        zone * zones = nullptr;
        unsigned char depth = DEFAULT_DEPTH;
        unsigned short zone_width;
        unsigned char f_v = 1;
        unsigned char f_r = 10;
        int width;
        int length;
        bool valid = true;

};

#endif // HEIGHTMAP_H
