#pragma once
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CSceneChange : public GenericVideoFilter
{
    PClip sClip;
    int blksize;
    int thSCD1;
    int thSCD2;
    int minKey;
    int maxLuma;
    bool write;
    bool inputTxt;
    ofstream file;
    ofstream file2;
    int count = 0;
    bool* isOut = new bool[vi.num_frames];
    bool* isOut2 = new bool[vi.num_frames];

public:
    CSceneChange(PClip _child, PClip _sClip, int _blksize, int _thSCD1, int _thSCD2, int _minKey, int _maxLuma, bool _write, bool _inputTxt, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};