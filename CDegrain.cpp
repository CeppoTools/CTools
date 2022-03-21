#include "CDegrain.h"

CDegrain::CDegrain(PClip _child, int _radius, int _nt, int _thr, int _blksize, PClip _dClip, IScriptEnvironment* env) : GenericVideoFilter(_child), radius(_radius), nt(_nt), thr(_thr), blksize(_blksize), dClip(_dClip)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CDegrain: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if ((vi.IsYV12() || vi.IsYV16()) && blksize < 2)
    {
        env->ThrowError("CDegrain: YV12/YV16 min. blksize is 2!");
    }
    else if (radius < 0 || radius > 7)
    {
        env->ThrowError("CDegrain: radius values must be in the [1, 7] range!");
    }
}

PVideoFrame __stdcall CDegrain::GetFrame(int n, IScriptEnvironment* env) {

    int i;
    PVideoFrame dst = env->NewVideoFrame(vi);

    PVideoFrame src[15];

    for (i = 0; i < radius; i += 1)
    {
        src[i] = child->GetFrame(max(n - radius + i, 0), env);
    }

    src[radius] = child->GetFrame(n, env);
    
    for (i = 0; i < radius; i += 1)
    {
        src[i + radius + 1] = child->GetFrame(min(n + i + 1, vi.num_frames - 1), env);
    }

    PVideoFrame den[15];

    if (!dClip)
    {
        vi.IsY8() ? CDegrain_Y8(src, src, dst, nt, thr, radius, blksize) : NULL;
        vi.IsYV12() ? CDegrain_YV12(src, src, dst, nt, thr, radius, blksize) : NULL;
        vi.IsYV16() ? CDegrain_YV16(src, src, dst, nt, thr, radius, blksize) : NULL;
        vi.IsYV24() ? CDegrain_YV24(src, src, dst, nt, thr, radius, blksize) : NULL;
    }
    else
    {
        for (i = 0; i < radius; i += 1)
        {
            den[i] = dClip->GetFrame(max(n - radius + i, 0), env);
        }

        den[radius] = dClip->GetFrame(n, env);

        for (i = 0; i < radius; i += 1)
        {
            den[i + radius + 1] = dClip->GetFrame(min(n + i + 1, vi.num_frames - 1), env);
        }

        vi.IsY8() ? CDegrain_Y8(src, den, dst, nt, thr, radius, blksize) : NULL;
        vi.IsYV12() ? CDegrain_YV12(src, den, dst, nt, thr, radius, blksize) : NULL;
        vi.IsYV16() ? CDegrain_YV16(src, den, dst, nt, thr, radius, blksize) : NULL;
        vi.IsYV24() ? CDegrain_YV24(src, den, dst, nt, thr, radius, blksize) : NULL;
    }

    return dst;
}

void CDegrain_Y8(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize)
{
    int i;
    unsigned char* dstp;
    dstp = dst->GetWritePtr(PLANAR_Y);

    const unsigned char* srcp[15];
    const unsigned char* denp[15];
    for (i = 0; i < 15; i += 1)
    {
        srcp[min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        denp[min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
    }

    int dst_pitch;
    dst_pitch = dst->GetPitch(PLANAR_Y);

    int src_pitch[15];
    int den_pitch[15];
    for (i = 0; i < 15; i += 1)
    {
        src_pitch[min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        den_pitch[min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_Y);
    }

    int height = dst->GetHeight(PLANAR_Y);
    int row_size = dst->GetRowSize(PLANAR_Y);

    int64_t BSY = 0;
    int64_t BSX = 0;
    BSY += min(1, height % blksize);
    BSY += height / blksize;
    BSX += min(1, row_size % blksize);
    BSX += row_size / blksize;
    auto SUM = new int[BSY * BSX][15];
    memset(SUM, 0, BSY * BSX * 15 * sizeof(int));

    int BX, BY;
    int pixel_sum[2];
    int x, y, a, b, temp;

    //DIFFERENCE
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                temp = abs(denp[i][x] - denp[radius][x]);
                SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
            }
        }
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            denp[i] += den_pitch[i];
        }
    }

    //BLENDING
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                pixel_sum[0] = 0;
                pixel_sum[1] = 0;
                for (i = 0; i < radius * 2 + 1; i += 1)
                {
                    pixel_sum[0] += SUM[BY * BSY + BX][i] <= thr ? 1 : 0;
                    pixel_sum[1] += SUM[BY * BSY + BX][i] <= thr ? srcp[i][x] : 0;
                }
                dstp[x] = int(pixel_sum[1] / static_cast<float>(pixel_sum[0]) + 0.5f);
            }
        }
        dstp += dst_pitch;
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            srcp[i] += src_pitch[i];
        }
    }
}

