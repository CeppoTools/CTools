#include "CTelecine.h"

CTelecine::CTelecine(PClip _child, bool _bob, int _mode, int _nt, int _ntN, bool _sse, float _thr60i, int _nt60i, float _sstr, float _mode2thr, PClip _dClip, bool _write, bool _inputTxt, IScriptEnvironment* env)
    : GenericVideoFilter(_child), bob(_bob), mode(_mode), nt(_nt), ntN(_ntN), sse(_sse), thr60i(_thr60i), nt60i(_nt60i), sstr(_sstr), mode2thr(_mode2thr), dClip(_dClip), write(_write), inputTxt(_inputTxt)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CTelecine: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (nt < 0 || nt > 255)
    {
        env->ThrowError("CTelecine: nt value must be in the [0, 255] range!");
    }
    else if (ntN < 0 || ntN > 255)
    {
        env->ThrowError("CTelecine: ntN value must be in the [0, 255] range!");
    }
    else if (mode < 0 || mode > 6)
    {
        env->ThrowError("CTelecine: mode value must be in the [0, 6] range!");
    }
    else if (write && inputTxt)
    {
        env->ThrowError("CTelecine: can't write and read file at the same time!");
    }

    C = child;
    N = env->Invoke("DoubleWeave", AVSValue({ C }, 1)).AsClip();
    dClipN = dClip ? env->Invoke("DoubleWeave", AVSValue({ dClip }, 1)).AsClip() : NULL;

    if (mode == 0 || mode == 5 || mode != 1 && mode != 6 && bob)
    {
        try
        {
            AVSValue args[2];
            args[0] = C;
            args[1] = sstr;
            CV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
            args[0] = N;
            NV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CTelecine: vinverse(.dll) function not found!");
        }
    }
    else if (mode == 1 || mode == 6)
    {
        try
        {
            AVSValue args[2];
            args[0] = C;
            args[1] = sstr;
            CV = env->Invoke("ex_vinverse", AVSValue(args, 2)).AsClip();
            args[0] = N;
            NV = env->Invoke("ex_vinverse", AVSValue(args, 2)).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CTelecine: ex_vinverse(.avsi) function not found!");
        }
    }

    
    if (dClip && (mode == 0 || mode == 5))
    {
        AVSValue args[2];
        args[0] = dClip;
        args[1] = sstr;
        dClipV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
        args[0] = dClipN;
        dClipNV = env->Invoke("Vinverse", AVSValue(args, 2)).AsClip();
    }
    else if (dClip && (mode == 1 || mode == 6))
    {
        AVSValue args[2];
        args[0] = dClip;
        args[1] = sstr;
        dClipV = env->Invoke("ex_vinverse", AVSValue(args, 2)).AsClip();
        args[0] = dClipN;
        dClipNV = env->Invoke("ex_vinverse", AVSValue(args, 2)).AsClip();
    }
    
    if (write)
    {
        int i;
        for (i = 0; i < vi.num_frames; i += 1)
        {
            isOut[i] = 0;
            isOut2[i] = 0;
        }
        file.open("CTelecine.txt", ios::trunc);
        bob ? file2.open("CTelecineBob.txt", ios::trunc) : NULL;
    }

    if (inputTxt)
    {
        ifstream ifile;
        ifile.open("CTelecine.txt");
        if (!ifile)
        {
            env->ThrowError("CTelecine: CTelecine.txt not found!");
        }

        int i = 0;
        string line;
        while (getline(ifile, line) && i < vi.num_frames)
        {
            vec[i] = (int)line[0] - 48;
            i += 1;
        }
        ifile.close();
    }

    if (inputTxt && bob)
    {
        ifstream ifile;
        ifile.open("CTelecineBob.txt");
        if (!ifile)
        {
            env->ThrowError("CTelecine: CTelecineBob.txt not found!");
        }

        int i = 0;
        string line;
        while (getline(ifile, line) && i < vi.num_frames)
        {
            vec2[i] = (int)line[0] - 48;
            i += 1;
        }
        ifile.close();
    }

    if (bob)
    {
        vi.num_frames *= 2;
        vi.SetFPS(vi.fps_numerator * 2, vi.fps_denominator);
    }
}

