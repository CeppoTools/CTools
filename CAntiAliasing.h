#pragma once
#include <cmath> 
#include <fstream>
#include <iostream>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CAntiAliasing : public GenericVideoFilter
{
    int nt;
    int mode;
    bool Y;
    bool U;
    bool V;

public:
    CAntiAliasing(PClip _child, int _nt, int _mode, bool _Y, bool _U, bool _V, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};
