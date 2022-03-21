#pragma once
#include <array>
#include <windows.h>
#include "avisynth.h"
using namespace std;

class CFieldDeBlend : public GenericVideoFilter
{
    float thr;
    float thr2;
    int nt;
    int thrC;
    int blksize;
    int blkthr;
    float sstr;
    PClip dClip;
    PClip childV;
    PClip dClipV;

public:
    CFieldDeBlend(PClip _child, float _thr, float _thr2, int _nt, int _thrC, int _blksize, int _blkthr, float _sstr, PClip _dClip, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

array<bool, 4> CFieldDeBlend_isDupFields(PVideoFrame& src, PVideoFrame& prv, int nt, float thr, float thr2, bool isY8);
bool CFieldDeBlend_isCombed(PVideoFrame& src, PVideoFrame& srv, int thr, int blksize, int blkthr, bool isY8);