PVideoFrame __stdcall CTelecine::GetFrame(int f, IScriptEnvironment* env) {

    if (inputTxt && bob)
    {
        int w = f / 2;
        PVideoFrame out = f & 1 && vec[w] == 1 ? N->GetFrame(f, env)
                        : vec2[w] == 1 ? N->GetFrame(max(f - 1, 0), env)
                        : C->GetFrame(w, env);
        return out;
    }
    else if (inputTxt && !bob)
    {
        PVideoFrame out = vec[f] == 1 ? N->GetFrame(f * 2 + 1, env)
                        : C->GetFrame(f, env);
        return out;
    }
    else if (bob && f & 1)
    {
        bool isN = 0;
        int w = f / 2;
        int m = w * 2 + 1;
        PVideoFrame c = dClip ? dClip->GetFrame(w, env) : C->GetFrame(w, env);
        PVideoFrame n = dClip ? dClipN->GetFrame(m, env) : N->GetFrame(m, env);
        PVideoFrame cv = dClip ? dClipV->GetFrame(w, env) : CV->GetFrame(w, env);
        if (mode == 0 || mode == 1 || mode == 5 || mode == 6)
        {
            PVideoFrame nv = dClip ? dClipNV->GetFrame(m, env) : NV->GetFrame(m, env);
            isN = mode < 2 ? CTelecine_mode01(c, cv, n, nv, nt, vi.IsY8())
                           : CTelecine_mode56(c, cv, n, nv, nt, ntN, sse, vi.IsY8());
        }
        else if (mode == 2)
        {
            PVideoFrame p_ = dClip ? dClip->GetFrame(max(w - 1, 0), env) : C->GetFrame(max(w - 1, 0), env);
            PVideoFrame n_ = dClip ? dClip->GetFrame(min(w + 1, vi.num_frames / 2 - 1), env) : C->GetFrame(min(w + 1, vi.num_frames / 2 - 1), env);
            isN = CTelecine_mode2(c, p_, nt, mode2thr, vi.IsY8()) == 1 ? 1
                : CTelecine_mode2(c, n_, nt, mode2thr, vi.IsY8()) == 2 ? 1 : 0;
        }
        else if (mode == 3 || mode == 4)
        {
            int order = child->GetParity(0) ? 1 : 0;
            isN = mode == 3 ? CTelecine_mode3(c, n, nt, ntN, sse, order, vi.IsY8())
                            : CTelecine_mode4(c, n, nt, ntN, sse, order, vi.IsY8());
        }
        if (write && isN && !isOut[w])
        {
            isOut[w] = 1;
            file << isN << " " << w << '\n';
            w == vi.num_frames / 2 - 1 ? file.close() : NULL;
        }
        else if (write && !isN && !isOut[w])
        {
            isOut[w] = 1;
            int is60i = CTelecine_is60i(c, cv, nt60i, thr60i, vi.IsY8()) ? 2 : 0;
            file << is60i << " " << w << '\n';
            w == vi.num_frames / 2 - 1 ? file.close() : NULL;
        }
        c = dClip ? C->GetFrame(w, env) : c;
        n = dClip ? N->GetFrame(m, env) : n;
        return isN ? n : c;
    }
    else if (bob)
    {
        bool isP = 0;
        int w = f / 2;
        int m = w * 2 + 1;
        PVideoFrame c = dClip ? dClip->GetFrame(w, env) : C->GetFrame(w, env);
        PVideoFrame p = dClip ? dClipN->GetFrame(max(m - 2, 0), env) : N->GetFrame(max(m - 2, 0), env);
        if (mode == 0 || mode == 1 || mode == 5 || mode == 6)
        {
            PVideoFrame cv = dClip ? dClipV->GetFrame(w, env) : CV->GetFrame(w, env);
            PVideoFrame pv = dClip ? dClipNV->GetFrame(max(m - 2, 0), env) : NV->GetFrame(max(m - 2, 0), env);
            isP = mode < 2 ? CTelecine_mode01(c, cv, p, pv, nt, vi.IsY8())
                           : CTelecine_mode56(c, cv, p, pv, nt, ntN, sse, vi.IsY8());
        }
        else if (mode == 2)
        {
            PVideoFrame p_ = dClip ? dClip->GetFrame(max(w - 1, 0), env) : C->GetFrame(max(w - 1, 0), env);
            PVideoFrame n_ = dClip ? dClip->GetFrame(min(w + 1, vi.num_frames / 2 - 1), env) : C->GetFrame(min(w + 1, vi.num_frames / 2 - 1), env);
            isP = CTelecine_mode2(c, p_, nt, mode2thr, vi.IsY8()) == 1 ? 1
                : CTelecine_mode2(c, n_, nt, mode2thr, vi.IsY8()) == 2 ? 1 : 0;
        }
        else if (mode == 3 || mode == 4)
        {
            int order = child->GetParity(0) ? 1 : 0;
            isP = mode == 3 ? CTelecine_mode3(c, p, nt, ntN, sse, order, vi.IsY8())
                            : CTelecine_mode4(c, p, nt, ntN, sse, order, vi.IsY8());
        }
        if (write && !isOut2[w])
        {
            isOut2[w] = 1;
            file2 << isP << " " << w << '\n';
            w == vi.num_frames / 2 ? file.close() : NULL;
        }
        c = dClip ? C->GetFrame(w, env) : c;
        p = dClip ? N->GetFrame(max(m - 2, 0), env) : p;
        return isP ? p : c;
    }
    else
    {
        bool isN = 0;
        PVideoFrame c = dClip ? dClip->GetFrame(f, env) : C->GetFrame(f, env);
        PVideoFrame n = dClip ? dClipN->GetFrame(f * 2 + 1, env) : N->GetFrame(f * 2 + 1, env);
        if (mode == 0 || mode == 1 || mode == 5 || mode == 6)
        {
            PVideoFrame cv = dClip ? dClipV->GetFrame(f, env) : CV->GetFrame(f, env);
            PVideoFrame nv = dClip ? dClipNV->GetFrame(f * 2 + 1, env) : NV->GetFrame(f * 2 + 1, env);
            isN = mode < 2 ? CTelecine_mode01(c, cv, n, nv, nt, vi.IsY8())
                           : CTelecine_mode56(c, cv, n, nv, nt, ntN, sse, vi.IsY8());
        }
        else if (mode == 2)
        {
            PVideoFrame p_ = dClip ? dClip->GetFrame(max(f - 1, 0), env) : C->GetFrame(max(f - 1, 0), env);
            PVideoFrame n_ = dClip ? dClip->GetFrame(min(f + 1, vi.num_frames - 1), env) : C->GetFrame(min(f + 1, vi.num_frames - 1), env);
            isN = CTelecine_mode2(c, p_, nt, mode2thr, vi.IsY8()) == 1 ? 1 
                : CTelecine_mode2(c, n_, nt, mode2thr, vi.IsY8()) == 2 ? 1 : 0;
        }
        else if (mode == 3 || mode == 4)
        {
            int order = child->GetParity(0) ? 1 : 0;
            isN = mode == 3 ? CTelecine_mode3(c, n, nt, ntN, sse, order, vi.IsY8())
                            : CTelecine_mode4(c, n, nt, ntN, sse, order, vi.IsY8());
        }
        if (write && !isOut[f])
        {
            isOut[f] = 1;
            file << isN << " " << f << '\n';
            f == vi.num_frames - 1 ? file.close() : NULL;
        }
        c = dClip ? C->GetFrame(f, env) : c;
        n = dClip ? N->GetFrame(f * 2 + 1, env) : n;
        return isN ? n : c;
    }
}

