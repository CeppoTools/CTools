#pragma once
#include <cmath> 
#include <windows.h>
#include "avisynth.h"

class CDegrain : public GenericVideoFilter
{
    int radius;
    int nt;
    int thr;
    int blksize;
    PClip dClip;

public:
    CDegrain(PClip _child, int _radius, int _nt, int _thr, int _blksize, PClip _dClip, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

void CDegrain_Y8(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize);
void CDegrain_YV12(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize);
void CDegrain_YV16(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize);
void CDegrain_YV24(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize);
