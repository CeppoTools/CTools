#pragma once
#include "CDuplicate.h"
#include "CReplaceDuplicate.h"
#include "CInterpolateDuplicate.h"
#include "CSharpen.h"
#include "CDeHalo.h"
#include "CAntiAliasing.h"
#include "CDegrain.h"
#include "CTemporalSoften.h"
#include "CSceneChange.h"
#include "CTelecine.h"
#include "CPostProcessing.h"
#include "CFieldDeBlend.h"
#include "CDecimate.h"

#include "CSetPattern.h"


AVSValue __cdecl Create_CDuplicate(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CDuplicate(args[0].AsClip(), args[1].AsFloatf(0.25f), args[2].AsFloatf(2.0f), args[3].AsInt(5), args[4].AsBool(false), args[5].IsClip() ? args[5].AsClip() : NULL, args[6].AsBool(false), env);
}

AVSValue __cdecl Create_CReplaceDuplicate(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CReplaceDuplicate(args[0].AsClip(), args[1].AsBool(false), env);
}

AVSValue __cdecl Create_CInterpolateDuplicate(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CInterpolateDuplicate(args[0].AsClip(), args[1].AsFloatf(0.25f), args[2].AsInt(5), args[3].IsClip() ? args[3].AsClip() : NULL, args[4].IsClip() ? args[4].AsClip() : NULL, env);
}

AVSValue __cdecl Create_CSharpen(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CSharpen(args[0].AsClip(), args[1].IsClip() ? args[1].AsClip() : NULL, args[2].AsInt(5), args[3].AsInt(1), args[4].AsBool(true), args[5].AsBool(false), args[6].AsBool(false), env);
}

AVSValue __cdecl Create_CDeHalo(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CDeHalo(args[0].AsClip(), args[1].IsClip() ? args[1].AsClip() : NULL, args[2].AsInt(5), args[3].AsInt(1), args[4].AsBool(true), args[5].AsBool(false), args[6].AsBool(false), env);
}

AVSValue __cdecl Create_CAntiAliasing(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CAntiAliasing(args[0].AsClip(), args[1].AsInt(5), args[2].AsInt(0), args[3].AsBool(true), args[4].AsBool(false), args[5].AsBool(false), env);
}

AVSValue __cdecl Create_CDegrain(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CDegrain(args[0].AsClip(), args[1].AsInt(1), args[2].AsInt(5), args[3].AsInt(32), args[4].AsInt(4), args[5].IsClip() ? args[5].AsClip() : NULL, env);
}

AVSValue __cdecl Create_CTemporalSoften(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CTemporalSoften(args[0].AsClip(), args[1].AsInt(1), args[2].AsBool(false), args[3].AsBool(true), args[4].AsBool(true), args[5].AsBool(true), env);
}

AVSValue __cdecl Create_CSceneChange(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CSceneChange(args[0].AsClip(), args[1].IsClip() ? args[1].AsClip() : NULL, args[2].AsInt(64), args[3].AsInt(400), args[4].AsInt(130), args[5].AsInt(30), args[6].AsInt(128), args[7].AsBool(true), args[8].AsBool(false), env);
}

AVSValue __cdecl Create_CTelecine(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CTelecine(args[0].AsClip(), args[1].AsBool(false), args[2].AsInt(5), args[3].AsInt(0), args[4].AsInt(args[3].AsInt(0)), args[5].AsBool(true), args[6].AsFloatf(1.0f), args[7].AsInt(10), args[8].AsFloatf(2.7f), args[9].AsFloatf(0.005f), args[10].IsClip() ? args[10].AsClip() : NULL, args[11].AsBool(false), args[12].AsBool(false), env);
}

AVSValue __cdecl Create_CPostProcessing(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CPostProcessing(args[0].AsClip(), args[1].AsInt(3), args[2].AsInt(6), args[3].AsInt(0), args[4].AsInt(8), args[5].AsInt(32), args[6].AsBool(false), args[7].AsFloatf(2.7f), args[8].AsInt(10), args[9].AsInt(3), args[10].IsClip() ? args[10].AsClip() : NULL, args[11].IsClip() ? args[11].AsClip() : NULL, args[12].IsClip() ? args[12].AsClip() : NULL, env);
}