bool CTelecine_is60i(PVideoFrame& c, PVideoFrame& cv, int nt, float thr, bool isY8)
{
    const unsigned char* cp;
    const unsigned char* cvp;

    int p;
    int x, y;
    int height;
    int row_size;
    int c_pitch, cv_pitch;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };
    float avg[3];
    int64_t sum[3] = { 0, 0, 0 };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        cp = c->GetReadPtr(planes[p]);
        cvp = cv->GetReadPtr(planes[p]);

        c_pitch = c->GetPitch(planes[p]);
        cv_pitch = cv->GetPitch(planes[p]);

        height = c->GetHeight(planes[p]);
        row_size = c->GetRowSize(planes[p]);

        for (y = 0; y < height; y += 1)
        {
            for (x = 0; x < row_size; x += 1)
            {
                sum[p] += abs(cp[x] - cvp[x]) > nt ? 255 : 0;
            }
            cp += c_pitch;
            cvp += cv_pitch;
        }
        avg[p] = sum[p] / static_cast<float>(height) / static_cast<float>(row_size);
    }
    return isY8 ? avg[0] > thr
                : (avg[0] + avg[1] + avg[2]) / 3.0f > thr;
}

bool CTelecine_mode01(PVideoFrame& c, PVideoFrame& cv, PVideoFrame& n, PVideoFrame& nv, int nt, bool isY8)
{
    const unsigned char* cp;
    const unsigned char* cvp;
    const unsigned char* np;
    const unsigned char* nvp;

    int p;
    int x, y;
    int height;
    int row_size;
    int c_pitch, cv_pitch;
    int n_pitch, nv_pitch;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };
    float avg[3][2];
    int64_t sum[3][2] = { {0,0}, {0,0}, {0,0} };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        cp = c->GetReadPtr(planes[p]);
        cvp = cv->GetReadPtr(planes[p]);
        np = n->GetReadPtr(planes[p]);
        nvp = nv->GetReadPtr(planes[p]);

        c_pitch = c->GetPitch(planes[p]);
        cv_pitch = cv->GetPitch(planes[p]);
        n_pitch = n->GetPitch(planes[p]);
        nv_pitch = nv->GetPitch(planes[p]);

        height = c->GetHeight(planes[p]);
        row_size = c->GetRowSize(planes[p]);

        for (y = 0; y < height; y += 1)
        {
            for (x = 0; x < row_size; x += 1)
            {
                sum[p][0] += abs(cp[x] - cvp[x]) > nt ? 255 : 0;
                sum[p][1] += abs(np[x] - nvp[x]) > nt ? 255 : 0;
            }
            cp += c_pitch;
            cvp += cv_pitch;
            np += c_pitch;
            nvp += cv_pitch;
        }
        avg[p][0] = sum[p][0] / static_cast<float>(height) / static_cast<float>(row_size);
        avg[p][1] = sum[p][1] / static_cast<float>(height) / static_cast<float>(row_size);
    }
    return isY8 ? avg[0][1] < avg[0][0] 
                : (avg[0][1] + avg[1][1] + avg[2][1]) / 3.0f < (avg[0][0] + avg[1][0] + avg[2][0]) / 3.0f;
}

