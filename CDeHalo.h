#pragma once
#include <cmath> 
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CDeHalo : public GenericVideoFilter
{
    PClip bClip;
    int nt;
    int mode;
    bool Y;
    bool U;
    bool V;

public:
    CDeHalo(PClip _child, PClip _bClip, int _nt, int _mode, bool _Y, bool _U, bool _V, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};