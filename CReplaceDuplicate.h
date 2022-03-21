#pragma once
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CReplaceDuplicate : public GenericVideoFilter
{
    bool drop;
    int* vector = new int[vi.num_frames];

public:
    CReplaceDuplicate(PClip _child, bool _drop, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};