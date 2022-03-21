#pragma once
#include <cmath>
#include <array>
#include <fstream>
#include <iostream>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CDuplicate : public GenericVideoFilter
{
    float thr;
    float thr2;
    int nt;
    bool fields;
    PClip dClip;
    bool write;
    ofstream file;
    ofstream file2;
    bool* isOut = new bool[vi.num_frames];

public:
    CDuplicate(PClip _child, float _thr, float _thr2, int _nt, bool _fields, PClip _dClip, bool _write, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

array<bool, 2> CDuplicate_isDup(PVideoFrame& src, PVideoFrame& prv, int nt, float thr, float thr2, bool isY8);
array<bool, 4> CDuplicate_isDupFields(PVideoFrame& src, PVideoFrame& prv, int nt, float thr, float thr2, bool isY8);
