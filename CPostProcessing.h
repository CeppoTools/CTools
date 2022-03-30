#pragma once
#include <cmath> 
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CPostProcessing : public GenericVideoFilter
{
    PClip C;
    PClip CV;
    int thr;
    int thr2;
    int nt;
    int ntMask;
    float sstr;
    bool isBob;
    PClip edeint;
    PClip edeint2;
    int mode;
    int blkthr;
    int blksize;
    PClip dClip;
    PClip dClipV;

public:
    CPostProcessing(PClip _child, int _thr, int _thr2, int _mode, int _blksize, int _blkthr, bool _isBob, float _sstr, int _nt, int _ntMask, PClip _edeint, PClip _edeint2, PClip _dClip, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

bool CPostProcessing_mode01(PVideoFrame& src, PVideoFrame& srv, PVideoFrame& mask, int order, int nt, float thr, float thr2, bool isY8);
bool CPostProcessing_mode2(PVideoFrame& src, PVideoFrame& mask, int order, float thr, float thr2, bool isY8);
bool CPostProcessing_mode3(PVideoFrame& src, PVideoFrame& mask, int order, float thr, float thr2, bool isY8);
bool CPostProcessing_isCombed(PVideoFrame& mask, int mode, int order, int blkthr, int blksize, bool isY8);
void CPostProcessing_ReplaceCombedPixels(PVideoFrame& src, PVideoFrame& edt, PVideoFrame& mask, int mode, int order);

