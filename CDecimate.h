#pragma once
#include <cmath> 
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CDecimate : public GenericVideoFilter
{
    bool isBob;
    bool error;
    bool heuristic;
    PClip iClip;
    int* vec = new int[vi.num_frames];
    bool* isInt = new bool[vi.num_frames];

public:
    CDecimate(PClip _child, bool _isBob, bool _error, bool _heuristic, PClip _iClip, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

int CDecimate_StartUp(int nf, bool error, bool heuristic, bool iClip);