#ifndef IOMANAGER_H
#define IOMANAGER_H
#include <vector>
#include <string>
#include <unordered_map>
class IOManager
{
    public:
        IOManager();
        static bool readFileToBuffer(const std::string &filePath, std::vector<unsigned char> &buffer,int start=0,int end=-1);
        static bool readFileToBuffer(const std::string &filePath, unsigned char *buffer,int bufferSize,int start=0, int end=-1);
        static const unsigned char * const getFileBuffer(const std::string &filePath);
        static void clearCache();
        static int getSizeOfFile(const std::string &filePath);

        virtual ~IOManager();
    protected:
    private:
        static std::unordered_map<std::string,unsigned char*> fileCache;

};

#endif // IOMANAGER_H

