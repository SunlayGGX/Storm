#include "CoInitializerRAII.h"

#include <objbase.h>


Storm::CoInitializerRAII::CoInitializerRAII() :
    _result{ CoInitialize(NULL) },
    _shouldCoUninitialize{ true }
{

}

Storm::CoInitializerRAII::CoInitializerRAII(DWORD flag) :
    _result{ CoInitializeEx(NULL, flag) },
    _shouldCoUninitialize{ true }
{

}

Storm::CoInitializerRAII::~CoInitializerRAII()
{
    if (_shouldCoUninitialize && _result != RPC_E_CHANGED_MODE)
    {
        CoUninitialize();
    }
}

HRESULT Storm::CoInitializerRAII::getCoInitializeCallResult() const
{
    return _result;
}
