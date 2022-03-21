#pragma once
#include <cmath> 
#include <windows.h>
#include "avisynth.h"

class CTemporalSoften : public GenericVideoFilter
{
    int radius;
    bool isb;
    bool Y;
    bool U;
    bool V;

public:
    CTemporalSoften(PClip _child, int _radius, bool _isb, bool _Y, bool _U, bool _V, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};