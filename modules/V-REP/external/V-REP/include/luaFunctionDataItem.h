//PUT_FULLYFREE_COPYRIGHT_NOTICE_HERE

#pragma once

#include <vector>
#include <string>

class CLuaFunctionDataItem
{
public:
    CLuaFunctionDataItem();
    CLuaFunctionDataItem(bool v);
    CLuaFunctionDataItem(int v);
    CLuaFunctionDataItem(float v);
    CLuaFunctionDataItem(double v);
    CLuaFunctionDataItem(const std::string& v);
    CLuaFunctionDataItem(const char* bufferPtr,unsigned int bufferLength);

    CLuaFunctionDataItem(const std::vector<bool>& v);
    CLuaFunctionDataItem(const std::vector<int>& v);
    CLuaFunctionDataItem(const std::vector<float>& v);
    CLuaFunctionDataItem(const std::vector<double>& v);
    CLuaFunctionDataItem(const std::vector<std::string>& v);

    virtual ~CLuaFunctionDataItem();

    bool isTable();
    int getType();
    void setNilTable(int size);
    int getNilTableSize();

    std::vector<bool> boolData;
    std::vector<int> intData;
    std::vector<float> floatData;
    std::vector<double> doubleData;
    std::vector<std::string> stringData;

protected:
    int _nilTableSize;
    bool _isTable;
    int _type; // -1=nil,0=bool,1=int,2=float,3=string,4=buffer,5=double
};
