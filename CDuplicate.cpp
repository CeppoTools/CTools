#include "CDuplicate.h"

CDuplicate::CDuplicate(PClip _child, float _thr, float _thr2, int _nt, bool _fields, PClip _dClip, bool _write, IScriptEnvironment* env) : GenericVideoFilter(_child), thr(_thr), thr2(_thr), nt(_nt), fields(_fields), dClip(_dClip), write(_write)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CDuplicate: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (thr < 0 || thr > 255)
    {
        env->ThrowError("CDuplicate: thr value must be in the [0, 255] range!");
    }
    else if (thr2 < 0 || thr2 > 255)
    {
        env->ThrowError("CDuplicate: thr2 value must be in the [0, 255] range!");
    }
    else if (nt < 0 || nt > 255)
    {
        env->ThrowError("CDuplicate: nt value must be in the [0, 255] range!");
    }
    if (write && !fields)
    {
        int i;
        for (i = 0; i < vi.num_frames; i += 1)
        {
            isOut[i] = 0;
        }
        file.open("CDuplicate.txt", ios::trunc);
        file2.open("CDuplicate2.txt", ios::trunc);
    }
    else if (write && fields)
    {
        int i;
        for (i = 0; i < vi.num_frames; i += 1)
        {
            isOut[i] = 0;
        }
        file.open("CDuplicateFields.txt", ios::trunc);
        file2.open("CDuplicateFields2.txt", ios::trunc);
    }

    
}

PVideoFrame __stdcall CDuplicate::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame src = dClip ? dClip->GetFrame(n, env) : child->GetFrame(n, env);
    PVideoFrame prv = dClip ? dClip->GetFrame(max(0, n - 1), env) : child->GetFrame(max(0, n - 1), env);
    
    if (!fields)
    {
        array<bool, 2> isDup;

        if (n == 0)
        {
            isDup[0] = 0;
            isDup[1] = 0;
        }
        else
        {
            isDup = CDuplicate_isDup(src, prv, nt, thr, thr2, vi.IsY8());
        }
            
        if (write && !isOut[n])
        {
            file  << isDup[0] << " " << n << "\n";
            file2 << isDup[1] << " " << n << "\n";
            n == vi.num_frames - 1 ? file.close(), file2.close() : NULL;
        }

        if (!write)
        {
            string temp[2];
            temp[0] = isDup[0] ? "Duplicate" : "Not Duplicate";
            temp[1] = isDup[1] ? "Duplicate" : "Not Duplicate";
            string text1 = "thr: " + temp[0];
            string text2 = "thr2: " + temp[1];
            AVSValue args[4];
            args[0] = child;
            args[1] = text1.c_str();
            args[2] = 8;
            args[3] = 0;
            PClip sub = env->Invoke("subtitle", AVSValue(args, 4)).AsClip();
            args[0] = sub;
            args[1] = text2.c_str();
            args[2] = 8;
            args[3] = 18;
            PClip sub2 = env->Invoke("subtitle", AVSValue(args, 4)).AsClip();
            PVideoFrame subFrame = sub2->GetFrame(n, env);
            return subFrame;
        }
        else
        {
            src = dClip ? child->GetFrame(n, env) : src;
            return src;
        }
    }
    else
    {
        array<bool, 4> isDup; 

        if (n == 0)
        {
            isDup[0] = 0;
            isDup[1] = 0;
            isDup[2] = 0;
            isDup[3] = 0;
        }
        else
        {
            isDup = CDuplicate_isDupFields(src, prv, nt, thr, thr2, vi.IsY8());
        }

        if (write && !isOut[n])
        {
            file  << isDup[0] << " " << isDup[2] << " " << n << "\n";
            file2 << isDup[1] << " " << isDup[3] << " " << n << "\n";
            n == vi.num_frames - 1 ? file.close(), file2.close() : NULL;
        }

        if (!write)
        {
            string temp[4];
            temp[0] = isDup[0] ? "Duplicate" : "Not Duplicate";
            temp[1] = isDup[1] ? "Duplicate" : "Not Duplicate";
            temp[2] = isDup[2] ? "Duplicate" : "Not Duplicate";
            temp[3] = isDup[3] ? "Duplicate" : "Not Duplicate";
            string text1 = "FirstField(thr): " + temp[0];
            string text2 = "SecondField(thr): " + temp[2];
            string text3 = "FirstField(thr2): " + temp[1];
            string text4 = "SecondField(thr2): " + temp[3];
            AVSValue args[4];
            args[0] = child;
            args[1] = text1.c_str();
            args[2] = 8;
            args[3] = 0;
            PClip sub = env->Invoke("subtitle", AVSValue(args, 4)).AsClip();
            args[0] = sub;
            args[1] = text2.c_str();
            args[2] = 8;
            args[3] = 18;
            PClip sub2 = env->Invoke("subtitle", AVSValue(args, 4)).AsClip();
            args[0] = sub2;
            args[1] = text3.c_str();
            args[2] = 8;
            args[3] = 36;
            PClip sub3 = env->Invoke("subtitle", AVSValue(args, 4)).AsClip();
            args[0] = sub3;
            args[1] = text4.c_str();
            args[2] = 8;
            args[3] = 54;
            PClip sub4 = env->Invoke("subtitle", AVSValue(args, 4)).AsClip();
            PVideoFrame subFrame = sub4->GetFrame(n, env);
            return subFrame;
        }
        else
        {
            src = dClip ? child->GetFrame(n, env) : src;
            return src;
        }
    }   
}

