#include "CReplaceDuplicate.h"

CReplaceDuplicate::CReplaceDuplicate(PClip _child, bool _drop, IScriptEnvironment* env) : GenericVideoFilter(_child), drop(_drop)
{
    ifstream ifile;
    ifile.open("CDuplicate.txt");
    if (!ifile)
    {
        env->ThrowError("CReplaceDuplicate: CDuplicate.txt not found!");
    }

    int i = 0;
    string line;
    if (!drop)
    {
        while (getline(ifile, line) && i < vi.num_frames)
        {
            if (line[0] == '0')
            {
                vector[i] = int(stoi(line.substr(2, string::npos)));
            }
            else
            {
                vector[i] = -1;
            }
            i += 1;
        }
    }
    else
    {
        while (getline(ifile, line) && i < vi.num_frames)
        {
            if (line[0] == '0')
            {
                vector[i] = int(stoi(line.substr(2, string::npos)));
                i += 1;
            }
        }
        vi.num_frames = i;
    }

    ifile.close();
}

PVideoFrame __stdcall CReplaceDuplicate::GetFrame(int n, IScriptEnvironment* env)
{
    if (!drop)
    {
        if (vector[n] == -1)
        {
            while (vector[n] == -1)
            {
                n -= 1;
            }
        }
    }
    PVideoFrame src = child->GetFrame(vector[n], env);
    return src;
}