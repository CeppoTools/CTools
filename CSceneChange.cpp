#include "CSceneChange.h"

CSceneChange::CSceneChange(PClip _child, PClip _sClip, int _blksize, int _thSCD1, int _thSCD2, int _minKey, int _maxLuma, bool _write, bool _inputTxt, IScriptEnvironment* env) : GenericVideoFilter(_child), sClip(_sClip), blksize(_blksize), thSCD1(_thSCD1), thSCD2(_thSCD2), minKey(_minKey), maxLuma(_maxLuma), write(_write), inputTxt(_inputTxt)
{
    if (!vi.IsY8() && !vi.IsYV12() && !vi.IsYV16() && !vi.IsYV24())
    {
        env->ThrowError("CSceneChange: supported colorspaces are Y8, YV12, YV16, YV24!");
    }
    else if (write && inputTxt)
    {
        env->ThrowError("CSceneChange: can't read and write a file at the same time!");
    }

    if (write)
    {
        int i;
        for (i = 0; i < vi.num_frames; i += 1)
        {
            isOut[i] = 0;
            isOut2[i] = 0;
        }
        file.open("CSceneChange.txt", ios::trunc);
        file2.open("CSceneChange_AverageLuma.txt", ios::trunc);
    }
    else if (inputTxt)
    {
        ofstream TRIM;
        ifstream SC, LU;
        SC.open("CSceneChange.txt");
        LU.open("CSceneChange_AverageLuma.txt");
        TRIM.open("SCTrim.txt", ios::trunc);

        if (!SC || !LU)
        {
            env->ThrowError("CSceneChange: CSceneChange.txt or CSceneChange_AverageLuma.txt not found!");
        }

        int i;
        int j = 0;
        string line;
        int64_t sum = 0;
        int index[2] = { 0, -1 };
        for (i = 0; i < vi.num_frames; i += 1)
        {
            j += 1;
            getline(LU, line);
            sum += (int)stoi(line.substr(0, line.find(' ')));
            getline(SC, line);
            if (i == vi.num_frames - 1)
            {
                index[0] = index[1] + 1;
                index[1] = (int)stoi(line.substr(2, string::npos));
                j = sum / (float)j <= maxLuma;
                line = j ? "[*DARK*] " : "";
                TRIM << line << "Trim(" << index[0] << ", " << index[1] << ")\n";
            }
            else if (i && line[0] == '1')
            {
                index[0] = index[1] + 1;
                index[1] = (int)stoi(line.substr(2, string::npos)) - 1;
                j = sum / (float)j < maxLuma;
                line = j ? "[*DARK*] " : "";
                TRIM << line << "Trim(" << index[0] << ", " << index[1] << ")+\\\n";
                j = 0;
                sum = 0;
            }
        }
        SC.close();
        LU.close();
        TRIM.close();
        env->ThrowError("CSceneChange: SCTrim.txt has been created!");
    }

    if (!sClip)
    {
        try
        {
            AVSValue sup_args[1] = { child };
            PClip Super = env->Invoke("MSuper", AVSValue(sup_args, 1)).AsClip();
            AVSValue vec_args[5] = { Super, blksize, blksize / 2, false, 1 };
            const char* vec_names[5] = { NULL, "blksize", "overlap", "isb", "delta" };
            PClip forwvec = env->Invoke("MAnalyse", AVSValue(vec_args, 5), vec_names).AsClip();
            AVSValue sClip_args[4] = { child, forwvec, thSCD1, thSCD2 };
            const char* sClip_names[4] = { NULL, NULL, "thSCD1", "thSCD2" };
            sClip = env->Invoke("MSCDetection", AVSValue(sClip_args, 4), sClip_names).AsClip();
        }
        catch (IScriptEnvironment::NotFound)
        {
            env->ThrowError("CSceneChange: mvtools2(.dll) not found!");
        }
    }
}

PVideoFrame __stdcall CSceneChange::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame src = child->GetFrame(n, env);
    PVideoFrame scc = sClip->GetFrame(n, env);
    
    const unsigned char* sccp;
    sccp = scc->GetReadPtr(PLANAR_Y);
    bool isSC = min(1, sccp[0]);

    if (count >= minKey && isSC && !isOut[n] || n == 0)
    {
        count = 0;
        file << 1 << " " << n << "\n";
        isOut[n] = 1;
        n == vi.num_frames - 1 ? file.close() : NULL;
    }
    else if(!isOut[n])
    {
        count += 1;
        file << 0 << " " << n << "\n";
        isOut[n] = 1;
        n == vi.num_frames - 1 ? file.close() : NULL;
    }
    
    int x, y;
    int64_t sum = 0;
    int height = src->GetHeight(PLANAR_Y);
    int row_size = src->GetRowSize(PLANAR_Y);
    int src_pitch = src->GetPitch(PLANAR_Y);
    const unsigned char* srcp = src->GetReadPtr(PLANAR_Y);

    for (y = 0; y < height; y += 1)
    {
        for (x = 0; x < row_size; x += 1)
        {
            sum += srcp[x];
        }
        srcp += src_pitch;
    }

    if (!isOut2[n])
    {
        file2 << int(sum / static_cast<float>(height)
                         / static_cast<float>(row_size) + 0.5f) << " " << n << "\n";
        isOut2[n] = 1;
        n == vi.num_frames - 1 ? file2.close() : NULL;
    }

    return src;
}