#include "CSharpen.h"

CSharpen::CSharpen(PClip _child, PClip _bClip, int _nt, int _mode, bool _Y, bool _U, bool _V, IScriptEnvironment* env) : GenericVideoFilter(_child), bClip(_bClip), nt(_nt), mode(_mode), Y(_Y), U(_U), V(_V)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CSharpen: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (nt < 0 || nt > 255)
    {
        env->ThrowError("CSharpen: nt avaible range is [0, 255]!");
    }
    else if (mode < 0 || mode > 2)
    {
        env->ThrowError("CSharpen: mode avaible mode values are 0, 1, 2!");
    }
    else if (!bClip)
    {
        env->ThrowError("CSharpen: a blur clip must be specified!");
    }

}

PVideoFrame __stdcall CSharpen::GetFrame(int n, IScriptEnvironment* env) {

    PVideoFrame dst = env->NewVideoFrame(vi);
    PVideoFrame src = child->GetFrame(n, env);
    PVideoFrame blu = bClip->GetFrame(n, env);
    
    unsigned char* dstp;
    const unsigned char* srcp;
    const unsigned char* blup;

    int height;
    int row_size;
    int src_pitch;
    int blu_pitch;
    int dst_pitch;
    int x, y, p, temp;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p++)
    {
        dstp = dst->GetWritePtr(planes[p]);
        srcp = src->GetReadPtr(planes[p]);
        blup = blu->GetReadPtr(planes[p]);
        dst_pitch = dst->GetPitch(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        blu_pitch = blu->GetPitch(planes[p]);
        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

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
            for (y = 0; y < height; y += 1)
            {
                for (x = 0; x < row_size; x += 1)
                {
                    temp = srcp[x] - blup[x];
                    temp = abs(temp) < nt ? 0 : temp;
                    if (mode == 1)
                    {
                        temp = int(sqrt(abs(temp))) * ((temp > 0) - (temp < 0));
                    }
                    else if (mode == 2)
                    {
                        temp = int(atan(abs(temp)) * sqrt(abs(temp))) * ((temp > 0) - (temp < 0));
                    }
                    dstp[x] = max(0, min(255, srcp[x] + temp));
                    
                }
                dstp += dst_pitch;
                srcp += src_pitch;
                blup += blu_pitch;
            }
        } 
    }
    return dst;
}