#include "CInterpolateDuplicate.h"

CInterpolateDuplicate::CInterpolateDuplicate(PClip _child, float _thr, int _nt, PClip _dClip, PClip _iClip, IScriptEnvironment* env) : GenericVideoFilter(_child), thr(_thr), nt(_nt), dClip(_dClip), iClip(_iClip)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CInterpolateDuplicate: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (!iClip)
    {
        try
        {
            AVSValue sup_args[1] = { child };
            PClip Super = env->Invoke("MSuper", AVSValue(sup_args, 1)).AsClip();
            AVSValue vec_args[5] = { Super, 64, 32, true, 1 };
            const char* vec_names[5] = { NULL, "blksize", "overlap", "isb", "delta" };
            PClip backvec = env->Invoke("MAnalyse", AVSValue(vec_args, 5), vec_names).AsClip();
            vec_args[3] = false;
            PClip forwvec = env->Invoke("MAnalyse", AVSValue(vec_args, 5), vec_names).AsClip();
            AVSValue iClip_args[6] = { child, Super, backvec, forwvec, false, 50 };
            const char* iClip_names[6] = { NULL, NULL, NULL, NULL, "blend", "time" };
            iClip = env->Invoke("MFlowInter", AVSValue(iClip_args, 6), iClip_names).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CInterpolateDuplicate: mvtools2(.dll) not found!");
        }
    }
}

PVideoFrame __stdcall CInterpolateDuplicate::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame src = dClip ? dClip->GetFrame(n, env) : child->GetFrame(n, env);
    PVideoFrame prv = dClip ? dClip->GetFrame(max(0, n - 1), env) : child->GetFrame(max(0, n - 1), env);
    PVideoFrame nxt = dClip ? dClip->GetFrame(min(vi.num_frames - 1, n + 1), env) : child->GetFrame(min(vi.num_frames - 1, n + 1), env);

    bool isDup = CInterpolateDuplicate_isDup(src, prv, nxt, nt, thr, vi.IsY8());

    src = isDup ? iClip->GetFrame(n, env) : dClip ? child->GetFrame(n, env) : src;
    return src;
}

bool CInterpolateDuplicate_isDup(PVideoFrame& src, PVideoFrame& prv, PVideoFrame& nxt, int nt, float thr, bool isY8)
{
    const unsigned char* srcp;
    const unsigned char* prvp;
    const unsigned char* nxtp;

    int height;
    int row_size;
    int src_pitch;
    int prv_pitch;
    int nxt_pitch;
    int p, x, y;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    float AVG[3][3] = { { 0, 0, 0 }, { 0, 0, 0 } };
    int64_t SUM[3][3] = { { 0, 0, 0 }, { 0, 0, 0 } };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcp = src->GetReadPtr(planes[p]);
        prvp = prv->GetReadPtr(planes[p]);
        nxtp = nxt->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        prv_pitch = prv->GetPitch(planes[p]);
        nxt_pitch = nxt->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        for (y = 0; y < height; y += 1)
        {
            for (x = 0; x < row_size; x += 1)
            {
                SUM[0][p] += srcp[x];
                SUM[1][p] += abs(srcp[x] - prvp[x]) < nt ? srcp[x] : 255;
                SUM[2][p] += abs(srcp[x] - nxtp[x]) < nt ? srcp[x] : 255;
            }
            srcp += src_pitch;
            prvp += prv_pitch;
            nxtp += nxt_pitch;
        }
        AVG[0][p] = SUM[0][p] / static_cast<float>(height) / static_cast<float>(row_size);
        AVG[1][p] = SUM[1][p] / static_cast<float>(height) / static_cast<float>(row_size);
        AVG[2][p] = SUM[2][p] / static_cast<float>(height) / static_cast<float>(row_size);
    }
    return isY8 ? AVG[1][0] < AVG[0][0] + thr 
               && AVG[2][0] > AVG[0][0] + thr
                : AVG[1][0] + AVG[1][1] + AVG[1][2] < AVG[0][0] + AVG[0][1] + AVG[0][2] + thr
               && AVG[2][0] + AVG[2][1] + AVG[2][2] > AVG[0][0] + AVG[0][1] + AVG[0][2] + thr;
}