void CDegrain_YV12(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize)
{
    int i;
    unsigned char* dstp[3];
    dstp[0] = dst->GetWritePtr(PLANAR_Y);
    dstp[1] = dst->GetWritePtr(PLANAR_U);
    dstp[2] = dst->GetWritePtr(PLANAR_V);

    const unsigned char* srcp[3][15];
    const unsigned char* denp[3][15];
    for (i = 0; i < 15; i += 1)
    {
        srcp[0][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        denp[0][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        srcp[1][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_U);
        denp[1][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_U);
        srcp[2][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_V);
        denp[2][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_V);
    }

    int dst_pitch[3];
    dst_pitch[0] = dst->GetPitch(PLANAR_Y);
    dst_pitch[1] = dst->GetPitch(PLANAR_U);
    dst_pitch[2] = dst->GetPitch(PLANAR_V);

    int src_pitch[3][15];
    int den_pitch[3][15];
    for (i = 0; i < 15; i += 1)
    {
        src_pitch[0][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        den_pitch[0][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        src_pitch[1][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_U);
        den_pitch[1][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_U);
        src_pitch[2][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_V);
        den_pitch[2][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_V);
    }

    int height = dst->GetHeight(PLANAR_Y);
    int row_size = dst->GetRowSize(PLANAR_Y);

    int64_t BSY = 0;
    int64_t BSX = 0;
    BSY += min(1, height % blksize);
    BSY += height / blksize;
    BSX += min(1, row_size % blksize);
    BSX += row_size / blksize;
    auto SUM = new int[BSY * BSX][15];
    memset(SUM, 0, BSY * BSX * 15 * sizeof(int));

    int BX, BY;
    int pixel_sum[3];
    int x, y, a, b, temp;

    //DIFFERENCE
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                temp = abs(denp[0][i][x] - denp[0][radius][x]);
                SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
            }
            if (!(y % 2) && !(x % 2))
            {
                for (i = 0; i < radius * 2 + 1; i += 1)
                {
                    temp = abs(denp[1][i][x / 2] - denp[1][radius][x / 2]);
                    SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
                    temp = abs(denp[2][i][x / 2] - denp[2][radius][x / 2]);
                    SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
                }
            }
        }
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            denp[0][i] += den_pitch[0][i];
        }
        if (!(y % 2))
        {
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                denp[1][i] += den_pitch[1][i];
                denp[2][i] += den_pitch[2][i];
            }
        }
    }

    //BLENDING
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                pixel_sum[0] = 0;
                pixel_sum[1] = 0;
                for (i = 0; i < radius * 2 + 1; i += 1)
                {
                    pixel_sum[0] += SUM[BY * BSY + BX][i] <= thr ? 1 : 0;
                    pixel_sum[1] += SUM[BY * BSY + BX][i] <= thr ? srcp[0][i][x] : 0;
                }
                dstp[0][x] = int(pixel_sum[1] / static_cast<float>(pixel_sum[0]) + 0.5f);
                
                if (!(y % 2) && !(x % 2))
                {
                    pixel_sum[0] = 0;
                    pixel_sum[1] = 0;
                    pixel_sum[2] = 0;
                    for (i = 0; i < radius * 2 + 1; i += 1)
                    {
                        pixel_sum[0] += SUM[BY * BSY + BX][i] <= thr ? 1 : 0;
                        pixel_sum[1] += SUM[BY * BSY + BX][i] <= thr ? srcp[1][i][x / 2] : 0;
                        pixel_sum[2] += SUM[BY * BSY + BX][i] <= thr ? srcp[2][i][x / 2] : 0;
                    }
                    dstp[1][x / 2] = int(pixel_sum[1] / static_cast<float>(pixel_sum[0]) + 0.5f);
                    dstp[2][x / 2] = int(pixel_sum[2] / static_cast<float>(pixel_sum[0]) + 0.5f);
                }  
            }
        }
        dstp[0] += dst_pitch[0];
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            srcp[0][i] += src_pitch[0][i];
        }
        if (!(y % 2))
        {
            dstp[1] += dst_pitch[1];
            dstp[2] += dst_pitch[2];
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                srcp[1][i] += src_pitch[1][i];
                srcp[2][i] += src_pitch[2][i];
            }
        }
    }
}

