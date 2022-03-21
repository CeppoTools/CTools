#include "CPostProcessing.h"

CPostProcessing::CPostProcessing(PClip _child, int _thr, int _thr2, int _mode, int _blksize, int _blkthr, bool _isBob, float _sstr, int _nt, int _ntMask, PClip _edeint, PClip _edeint2, IScriptEnvironment* env)
    : GenericVideoFilter(_child), thr(_thr), thr2(_thr2), mode(_mode), blksize(_blksize), blkthr(_blkthr), isBob(_isBob), sstr(_sstr), nt(_nt), ntMask(_ntMask), edeint(_edeint), edeint2(_edeint2)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CPostProcessing: supported colospaces are Y8, YV12, YV16, YV24!");
    }
    else if (mode < 0 || mode > 3)
    {
        env->ThrowError("CPostProcessing: mode values must be in the [0, 3] range!");
    }
    else if (blksize < 4)
    {
        env->ThrowError("CPostProcessing: min. blksize value is 4!");
    }

    C = child;
    if (mode == 0)
    {
        try
        {
            AVSValue args[2];
            args[0] = C;
            args[1] = sstr;
            CV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CPostProcessing: vinverse(.dll) function not found!");
        }
    }
    else if (mode == 1)
    {
        try
        {
            AVSValue args[2];
            args[0] = C;
            args[1] = sstr;
            CV = env->Invoke("ex_vinverse", AVSValue(args, 2)).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CTelecineNew: ex_vinverse(.avsi) function not found!");
        }
    }

    if (!edeint2 && !isBob)
    {
        try
        {
            edeint2 = env->Invoke("QTGMC", AVSValue({ C }, 1)).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CPostProcessing: QTGMC(.avsi) function not found!");
        }
    }
    else if (!edeint2)
    {
        env->ThrowError("CPostProcessing: for isBob=True a edeint2 clip must be specified!");
    }

    if (!edeint)
    {
        try
        {
            AVSValue args[4];
            args[0] = C;
            args[1] = edeint2;
            args[2] = ntMask;
            args[3] = isBob;
            edeint = env->Invoke("CPostProcessingMask", AVSValue(args, 4)).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CPostProcessing: CPostProcessingMask(CTools.avsi) function not found!");
        }
    }
}

PVideoFrame __stdcall CPostProcessing::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame c = C->GetFrame(n, env);
    PVideoFrame mask = env->NewVideoFrame(vi);
    PVideoFrame cv = mode == 0 || mode == 1 ? CV->GetFrame(n, env) : NULL;

    int order = child->GetParity(0) ? 1 : 0;
    bool is60i = mode == 0 || mode == 1 ? CPostProcessing_mode01(c, cv, mask, order, nt, thr, thr2, vi.IsY8())
               : mode == 2 ? CPostProcessing_mode2(c, mask, order, thr, thr2, vi.IsY8())
               : CPostProcessing_mode3(c, mask, order, thr, thr2, vi.IsY8());
    
    if (is60i)
    {
        PVideoFrame ed2 = isBob ? edeint2->GetFrame(n, env) : edeint2->GetFrame(n * 2 + 1, env);
        return ed2;
    }
    else if (CPostProcessing_isCombed(mask, mode, order, blkthr, blksize, vi.IsY8()))
    {
        PVideoFrame ed1 = isBob ? edeint->GetFrame(n, env) : edeint->GetFrame(n * 2 + 1, env);
        //env->MakeWritable(&c);
        //CPostProcessing_ReplaceCombedPixels(c, ed1, mask, mode, order);
        return ed1;
    }
    else
    {
        return c;
    }
}

bool CPostProcessing_mode01(PVideoFrame& src, PVideoFrame& srv, PVideoFrame& mask, int order, int nt, float thr, float thr2, bool isY8)
{
    unsigned char* maskp;
    const unsigned char* srcp;
    const unsigned char* srvp;

    int height;
    int row_size;
    int src_pitch;
    int srv_pitch;
    int mask_pitch;
    int p, x, y, temp;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    float avg[] = { 0, 0, 0 };
    int64_t sum[] = { 0, 0, 0 };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcp = src->GetReadPtr(planes[p]);
        srvp = srv->GetReadPtr(planes[p]);
        maskp = mask->GetWritePtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        srv_pitch = srv->GetPitch(planes[p]);
        mask_pitch = mask->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        if (order == 0)
        {
            srcp += src_pitch;
            srvp += srv_pitch;
            maskp += mask_pitch;
        }

        for (y = 0; y < height; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                temp = abs(srcp[x] - srvp[x]);
                sum[p] += temp > nt ? 255 : 0;
                maskp[x] = temp > thr ? 1 : 0;
            }
            srcp += src_pitch * static_cast<int64_t>(2);
            srvp += srv_pitch * static_cast<int64_t>(2);
            maskp += mask_pitch * static_cast<int64_t>(2);
        }
        avg[p] = sum[p] / static_cast<float>(height / 2) / static_cast<float>(row_size);
    }
    return isY8 ?  avg[0] > thr2 / 6.0f
                : (avg[0] + avg[1] + avg[2]) / 3.0f > thr2 / 6.0f;
}

