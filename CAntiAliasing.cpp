#include "CAntiAliasing.h"

CAntiAliasing::CAntiAliasing(PClip _child, int _nt, int _mode, bool _Y, bool _U, bool _V, IScriptEnvironment* env) : GenericVideoFilter(_child), nt(_nt), mode(_mode), Y(_Y), U(_U), V(_V)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CAntiAliasing: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (nt < 0 || nt > 255)
    {
        env->ThrowError("CAntiAliasing: nt avaible range is [0, 255]!");
    }
    else if (mode < 0 || mode > 2)
    {
        env->ThrowError("CAntiAliasing: mode avaible mode values are 0, 1, 2!");
    }
}

PVideoFrame __stdcall CAntiAliasing::GetFrame(int n, IScriptEnvironment* env) {

    PVideoFrame dst = env->NewVideoFrame(vi);
    PVideoFrame src = child->GetFrame(n, env);

    unsigned char* dstp;
    const unsigned char* srcp;
    const unsigned char* srcpp;
    const unsigned char* srcpn;

    int height;
    int row_size;
    int src_pitch;
    int dst_pitch;
    int x, y, p, temp[3];
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p++)
    {
        dstp = dst->GetWritePtr(planes[p]);
        srcp = src->GetReadPtr(planes[p]);
        dst_pitch = dst->GetPitch(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        srcpp = srcp - src_pitch;
        srcpn = srcp + src_pitch;

        if (p > 0 && vi.IsY8())
        {
            break;
        }
        else if (p == 0 && !Y)
        {
            for (y = 0; y < height; y += 1)
            {
                memcpy(dstp, srcp, row_size);
                dstp += dst_pitch;
                srcp += src_pitch;
            }
        }
        else if (p == 1 && !U)
        {
            for (y = 0; y < height; y += 1)
            {
                memcpy(dstp, srcp, row_size);
                dstp += dst_pitch;
                srcp += src_pitch;
            }
        }
        else if (p == 2 && !V)
        {
            for (y = 0; y < height; y += 1)
            {
                memcpy(dstp, srcp, row_size);
                dstp += dst_pitch;
                srcp += src_pitch;
            }
        }
        else
        {
            for (x = 0; x < row_size; x += 1)
            {
                dstp[x] = srcp[x];
            }
            dstp += dst_pitch;
            srcp += src_pitch;
            srcpp += src_pitch;
            srcpn += src_pitch;
            for (y = 1; y < height - 1; y += 1)
            {
                dstp[0] = srcp[0];
                dstp[row_size - 1] = srcp[row_size - 1];
                for (x = 1; x < row_size - 1; x += 1)
                {
                    temp[0] = int((srcp[x] + (static_cast<int64_t>(srcpp[x - 1]) + srcpp[x] + srcpp[x + 1] + srcpn[x - 1] + srcpn[x] + srcpn[x + 1] + srcp[x - 1] + srcp[x + 1]) / 8.0) / 2.0 + 0.5f);
                    temp[1] = srcp[x] - temp[0];
                    temp[1] = abs(temp[1]) < nt ? 0 : temp[1];

                    if (mode == 1)
                    {
                        
                        temp[2] = int(sqrt(abs(temp[1]))) * ((temp[1] > 0) - (temp[1] < 0));
                        dstp[x] = max(0, min(255, temp[0] + temp[2]));
                    }
                    else if (mode == 2)
                    {
                        temp[2] = int(atan(abs(temp[1])) * sqrt(abs(temp[1]))) * ((temp[1] > 0) - (temp[1] < 0));
                        dstp[x] = max(0, min(255, temp[0] + temp[2]));
                    }
                    else
                    {
                        dstp[x] = max(0, min(255, srcp[x] - temp[1]));
                    } 
                }
                dstp += dst_pitch;
                srcp += src_pitch;
                srcpp += src_pitch;
                srcpn += src_pitch;
            }
            for (x = 0; x < row_size; x += 1)
            {
                dstp[x] = srcp[x];
            }
        }
    }
    return dst;
}