bool CTelecine_mode56(PVideoFrame& c, PVideoFrame& cv, PVideoFrame& n, PVideoFrame& nv, int nt, int ntN, bool sse, bool isY8)
{
    const unsigned char* cp;
    const unsigned char* cvp;
    const unsigned char* np;
    const unsigned char* nvp;

    int p;
    int x, y;
    int temp[2];
    int height;
    int row_size;
    int c_pitch, cv_pitch;
    int n_pitch, nv_pitch;
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };
    int64_t sum[3][2] = { {0,0}, {0,0}, {0,0} };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        cp = c->GetReadPtr(planes[p]);
        cvp = cv->GetReadPtr(planes[p]);
        np = n->GetReadPtr(planes[p]);
        nvp = nv->GetReadPtr(planes[p]);

        c_pitch = c->GetPitch(planes[p]);
        cv_pitch = cv->GetPitch(planes[p]);
        n_pitch = n->GetPitch(planes[p]);
        nv_pitch = nv->GetPitch(planes[p]);

        height = c->GetHeight(planes[p]);
        row_size = c->GetRowSize(planes[p]);

        for (y = 0; y < height; y += 1)
        {
            for (x = 0; x < row_size; x += 1)
            {
                temp[0] = abs(cp[x] - cvp[x]);
                temp[1] = abs(np[x] - nvp[x]);
                temp[0] = temp[0] < nt ? 0 : temp[0];
                temp[1] = temp[1] < ntN ? 0 : temp[1];
                sum[p][0] += sse ? temp[0] * temp[0] : temp[0];
                sum[p][1] += sse ? temp[1] * temp[1] : temp[1];
            }
            cp += c_pitch;
            cvp += cv_pitch;
            np += c_pitch;
            nvp += cv_pitch;
        }
    }
    return isY8 ? sum[0][1] < sum[0][0]
                : sum[0][1] + sum[1][1] + sum[2][1] < sum[0][0] + sum[1][0] + sum[2][0];
}

