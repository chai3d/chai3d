//PUT_FULLYFREE_COPYRIGHT_NOTICE_HERE

#pragma once

#include "luaFunctionDataItem.h"
#include "v_repLib.h"

class CLuaFunctionData  
{
public:
    CLuaFunctionData();
    virtual ~CLuaFunctionData();

    bool readDataFromLua(const SLuaCallBack* p,const int* expectedArguments,int requiredArgumentCount,const char* functionName);
    std::vector<CLuaFunctionDataItem>* getInDataPtr();

    void pushOutData(const CLuaFunctionDataItem& dataItem);
    void writeDataToLua(SLuaCallBack* p);

    static void getInputDataForFunctionRegistration(const int* dat,std::vector<int>& outDat);


protected:
    std::vector<CLuaFunctionDataItem> _inData;
    std::vector<CLuaFunctionDataItem> _outData;
};