bool CPostProcessing_mode2(PVideoFrame& src, PVideoFrame& mask, int order, float thr, float thr2, bool isY8)
{
    unsigned char* maskp;
    const unsigned char* srcp;
    const unsigned char* srcpp;
    const unsigned char* srcpn;

    int height;
    int row_size;
    int src_pitch;
    int mask_pitch;
    float avg = 0;
    int64_t sum = 0;
    int p, x, y, temp;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };
    

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcp = src->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        maskp = mask->GetWritePtr(planes[p]);
        mask_pitch = mask->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        srcp += src_pitch;
        srcpp = srcp - src_pitch;
        srcpn = srcp + src_pitch;
        maskp += mask_pitch;

        if (order == 1)
        {
            srcp += src_pitch;
            srcpp += src_pitch;
            srcpn += src_pitch;
            maskp += mask_pitch;
        }

        for (y = 0; y < height - 2; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                temp = (srcpp[x] - srcp[x]) * (srcpn[x] - srcp[x]);
                sum += temp;
                maskp[x] = temp > thr * thr ? 1 : 0;
            }
            srcp += src_pitch * static_cast<int64_t>(2);
            srcpp += src_pitch * static_cast<int64_t>(2);
            srcpn += src_pitch * static_cast<int64_t>(2);
            maskp += mask_pitch * static_cast<int64_t>(2);
        }
    }
    int src_width[2];
    int src_height[2];
    src_width[0] = src->GetRowSize(PLANAR_Y);
    src_height[0] = src->GetHeight(PLANAR_Y);
    src_width[1] = src->GetRowSize(PLANAR_U);
    src_height[1] = src->GetHeight(PLANAR_U);

    avg = isY8 ? sum / (static_cast<float>(src_width[0]) * static_cast<float>(src_height[0] - 2) / 2.0f)
        :        sum / (static_cast<float>(src_width[0]) * static_cast<float>(src_height[0] - 2) / 2.0f + static_cast<float>(src_width[1]) * static_cast<float>(src_height[1] - 2));

    return avg > thr2 * thr2;
}

bool CPostProcessing_mode3(PVideoFrame& src, PVideoFrame& mask, int order, float thr, float thr2, bool isY8)
{
    unsigned char* maskp;
    const unsigned char* srcp;
    const unsigned char* srcpp;
    const unsigned char* srcpn;
    const unsigned char* srcppp;
    const unsigned char* srcpnn;

    int height;
    int row_size;
    int src_pitch;
    int mask_pitch;
    float avg = 0;
    int64_t sum = 0;
    int p, x, y, c1, c2, temp;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };


    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcp = src->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        maskp = mask->GetWritePtr(planes[p]);
        mask_pitch = mask->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        srcp += src_pitch * static_cast<int64_t>(2);
        maskp += mask_pitch * static_cast<int64_t>(2);

        srcpp = srcp - src_pitch;
        srcpn = srcp + src_pitch;
        srcppp = srcpp - src_pitch;
        srcpnn = srcpn + src_pitch;

        if (order == 0)
        {
            srcp += src_pitch;
            srcpp += src_pitch;
            srcpn += src_pitch;
            srcppp += src_pitch;
            srcpnn += src_pitch;
            maskp += mask_pitch;
        }

        for (y = 0; y < height - 4; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                c1 = (srcp[x] - srcpp[x]);
                c2 = (srcp[x] - srcpn[x]);
                temp = abs((srcppp[x] + (srcp[x] << 2) + srcpnn[x]) - 3 * (srcpp[x] + srcpn[x]));
                sum += temp;
                maskp[x] = (c1 > thr && c2 > thr) || (c1 < -thr && c2 < -thr) && temp > thr * 6 ? 1 : 0;
            }
            srcp += src_pitch * static_cast<int64_t>(2);
            srcpp += src_pitch * static_cast<int64_t>(2);
            srcppp += src_pitch * static_cast<int64_t>(2);
            srcpn += src_pitch * static_cast<int64_t>(2);
            srcpnn += src_pitch * static_cast<int64_t>(2);
            maskp += mask_pitch * static_cast<int64_t>(2);
        }
    }
    int src_width[2];
    int src_height[2];
    src_width[0] = src->GetRowSize(PLANAR_Y);
    src_height[0] = src->GetHeight(PLANAR_Y);
    src_width[1] = src->GetRowSize(PLANAR_U);
    src_height[1] = src->GetHeight(PLANAR_U);
    
    avg = isY8 ? sum / (static_cast<float>(src_width[0]) * static_cast<float>(src_height[0] - 4) / 2.0f)
        :        sum / (static_cast<float>(src_width[0]) * static_cast<float>(src_height[0] - 4) / 2.0f + static_cast<float>(src_width[1]) * static_cast<float>(src_height[1] - 4));

    return avg > thr2 * 6;
}