int CTelecine_mode2(PVideoFrame& src, PVideoFrame& nxt, int nt, float thr, bool isY8)
{
    const unsigned char* srcp;
    const unsigned char* nxtp;

    int height;
    int row_size;
    int src_pitch;
    int nxt_pitch;

    int p, x, y;
    bool isDup[6];
    float avg[3][4];
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };
    int64_t sum[3][4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcp = src->GetReadPtr(planes[p]);
        nxtp = nxt->GetReadPtr(planes[p]);

        src_pitch = src->GetPitch(planes[p]);
        nxt_pitch = nxt->GetPitch(planes[p]);

        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        for (y = 0; y < height; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                sum[p][0] += srcp[x];
                sum[p][1] += abs(srcp[x] - nxtp[x]) < nt ? srcp[x] : 255;
            }
            srcp += src_pitch;
            nxtp += nxt_pitch;
            for (x = 0; x < row_size; x += 1)
            {
                sum[p][2] += srcp[x];
                sum[p][3] += abs(srcp[x] - nxtp[x]) < nt ? srcp[x] : 255;
            }
            srcp += src_pitch;
            nxtp += nxt_pitch;
        }
        avg[p][0] = sum[p][0] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
        avg[p][1] = sum[p][1] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
        avg[p][2] = sum[p][2] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
        avg[p][3] = sum[p][3] / static_cast<float>(height) / static_cast<float>(row_size) / 2.0f;
    }
    isDup[0] = isY8 ? avg[0][1] <= avg[0][0] + thr
             : (avg[0][1] + avg[1][1] + avg[2][1]) / 3.0f <= (avg[0][0] + avg[1][0] + avg[2][0]) / 3.0f + thr;
    
    isDup[1] = isY8 ? avg[0][3] <= avg[0][2] + thr
             : (avg[0][3] + avg[1][3] + avg[2][3]) / 3.0f <= (avg[0][2] + avg[1][2] + avg[2][2]) / 3.0f + thr;
    
    isDup[2] = isY8 ? avg[0][1] <= avg[0][0] + thr * 10
             : (avg[0][1] + avg[1][1] + avg[2][1]) / 3.0f <= (avg[0][0] + avg[1][0] + avg[2][0]) / 3.0f + thr * 10;
    
    isDup[3] = isY8 ? avg[0][3] <= avg[0][2] + thr * 10
             : (avg[0][3] + avg[1][3] + avg[2][3]) / 3.0f <= (avg[0][2] + avg[1][2] + avg[2][2]) / 3.0f + thr * 10;
    
    isDup[4] = isY8 ? avg[0][1] <= avg[0][0] + thr * 100
             : (avg[0][1] + avg[1][1] + avg[2][1]) / 3.0f <= (avg[0][0] + avg[1][0] + avg[2][0]) / 3.0f + thr * 100;
    
    isDup[5] = isY8 ? avg[0][3] <= avg[0][2] + thr * 100
             : (avg[0][3] + avg[1][3] + avg[2][3]) / 3.0f <= (avg[0][2] + avg[1][2] + avg[2][2]) / 3.0f + thr * 100;
    
    return ( isDup[0] && !isDup[1] ||  isDup[2] && !isDup[3] ||  isDup[4] && !isDup[5]) ? 1 
         : (!isDup[0] &&  isDup[1] || !isDup[2] &&  isDup[3] || !isDup[4] &&  isDup[5]) ? 2 : 0;
}

