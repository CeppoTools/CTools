#include "CTemporalSoften.h"

CTemporalSoften::CTemporalSoften(PClip _child, int _radius, bool _isb, bool _Y, bool _U, bool _V, IScriptEnvironment* env) : GenericVideoFilter(_child), radius(_radius), isb(_isb), Y(_Y), U(_U), V(_V)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CTemporalSoften: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (radius < 0 || radius > 7)
    {
        env->ThrowError("CTemporalSoften: radius values must be in the [1, 7] range!");
    }
}

PVideoFrame __stdcall CTemporalSoften::GetFrame(int n, IScriptEnvironment* env) {

    int i;
    PVideoFrame src[8];
    src[0] = child->GetFrame(n, env);

    for (i = 1; i < radius + 1; i += 1)
    {
        src[i] = isb ? child->GetFrame(max(n - i, 0), env) : child->GetFrame(min(n + i, vi.num_frames - 1), env);
    }
    PVideoFrame dst = env->NewVideoFrame(vi);

    unsigned char* dstp;
    const unsigned char* srcp[8];

    int dst_pitch;
    int src_pitch[8];
    int height;
    int row_size;
    int x, y, p;
    int64_t temp;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p += 1)
    {
        dstp = dst->GetWritePtr(planes[p]);
        dst_pitch = dst->GetPitch(planes[p]);
        height = dst->GetHeight(planes[p]);
        row_size = dst->GetRowSize(planes[p]);
        srcp[0] = src[0]->GetReadPtr(planes[p]);
        src_pitch[0] = src[0]->GetPitch(planes[p]);

        if (p > 0 && vi.IsY8())
        {
            break;
        }
        else if (p == 0 && !Y)
        {
            for (y = 0; y < height; y += 1)
            {
                memcpy(dstp, srcp[0], row_size);
                dstp += dst_pitch;
                srcp[0] += src_pitch[0];
            }
        }
        else if (p == 1 && !U)
        {
            for (y = 0; y < height; y += 1)
            {
                memcpy(dstp, srcp[0], row_size);
                dstp += dst_pitch;
                srcp[0] += src_pitch[0];
            }
        }
        else if (p == 2 && !V)
        {
            for (y = 0; y < height; y += 1)
            {
                memcpy(dstp, srcp[0], row_size);
                dstp += dst_pitch;
                srcp[0] += src_pitch[0];
            }
        }
        else
        {
            for (i = 1; i < radius + 1; i += 1)
            {
                srcp[i] = src[i]->GetReadPtr(planes[p]);
                src_pitch[i] = src[i]->GetPitch(planes[p]);
            }
            for (y = 0; y < height; y += 1)
            {
                for (x = 0; x < row_size; x += 1)
                {
                    temp = 0;
                    for (i = 0; i < radius + 1; i += 1)
                    {
                        temp += srcp[i][x];
                    }
                    dstp[x] = int(temp / static_cast<float>(radius + 1) + 0.5f);
                }
                dstp += dst_pitch;
                for (i = 0; i < radius + 1; i += 1)
                {
                    srcp[i] += src_pitch[i];
                }
            }
        }    
    }
    return dst;
}