bool CPostProcessing_isCombed(PVideoFrame& mask, int mode, int order, int blkthr, int blksize, bool isY8)
{
    const unsigned char* maskp;

    int height;
    int row_size;
    int mask_pitch;

    int sum = 0;
    int p, x, y, a, b;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p += 1)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        maskp = mask->GetReadPtr(planes[p]);
        mask_pitch = mask->GetPitch(planes[p]);

        height = mask->GetHeight(planes[p]);
        row_size = mask->GetRowSize(planes[p]);
        height = mode == 2 ? height - 2 
               : mode == 3 ? height - 4
               : height;

        maskp += mode == 0 && order == 0 ? mask_pitch
               : mode == 1 && order == 0 ? mask_pitch
               : mode == 2 && order == 0 ? mask_pitch
               : mode == 2 && order == 1 ? mask_pitch * static_cast<int64_t>(2)
               : mode == 3 && order == 0 ? mask_pitch * static_cast<int64_t>(3)
               : mode == 3 && order == 1 ? mask_pitch * static_cast<int64_t>(2)
               : 0;

        for (y = 0; y < height; y += blksize * 2)
        {
            for (x = 0; x < row_size; x += blksize)
            {
                for (a = 0; a < blksize * 2 && a < height - y; a += 2)
                {
                    for (b = x; b < x + blksize && b < row_size; b += 1)
                    {
                        sum += maskp[b];
                    }
                    maskp += mask_pitch * static_cast<int64_t>(2);
                }
                maskp -= mask_pitch * static_cast<int64_t>(min(height - y, blksize * 2));

                if (sum >= blkthr)
                {
                    return true;
                }
                else
                {
                    sum = 0;
                }
            }
            maskp += mask_pitch * static_cast<int64_t>(min(height - y, blksize * 2));
        }
    }
    return false;
}

// SEMANTIC BUG HUNTER
void CPostProcessing_ReplaceCombedPixels(PVideoFrame& src, PVideoFrame& edt, PVideoFrame& mask, int mode, int order)
{
    unsigned char* srcp;
    const unsigned char* edtp;
    const unsigned char* mskp;

    int height;
    int row_size;
    int src_pitch;
    int edt_pitch;
    int msk_pitch;
    int p, x, y;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p += 1)
    {
        srcp = src->GetWritePtr(planes[p]);
        edtp = edt->GetReadPtr(planes[p]);
        mskp = mask->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        edt_pitch = edt->GetPitch(planes[p]);
        msk_pitch = mask->GetPitch(planes[p]);

        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);
        height = mode == 2 ? height - 2
               : mode == 3 ? height - 4
               : height;

        srcp += mode == 0 && order == 0 ? src_pitch
              : mode == 1 && order == 0 ? src_pitch
              : mode == 2 && order == 0 ? src_pitch
              : mode == 2 && order == 1 ? src_pitch * static_cast<int64_t>(2)
              : mode == 3 && order == 0 ? src_pitch * static_cast<int64_t>(3)
              : mode == 3 && order == 1 ? src_pitch * static_cast<int64_t>(2)
              : 0;
        edtp += mode == 0 && order == 0 ? edt_pitch
              : mode == 1 && order == 0 ? edt_pitch
              : mode == 2 && order == 0 ? edt_pitch
              : mode == 2 && order == 1 ? edt_pitch * static_cast<int64_t>(2)
              : mode == 3 && order == 0 ? edt_pitch * static_cast<int64_t>(3)
              : mode == 3 && order == 1 ? edt_pitch * static_cast<int64_t>(2)
              : 0;
        mskp += mode == 0 && order == 0 ? msk_pitch
              : mode == 1 && order == 0 ? msk_pitch
              : mode == 2 && order == 0 ? msk_pitch
              : mode == 2 && order == 1 ? msk_pitch * static_cast<int64_t>(2)
              : mode == 3 && order == 0 ? msk_pitch * static_cast<int64_t>(3)
              : mode == 3 && order == 1 ? msk_pitch * static_cast<int64_t>(2)
              : 0;

        for (y = 0; y < height; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                srcp[x] = mskp[x] ? edtp[x] : srcp[x];
            }
            srcp += src_pitch * static_cast<int64_t>(2);
            edtp += edt_pitch * static_cast<int64_t>(2);
            mskp += msk_pitch * static_cast<int64_t>(2);
        }
    }
}