AVSValue __cdecl Create_CFieldDeBlend(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CFieldDeBlend(args[0].AsClip(), args[1].AsFloatf(0.005f), args[2].AsFloatf(0.25f), args[3].AsInt(5), args[4].AsInt(1), args[5].AsInt(8), args[6].AsInt(32), args[7].AsFloatf(2.7f), args[8].IsClip() ? args[8].AsClip() : NULL, env);
}

AVSValue __cdecl Create_CDecimate(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CDecimate(args[0].AsClip(), args[1].AsBool(false), args[2].AsBool(true), args[3].AsBool(true), args[4].IsClip() ? args[4].AsClip() : NULL, env);
}

AVSValue __cdecl Create_CSetPattern(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return new CSetPattern(args[0].AsClip(), args[1].AsInt(), args[2].AsInt(), args[3].AsBool(false), args[4].AsBool(false), args[5].AsBool(false), args[6].AsBool(false), args[7].AsBool(false), args[8].AsBool(false), env);
}

const AVS_Linkage* AVS_linkage = 0;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment * env, const AVS_Linkage* const vectors)
{
    AVS_linkage = vectors;
    env->AddFunction("CDuplicate", "c[thr]f[thr2]f[nt]i[fields]b[dClip]c[write]b", Create_CDuplicate, 0);
    env->AddFunction("CReplaceDuplicate", "c[drop]b", Create_CReplaceDuplicate, 0);
    env->AddFunction("CInterpolateDuplicate", "c[thr]f[nt]i[dClip]c[iClip]c", Create_CInterpolateDuplicate, 0);
    env->AddFunction("CSharpen", "cc[nt]i[mode]i[Y]b[U]b[V]b", Create_CSharpen, 0);
    env->AddFunction("CDeHalo", "cc[nt]i[mode]i[Y]b[U]b[V]b", Create_CDeHalo, 0);
    env->AddFunction("CAntiAliasing", "c[nt]i[mode]i[Y]b[U]b[V]b", Create_CAntiAliasing, 0);
    env->AddFunction("CDegrain", "c[radius]i[nt]i[thr]i[blksize]i[dClip]c", Create_CDegrain, 0);
    env->AddFunction("CTemporalSoften", "c[radius]i[isb]b[Y]b[U]b[V]b", Create_CTemporalSoften, 0);
    env->AddFunction("CSceneChange", "c[sClip]c[blksize]i[thSCD1]i[thSCD2]i[minKey]i[maxLuma]i[write]b[inputTxt]b", Create_CSceneChange, 0);
    env->AddFunction("CTelecine", "c[bob]b[mode]i[nt]i[ntN]i[sse]b[thr60i]f[nt60i]i[sstr]f[mode2thr]f[dClip]c[write]b[inputTxt]b", Create_CTelecine, 0);
    env->AddFunction("CPostProcessing", "c[thr]i[thr2]i[mode]i[blksize]i[blkthr]i[isBob]b[sstr]f[nt]i[ntMask]i[edeint]c[edeint2]c[dClip]c", Create_CPostProcessing, 0);
    env->AddFunction("CFieldDeBlend", "c[thr]f[thr2]f[nt]i[thrC]i[blksize]i[blkthr]i[sstr]f[dClip]c", Create_CFieldDeBlend, 0);
    env->AddFunction("CDecimate", "c[isBob]b[error]b[heuristic]b[iClip]c", Create_CDecimate, 0);
    env->AddFunction("CSetPattern", "c[start]i[end]i[CCCCC]b[NNCCC]b[CNNCC]b[CCNNC]b[CCCNN]b[NCCCN]b", Create_CSetPattern, 0);

    return "CTools plugin v1.2.0";
}