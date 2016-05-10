#include "iniFile.h"
#include <vector>
#include "IOManager.h"
#include <ios>
#include <boost/regex.hpp>
#include <exception>
#include <fstream>
#include <iostream>

iniFile::iniFile(){}
iniFile::iniFile(std::string filepath)
{
    this->filepath = filepath;
    load();
}
void iniFile::load(){
    load(filepath);
}
void iniFile::load(std::string filepath){
    std::string content((const char *)IOManager::getFileBuffer(filepath),IOManager::getSizeOfFile(filepath));

    boost::regex regx("\\s*(\\w+)\\s*=\\s*(\\\"*[!#-Z\\\\^-~]+\\\"*)");
    boost::sregex_iterator end;

    boost::regex regx2("\\[(\\w+)\\]((?:\\s*\\w+\\s*=[\\s\\\"]*[!#-Z\\\\^-~]+\\\"*)*)");
    boost::sregex_iterator iter2(content.begin(),content.end(),regx2);
    for(;iter2!=end;++iter2){
        std::string section = (*iter2)[1];
        std::cout << "Match: " << section << "\n";
        std::string sub = (*iter2)[2];
        boost::sregex_iterator iter(sub.begin(),sub.end(),regx);
        for(;iter!=end;++iter){
            for(int x=1;x<(*iter).size();x+=2){
                std::cout << "    " << (*iter)[x] << " = " << (*iter)[x+1]  << "\n";
                setValue(section,(*iter)[x],(*iter)[x+1]);
            }
        }
    }
}
void iniFile::save(){
    save(filepath);
}
void iniFile::save(std::string filepath){
    std::string output = "";
    for(auto i:fieldOrder){
        output += "[" + i.first + "]\n";
        for(std::string k:i.second){
            output += k + " = " + getValue(i.first,k) + "\n";
        }
        output += "\n";
    }
    std::ofstream ofile(filepath);
    ofile << output;
    ofile.close();
}
void iniFile::setValue( const std::string &section,const std::string &var,const std::string &val)
{
    std::vector<std::string> * lVec;
    if(fields.count(section) != 1){
        fields.insert(std::make_pair(section,std::map<std::string,std::string>()));
        fieldOrder.push_back(std::make_pair(section,std::vector<std::string>()));
        lVec = &(fieldOrder.at(fieldOrder.size()-1).second);
     }else{
        lVec = &(fieldOrder.at(0).second);
        for(int i=0; i<fieldOrder.size();i++){
            auto &k = fieldOrder.at(i);
            if(k.first == section){
                lVec = &(k.second);
                break;
            }
        }
    }
    if(fields.at(section).count(var) == 1){
        fields.at(section)[var] = val;
    }else{
        fields.at(section).insert(std::make_pair(var,val));
        lVec->push_back(var);
    }

}
std::string iniFile::getValue(const std::string &globalVar)
{
    return getValue("",globalVar);
}
std::string iniFile::getValue(const std::string &section,const std::string &var)
{
    try{
        return fields.at(section).at(var);
    }catch(std::out_of_range& e){
        std::cout << "Error: " << var << " in section " << section << " does not exist!\n";
    }
    return "";
}
iniFile::~iniFile()
{
    //dtor
}
