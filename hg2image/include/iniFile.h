#ifndef INIFILE_H
#define INIFILE_H

#include <map>
#include <string>
#include <vector>
class iniFile
{
    public:
        iniFile();
        iniFile(std::string filepath);
        std::string getValue(const std::string &globalVar);
        std::string getValue(const std::string &section,const std::string &var);
        void setValue(const std::string &globalVar,const std::string &val);
        void setValue(const std::string &section,const std::string &var,const std::string &val);
        std::string getFilePath();
        void load();
        void load(std::string filepath);
        void save();
        void save(std::string filepath);
        virtual ~iniFile();
    protected:

    private:
        std::string filepath;
        std::map<std::string,std::map<std::string,std::string>> fields;
        std::vector<std::pair<std::string,std::vector<std::string>>> fieldOrder;
};

#endif // INIFILE_H