void CDegrain_YV16(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize)
{
    int i;
    unsigned char* dstp[3];
    dstp[0] = dst->GetWritePtr(PLANAR_Y);
    dstp[1] = dst->GetWritePtr(PLANAR_U);
    dstp[2] = dst->GetWritePtr(PLANAR_V);

    const unsigned char* srcp[3][15];
    const unsigned char* denp[3][15];
    for (i = 0; i < 15; i += 1)
    {
        srcp[0][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        denp[0][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        srcp[1][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_U);
        denp[1][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_U);
        srcp[2][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_V);
        denp[2][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_V);
    }

    int dst_pitch[3];
    dst_pitch[0] = dst->GetPitch(PLANAR_Y);
    dst_pitch[1] = dst->GetPitch(PLANAR_U);
    dst_pitch[2] = dst->GetPitch(PLANAR_V);

    int src_pitch[3][15];
    int den_pitch[3][15];
    for (i = 0; i < 15; i += 1)
    {
        src_pitch[0][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        den_pitch[0][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        src_pitch[1][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_U);
        den_pitch[1][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_U);
        src_pitch[2][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_V);
        den_pitch[2][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_V);
    }

    int height = dst->GetHeight(PLANAR_Y);
    int row_size = dst->GetRowSize(PLANAR_Y);

    int64_t BSY = 0;
    int64_t BSX = 0;
    BSY += min(1, height % blksize);
    BSY += height / blksize;
    BSX += min(1, row_size % blksize);
    BSX += row_size / blksize;
    auto SUM = new int[BSY * BSX][15];
    memset(SUM, 0, BSY * BSX * 15 * sizeof(int));

    int BX, BY;
    int pixel_sum[3];
    int x, y, a, b, temp;

    //DIFFERENCE
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                temp = abs(denp[0][i][x] - denp[0][radius][x]);
                SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
            }
            if (!(x % 2))
            {
                for (i = 0; i < radius * 2 + 1; i += 1)
                {
                    temp = abs(denp[1][i][x / 2] - denp[1][radius][x / 2]);
                    SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
                    temp = abs(denp[2][i][x / 2] - denp[2][radius][x / 2]);
                    SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
                }
            }
        }
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            denp[0][i] += den_pitch[0][i];
            denp[1][i] += den_pitch[1][i];
            denp[2][i] += den_pitch[2][i];
        }
    }

    //BLENDING
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                pixel_sum[0] = 0;
                pixel_sum[1] = 0;
                for (i = 0; i < radius * 2 + 1; i += 1)
                {
                    pixel_sum[0] += SUM[BY * BSY + BX][i] <= thr ? 1 : 0;
                    pixel_sum[1] += SUM[BY * BSY + BX][i] <= thr ? srcp[0][i][x] : 0;
                }
                dstp[0][x] = int(pixel_sum[1] / static_cast<float>(pixel_sum[0]) + 0.5f);

                if (!(x % 2))
                {
                    pixel_sum[0] = 0;
                    pixel_sum[1] = 0;
                    pixel_sum[2] = 0;
                    for (i = 0; i < radius * 2 + 1; i += 1)
                    {
                        pixel_sum[0] += SUM[BY * BSY + BX][i] <= thr ? 1 : 0;
                        pixel_sum[1] += SUM[BY * BSY + BX][i] <= thr ? srcp[1][i][x / 2] : 0;
                        pixel_sum[2] += SUM[BY * BSY + BX][i] <= thr ? srcp[2][i][x / 2] : 0;
                    }
                    dstp[1][x / 2] = int(pixel_sum[1] / static_cast<float>(pixel_sum[0]) + 0.5f);
                    dstp[2][x / 2] = int(pixel_sum[2] / static_cast<float>(pixel_sum[0]) + 0.5f);
                }
            }
        }
        dstp[0] += dst_pitch[0];
        dstp[1] += dst_pitch[1];
        dstp[2] += dst_pitch[2];
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            srcp[0][i] += src_pitch[0][i];
            srcp[1][i] += src_pitch[1][i];
            srcp[2][i] += src_pitch[2][i];
        }
    }
}

