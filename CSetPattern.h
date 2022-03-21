#pragma once
#include <cmath> 
#include <fstream>
#include <iostream>
#include <windows.h>
#include "avisynth.h"
#include <string>
using namespace std;

class CSetPattern : public GenericVideoFilter
{
    int start;
    int end;
    bool CCCCC;
    bool NNCCC;
    bool CNNCC;
    bool CCNNC;
    bool CCCNN;
    bool NCCCN;

public:
    CSetPattern(PClip _child, int _start, int _end, bool _CCCCC, bool _NNCCC, bool _CNNCC, bool _CCNNC, bool _CCCNN, bool _NCCCN, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};