#include "CSetPattern.h"

CSetPattern::CSetPattern(PClip _child, int _start, int _end, bool _CCCCC, bool _NNCCC, bool _CNNCC, bool _CCNNC, bool _CCCNN, bool _NCCCN, IScriptEnvironment* env) : GenericVideoFilter(_child), start(_start), end(_end), CCCCC(_CCCCC), NNCCC(_NNCCC), CNNCC(_CNNCC), CCNNC(_CCNNC), CCCNN(_CCCNN), NCCCN(_NCCCN)
{
    ifstream ifile;
    ifile.open("CTelecine.txt");
    if (!ifile)
    {
        env->ThrowError("CSetPattern: CTelecine.txt not found!");
    }
    else if (start % 5 != 0)
    {
        env->ThrowError("CSetPattern: start must be a multiple of five!");
    }
    else if (end % 5 != 4)
    {
        env->ThrowError("CSetPattern: end must be a multiple of five - 1!");
    }
    else if (!CCCCC && !NNCCC && !CNNCC && !CCNNC && !CCCNN && !NCCCN)
    {
        env->ThrowError("CSetPattern: at least one pattern must be specified!");
    }

    int i;
    string line;
    int Cycle = vi.num_frames / 5 + 1;
    auto C = new string[Cycle][5];

    for (i = 0; i < vi.num_frames; i += 1)
    {
        getline(ifile, line);
        C[i / 5][i % 5] = line;
    }
    ifile.close();

    ofstream file;
    file.open("CTelecine.txt", ios::trunc);

    for (i = 0; i < Cycle; i += 1)
    {
        if (i * 5 < start || i * 5 + 4 > end)
        {
            file << C[i][0] << '\n';
            file << C[i][1] << '\n';
            file << C[i][2] << '\n';
            file << C[i][3] << '\n';
            file << C[i][4] << '\n';
        }
        else if (CCCCC)
        {
            file << 0 << " " << i * 5 + 0 << '\n';
            file << 0 << " " << i * 5 + 1 << '\n';
            file << 0 << " " << i * 5 + 2 << '\n';
            file << 0 << " " << i * 5 + 3 << '\n';
            file << 0 << " " << i * 5 + 4 << '\n';
        }
        else if (NNCCC)
        {
            file << 1 << " " << i * 5 + 0 << '\n';
            file << 1 << " " << i * 5 + 1 << '\n';
            file << 0 << " " << i * 5 + 2 << '\n';
            file << 0 << " " << i * 5 + 3 << '\n';
            file << 0 << " " << i * 5 + 4 << '\n';
        }
        else if (CNNCC)
        {
            file << 0 << " " << i * 5 + 0 << '\n';
            file << 1 << " " << i * 5 + 1 << '\n';
            file << 1 << " " << i * 5 + 2 << '\n';
            file << 0 << " " << i * 5 + 3 << '\n';
            file << 0 << " " << i * 5 + 4 << '\n';
        }
        else if (CCNNC)
        {
            file << 0 << " " << i * 5 + 0 << '\n';
            file << 0 << " " << i * 5 + 1 << '\n';
            file << 1 << " " << i * 5 + 2 << '\n';
            file << 1 << " " << i * 5 + 3 << '\n';
            file << 0 << " " << i * 5 + 4 << '\n';
        }
        else if (CCCNN)
        {
            file << 0 << " " << i * 5 + 0 << '\n';
            file << 0 << " " << i * 5 + 1 << '\n';
            file << 0 << " " << i * 5 + 2 << '\n';
            file << 1 << " " << i * 5 + 3 << '\n';
            file << 1 << " " << i * 5 + 4 << '\n';
        }
        else if (NCCCN)
        {
            file << 1 << " " << i * 5 + 0 << '\n';
            file << 0 << " " << i * 5 + 1 << '\n';
            file << 0 << " " << i * 5 + 2 << '\n';
            file << 0 << " " << i * 5 + 3 << '\n';
            file << 1 << " " << i * 5 + 4 << '\n';
        }
    }
    file.close();
}

PVideoFrame __stdcall CSetPattern::GetFrame(int n, IScriptEnvironment* env) {

    PVideoFrame src = child->GetFrame(n, env);
    return src;
}