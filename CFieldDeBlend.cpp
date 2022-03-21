#include "CFieldDeBlend.h"

CFieldDeBlend::CFieldDeBlend(PClip _child, float _thr, float _thr2, int _nt, int _thrC, int _blksize, int _blkthr, float _sstr, PClip _dClip, IScriptEnvironment* env)
    : GenericVideoFilter(_child), thr(_thr), nt(_nt), thrC(_thrC), blksize(_blksize), blkthr(_blkthr), sstr(_sstr), dClip(_dClip)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CFieldDeBlend: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (thr < 0 || thr > 255)
    {
        env->ThrowError("CFieldDeBlend: thr value must be in the [0, 255] range!");
    }
    else if (nt < 0 || nt > 255)
    {
        env->ThrowError("CFieldDeBlend: nt value must be in the [0, 255] range!");
    }
    else if (thrC < 0 || thrC > 255)
    {
        env->ThrowError("CFieldDeBlend: thrC value must be in the [0, 255] range!");
    }

    try
    {
        AVSValue args[2];
        args[0] = child;
        args[1] = sstr;
        childV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
        if (dClip)
        {
            args[0] = dClip;
            dClipV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
        }
    }
    catch (IScriptEnvironment::NotFound)
    {
        env->ThrowError("CFieldDeBlend: vinverse(.dll) function not found!");
    }
}

PVideoFrame __stdcall CFieldDeBlend::GetFrame(int n, IScriptEnvironment* env) 
{    
    PVideoFrame src  = dClip ? dClip->GetFrame(n, env)  : child->GetFrame(n, env);
    PVideoFrame srcV = dClip ? dClipV->GetFrame(n, env) : childV->GetFrame(n, env);
    bool isCombed = CFieldDeBlend_isCombed(src, srcV, thrC, blksize, blkthr, vi.IsY8());
    
    if (isCombed)
    {
        PVideoFrame prv = dClip ? dClip->GetFrame(max(n - 1, 0), env) : child->GetFrame(max(n - 1, 0), env);
        PVideoFrame prvV = dClip ? dClipV->GetFrame(max(n - 1, 0), env) : childV->GetFrame(max(n - 1, 0), env);
        PVideoFrame nxt = dClip ? dClip->GetFrame(min(n + 1, vi.num_frames - 1), env) : child->GetFrame(min(n + 1, vi.num_frames - 1), env);
        PVideoFrame nxtV = dClip ? dClipV->GetFrame(min(n + 1, vi.num_frames - 1), env) : childV->GetFrame(min(n + 1, vi.num_frames - 1), env);
        bool isCombedP = CFieldDeBlend_isCombed(prv, prvV, thrC, blksize, blkthr, vi.IsY8());
        bool isCombedN = CFieldDeBlend_isCombed(nxt, nxtV, thrC, blksize, blkthr, vi.IsY8());
        array<bool, 4> isP = CFieldDeBlend_isDupFields(src, prv, nt, thr, thr2, vi.IsY8());
        array<bool, 4> isN = CFieldDeBlend_isDupFields(src, nxt, nt, thr, thr2, vi.IsY8());

        if (((isP[0] && !isP[1]) || (!isP[0] && isP[1])) && !isCombedP
         || ((isP[2] && !isP[3]) || (!isP[2] && isP[3])) && !isCombedP)
        {
            prv = dClip ? child->GetFrame(max(n - 1, 0), env) : prv;
            return prv;
        }
        else if (((isN[0] && !isN[1]) || (!isN[0] && isN[1])) && !isCombedN
             ||  ((isN[2] && !isN[3]) || (!isN[2] && isN[3])) && !isCombedN) 
        {
            nxt = dClip ? child->GetFrame(min(n + 1, vi.num_frames - 1), env) : nxt;
            return nxt;
        }
        else
        {
            src = dClip ? child->GetFrame(n, env) : src;
            return src;
        }
    }
    else
    {
        src = dClip ? child->GetFrame(n, env) : src;
        return src;
    }
}

array<bool, 4> CFieldDeBlend_isDupFields(PVideoFrame& src, PVideoFrame& prv, int nt, float thr, float thr2, bool isY8)
{
    const unsigned char* srcp;
    const unsigned char* prvp;

    int height;
    int row_size;
    int src_pitch;
    int prv_pitch;
    int p, x, y;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    float AVG[4][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    int64_t SUM[4][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcp = src->GetReadPtr(planes[p]);
        prvp = prv->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        prv_pitch = prv->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        for (y = 0; y < height; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                SUM[0][p] += srcp[x];
                SUM[1][p] += abs(srcp[x] - prvp[x]) < nt ? srcp[x] : 255;
            }
            srcp += src_pitch;
            prvp += prv_pitch;
            for (x = 0; x < row_size; x += 1)
            {
                SUM[2][p] += srcp[x];
                SUM[3][p] += abs(srcp[x] - prvp[x]) < nt ? srcp[x] : 255;
            }
            srcp += src_pitch;
            prvp += prv_pitch;
        }
        AVG[0][p] = SUM[0][p] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
        AVG[1][p] = SUM[1][p] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
        AVG[2][p] = SUM[2][p] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
        AVG[3][p] = SUM[3][p] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
    }
    array<bool, 4> isDup;
    isDup[0] = AVG[1][0] + AVG[1][1] + AVG[1][2] < AVG[0][0] + AVG[0][1] + AVG[0][2] + thr;
    isDup[1] = AVG[3][0] + AVG[3][1] + AVG[3][2] < AVG[2][0] + AVG[2][1] + AVG[2][2] + thr;
    isDup[2] = AVG[1][0] + AVG[1][1] + AVG[1][2] < AVG[0][0] + AVG[0][1] + AVG[0][2] + thr2;
    isDup[3] = AVG[3][0] + AVG[3][1] + AVG[3][2] < AVG[2][0] + AVG[2][1] + AVG[2][2] + thr2;
    return isDup;
}

bool CFieldDeBlend_isCombed(PVideoFrame& src, PVideoFrame& srv, int thr, int blksize, int blkthr, bool isY8)
{
    int p, x, y;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p += 1)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        const unsigned char* srcp = src->GetReadPtr(planes[p]);
        const unsigned char* srvp = srv->GetReadPtr(planes[p]);
        int src_pitch = src->GetPitch(planes[p]);
        int srv_pitch = srv->GetPitch(planes[p]);
        int height = src->GetHeight(planes[p]);
        int row_size = src->GetRowSize(planes[p]);

        int BY, BX;
        int64_t BSY = 0;
        int64_t BSX = 0;
        BSY += min(1, height % blksize);
        BSX += min(1, row_size % blksize);
        BSY += height / blksize;
        BSX += row_size / blksize;
        auto SUM = new int[BSX * BSY];
        memset(SUM, 0, BSX * BSY * sizeof(int));

        for (y = 0; y < height; y += 1)
        {
            BY = y / blksize;
            BY += y % blksize ? 1 : 0;
            for (x = 0; x < row_size; x += 1)
            {
                BX = x / blksize;
                BX += x % blksize ? 1 : 0;
                SUM[BY * BSY + BX] += abs(srcp[x] - srvp[x]) > thr ? 1 : 0;
            }
            srcp += src_pitch;
            srvp += srv_pitch;
        }

        for (y = 0; y < BSY; y += 1)
        {
            for (x = 0; x < BSX; x += 1)
            {
                if (SUM[y * BSY + x] >= blkthr)
                {
                    return true;
                }
            }
        }
    }
    return false;
}