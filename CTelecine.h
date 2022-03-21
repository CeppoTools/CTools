#pragma once
#include <cmath> 
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CTelecine : public GenericVideoFilter
{
    PClip C;
    PClip N;
    PClip CV;
    PClip NV;
    PClip dClip;
    PClip dClipN;
    PClip dClipV;
    PClip dClipNV;

    float sstr;
    int nt;
    int ntN;
    bool bob;
    bool write;
    bool inputTxt;
    int mode;
    int nt60i;
    float thr60i;
    float mode2thr;
    bool sse;
    ofstream file;
    ofstream file2;

    int* vec = new int[vi.num_frames];
    int* vec2 = new int[vi.num_frames];
    bool* isOut = new bool[vi.num_frames];
    bool* isOut2 = new bool[vi.num_frames];
    
public:
    CTelecine(PClip _child, bool _bob, int _mode, int _nt, int _ntN, bool _sse, float _thr60i, int _nt60i, float _sstr, float _mode2thr, PClip _dClip, bool _write, bool _inputTxt,  IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int f, IScriptEnvironment* env);
};

bool CTelecine_is60i(PVideoFrame& c, PVideoFrame& cv, int nt, float thr, bool isY8);
bool CTelecine_mode01(PVideoFrame& c, PVideoFrame& cv, PVideoFrame& n, PVideoFrame& nv, int nt, bool isY8);
bool CTelecine_mode56(PVideoFrame& c, PVideoFrame& cv, PVideoFrame& n, PVideoFrame& nv, int nt, int ntN, bool sse, bool isY8);
int CTelecine_mode2(PVideoFrame& src, PVideoFrame& nxt, int nt, float thr, bool isY8);
bool CTelecine_mode3(PVideoFrame& src, PVideoFrame& nxt, int nt, int ntN, bool sse, int order, bool isY8);
bool CTelecine_mode4(PVideoFrame& src, PVideoFrame& nxt, int nt, int ntN, bool sse, int order, bool isY8);
