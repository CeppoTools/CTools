#pragma once
#include <cmath> 
#include <string>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CInterpolateDuplicate : public GenericVideoFilter
{
    float thr;
    int nt;
    PClip dClip;
    PClip iClip;

public:
    CInterpolateDuplicate(PClip _child, float _thr, int _nt, PClip _dClip, PClip _iClip, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

bool CInterpolateDuplicate_isDup(PVideoFrame& src, PVideoFrame& prv, PVideoFrame& nxt, int nt, float thr, bool isY8);