bool CTelecine_mode3(PVideoFrame& src, PVideoFrame& nxt, int nt, int ntN, bool sse, int order, bool isY8)
{
    const unsigned char* srcA;
    const unsigned char* srcB;
    const unsigned char* srcC;
    const unsigned char* srcD;
    const unsigned char* srcE;

    const unsigned char* nxtA;
    const unsigned char* nxtB;
    const unsigned char* nxtC;
    const unsigned char* nxtD;
    const unsigned char* nxtE;

    int height;
    int row_size;
    int src_pitch;
    int nxt_pitch;
    int x, y, p, temp;
    int64_t diff[2] = { 0, 0 };
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcC = src->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        nxtC = nxt->GetReadPtr(planes[p]);
        nxt_pitch = nxt->GetPitch(planes[p]);

        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        srcA = !order ? srcC : srcC + src_pitch;
        srcB = srcA + src_pitch;
        srcC = srcB + src_pitch;
        srcD = srcC + src_pitch;
        srcE = srcD + src_pitch;

        nxtA = order ? nxtC : nxtC + nxt_pitch;
        nxtB = nxtA + nxt_pitch;
        nxtC = nxtB + nxt_pitch;
        nxtD = nxtC + nxt_pitch;
        nxtE = nxtD + nxt_pitch;

        for (y = 0; y + 4 < height; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                temp = std::abs((srcA[x] + srcC[x] + srcE[x] + 1) / 3 - (srcB[x] + srcD[x] + 1) / 2);
                temp = temp < nt ? 0 : temp;
                diff[0] += sse ? temp * temp : temp;

                temp = std::abs((nxtA[x] + nxtC[x] + nxtE[x] + 1) / 3 - (nxtB[x] + nxtD[x] + 1) / 2);
                temp = temp < ntN ? 0 : temp;
                diff[1] += sse ? temp * temp : temp;
            }
            srcA += src_pitch * static_cast<int64_t>(2);
            srcB += src_pitch * static_cast<int64_t>(2);
            srcC += src_pitch * static_cast<int64_t>(2);
            srcD += src_pitch * static_cast<int64_t>(2);
            srcE += src_pitch * static_cast<int64_t>(2);
            nxtA += nxt_pitch * static_cast<int64_t>(2);
            nxtB += nxt_pitch * static_cast<int64_t>(2);
            nxtC += nxt_pitch * static_cast<int64_t>(2);
            nxtD += nxt_pitch * static_cast<int64_t>(2);
            nxtE += nxt_pitch * static_cast<int64_t>(2);
        }
    }
    return diff[1] < diff[0] ? 1 : 0;
}

bool CTelecine_mode4(PVideoFrame& src, PVideoFrame& nxt, int nt, int ntN, bool sse, int order, bool isY8)
{
    const unsigned char* srcA;
    const unsigned char* srcB;
    const unsigned char* srcC;
    const unsigned char* srcD;
    const unsigned char* srcE;

    const unsigned char* nxtA;
    const unsigned char* nxtB;
    const unsigned char* nxtC;
    const unsigned char* nxtD;
    const unsigned char* nxtE;

    int height;
    int row_size;
    int src_pitch;
    int nxt_pitch;
    int x, y, p, temp;
    int64_t diff[2] = { 0, 0 };
    int planes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (p = 0; p < 3; p++)
    {
        if (p > 0 && isY8)
        {
            break;
        }

        srcC = src->GetReadPtr(planes[p]);
        src_pitch = src->GetPitch(planes[p]);
        nxtC = nxt->GetReadPtr(planes[p]);
        nxt_pitch = nxt->GetPitch(planes[p]);

        height = src->GetHeight(planes[p]);
        row_size = src->GetRowSize(planes[p]);

        srcA = !order ? srcC : srcC + src_pitch;
        srcB = srcA + src_pitch;
        srcC = srcB + src_pitch;
        srcD = srcC + src_pitch;
        srcE = srcD + src_pitch;

        nxtA = order ? nxtC : nxtC + nxt_pitch;
        nxtB = nxtA + nxt_pitch;
        nxtC = nxtB + nxt_pitch;
        nxtD = nxtC + nxt_pitch;
        nxtE = nxtD + nxt_pitch;

        for (y = 0; y + 4 < height; y += 2)
        {
            for (x = 0; x < row_size; x += 1)
            {
                temp = std::abs((srcA[x] + (srcC[x] << 2) + srcE[x]) - 3 * (srcB[x] + srcD[x]));
                temp = temp < nt * 6 ? 0 : temp;
                diff[0] += sse ? temp * temp : temp;

                temp = std::abs((nxtA[x] + (nxtC[x] << 2) + nxtE[x]) - 3 * (nxtB[x] + nxtD[x]));
                temp = temp < ntN * 6 ? 0 : temp;
                diff[1] += sse ? temp * temp : temp;
            }
            srcA += src_pitch * static_cast<int64_t>(2);
            srcB += src_pitch * static_cast<int64_t>(2);
            srcC += src_pitch * static_cast<int64_t>(2);
            srcD += src_pitch * static_cast<int64_t>(2);
            srcE += src_pitch * static_cast<int64_t>(2);
            nxtA += nxt_pitch * static_cast<int64_t>(2);
            nxtB += nxt_pitch * static_cast<int64_t>(2);
            nxtC += nxt_pitch * static_cast<int64_t>(2);
            nxtD += nxt_pitch * static_cast<int64_t>(2);
            nxtE += nxt_pitch * static_cast<int64_t>(2);
        }
    }
    return diff[1] < diff[0] ? 1 : 0;
}