void CDegrain_YV24(PVideoFrame(&src)[15], PVideoFrame(&den)[15], PVideoFrame& dst, int nt, int thr, int radius, int blksize)
{
    int i;
    unsigned char* dstp[3];
    dstp[0] = dst->GetWritePtr(PLANAR_Y);
    dstp[1] = dst->GetWritePtr(PLANAR_U);
    dstp[2] = dst->GetWritePtr(PLANAR_V);

    const unsigned char* srcp[3][15];
    const unsigned char* denp[3][15];
    for (i = 0; i < 15; i += 1)
    {
        srcp[0][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        denp[0][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_Y);
        srcp[1][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_U);
        denp[1][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_U);
        srcp[2][min(i, radius * 2)] = src[min(i, radius * 2)]->GetReadPtr(PLANAR_V);
        denp[2][min(i, radius * 2)] = den[min(i, radius * 2)]->GetReadPtr(PLANAR_V);
    }

    int dst_pitch[3];
    dst_pitch[0] = dst->GetPitch(PLANAR_Y);
    dst_pitch[1] = dst->GetPitch(PLANAR_U);
    dst_pitch[2] = dst->GetPitch(PLANAR_V);

    int src_pitch[3][15];
    int den_pitch[3][15];
    for (i = 0; i < 15; i += 1)
    {
        src_pitch[0][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        den_pitch[0][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_Y);
        src_pitch[1][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_U);
        den_pitch[1][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_U);
        src_pitch[2][min(i, radius * 2)] = src[min(i, radius * 2)]->GetPitch(PLANAR_V);
        den_pitch[2][min(i, radius * 2)] = den[min(i, radius * 2)]->GetPitch(PLANAR_V);
    }

    int height = dst->GetHeight(PLANAR_Y);
    int row_size = dst->GetRowSize(PLANAR_Y);

    int64_t BSY = 0;
    int64_t BSX = 0;
    BSY += min(1, height % blksize);
    BSY += height / blksize;
    BSX += min(1, row_size % blksize);
    BSX += row_size / blksize;
    auto SUM = new int[BSY * BSX][15];
    memset(SUM, 0, BSY * BSX * 15 * sizeof(int));

    int BX, BY;
    int pixel_sum[4];
    int x, y, a, b, temp;

    //DIFFERENCE
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                temp = abs(denp[0][i][x] - denp[0][radius][x]);
                SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
                temp = abs(denp[1][i][x] - denp[1][radius][x]);
                SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
                temp = abs(denp[2][i][x] - denp[2][radius][x]);
                SUM[BY * BSY + BX][i] += temp > nt ? 255 : temp;
            }
        }
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            denp[0][i] += den_pitch[0][i];
            denp[1][i] += den_pitch[1][i];
            denp[2][i] += den_pitch[2][i];
        }
    }

    //BLENDING
    for (y = 0; y < height; y += 1)
    {
        BY = y / blksize;
        BY += y % blksize ? 1 : 0;
        for (x = 0; x < row_size; x += 1)
        {
            BX = x / blksize;
            BX += x % blksize ? 1 : 0;
            for (i = 0; i < radius * 2 + 1; i += 1)
            {
                pixel_sum[0] = 0;
                pixel_sum[1] = 0;
                pixel_sum[2] = 0;
                pixel_sum[3] = 0;
                for (i = 0; i < radius * 2 + 1; i += 1)
                {
                    pixel_sum[0] += SUM[BY * BSY + BX][i] <= thr ? 1 : 0;
                    pixel_sum[1] += SUM[BY * BSY + BX][i] <= thr ? srcp[0][i][x] : 0;
                    pixel_sum[2] += SUM[BY * BSY + BX][i] <= thr ? srcp[1][i][x] : 0;
                    pixel_sum[3] += SUM[BY * BSY + BX][i] <= thr ? srcp[2][i][x] : 0;
                }
                dstp[0][x] = int(pixel_sum[1] / static_cast<float>(pixel_sum[0]) + 0.5f);
                dstp[1][x] = int(pixel_sum[2] / static_cast<float>(pixel_sum[0]) + 0.5f);
                dstp[2][x] = int(pixel_sum[3] / static_cast<float>(pixel_sum[0]) + 0.5f);
            }
        }
        dstp[0] += dst_pitch[0];
        dstp[1] += dst_pitch[1];
        dstp[2] += dst_pitch[2];
        for (i = 0; i < radius * 2 + 1; i += 1)
        {
            srcp[0][i] += src_pitch[0][i];
            srcp[1][i] += src_pitch[1][i];
            srcp[2][i] += src_pitch[2][i];
        }
    }
}