array<bool, 2> CDuplicate_isDup(PVideoFrame& src, PVideoFrame& prv, int nt, float thr, float thr2, bool isY8)
{
    const unsigned char* srcp;
    const unsigned char* prvp;

    int height;
    int row_size;
    int src_pitch;
    int prv_pitch;
    int p, x, y;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    float AVG[2][3] = { { 0, 0, 0 }, { 0, 0, 0 } };
    int64_t SUM[2][3] = { { 0, 0, 0 }, { 0, 0, 0 } };

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
        
        for (y = 0; y < height; y += 1)
        {
            for (x = 0; x < row_size; x += 1)
            {
                SUM[0][p] += srcp[x];
                SUM[1][p] += abs(srcp[x] - prvp[x]) < nt ? srcp[x] : 255;
            }
            srcp += src_pitch;
            prvp += prv_pitch;
        }
        AVG[0][p] = SUM[0][p] / static_cast<float>(height) / static_cast<float>(row_size);
        AVG[1][p] = SUM[1][p] / static_cast<float>(height) / static_cast<float>(row_size);
    }

    array<bool, 2> isDup;
    isDup[0] = isY8 ? AVG[1][0] < AVG[0][0] + thr  : AVG[1][0] + AVG[1][1] + AVG[1][2] < AVG[0][0] + AVG[0][1] + AVG[0][2] + thr;
    isDup[1] = isY8 ? AVG[1][0] < AVG[0][0] + thr2 : AVG[1][0] + AVG[1][1] + AVG[1][2] < AVG[0][0] + AVG[0][1] + AVG[0][2] + thr2;
    return  isDup;
}

array<bool, 4> CDuplicate_isDupFields(PVideoFrame& src, PVideoFrame& prv, int nt, float thr, float thr2, bool isY8)
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
    isDup[1] = AVG[1][0] + AVG[1][1] + AVG[1][2] < AVG[0][0] + AVG[0][1] + AVG[0][2] + thr2;
    isDup[2] = AVG[3][0] + AVG[3][1] + AVG[3][2] < AVG[2][0] + AVG[2][1] + AVG[2][2] + thr;
    isDup[3] = AVG[3][0] + AVG[3][1] + AVG[3][2] < AVG[2][0] + AVG[2][1] + AVG[2][2] + thr2;
    return isDup;
}
