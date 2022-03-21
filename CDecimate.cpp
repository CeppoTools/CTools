#include "CDecimate.h"

CDecimate::CDecimate(PClip _child, bool _isBob, bool _error, bool _heuristic, PClip _iClip, IScriptEnvironment* env)
    : GenericVideoFilter(_child), isBob(_isBob), error(_error), heuristic(_heuristic), iClip(_iClip)
{
    ifstream ifile;
    ifile.open("CTelecine.txt");
    if (!ifile)
    {
        env->ThrowError("CDecimate: CTelecine.txt not found!");
    }
    ifile.close();

    int check = CDecimate_StartUp(isBob ? vi.num_frames / 2 : vi.num_frames, error, heuristic, iClip ? 1 : 0);
    if (check <= 0)
    {
        string text = "CDecimate: cycle at frame " + to_string(abs(check)) + " has not a correct decimation pattern. Edit the txt file.";
        char const* pchar = text.c_str();
        env->ThrowError(pchar);
    }

    int i = 0;
    string line;
    ifstream ifs("CTelecine.txt");
    
    for (i = 0; i < vi.num_frames; i += 1)
    {
        isInt[i] = 0;
    }

    int j = 0;
    int k = 0;
    int val[5];
    int frm[5];
    int nf = vi.num_frames - 1;
    int NF = isBob ? vi.num_frames / 2 : vi.num_frames;

    for (i = 0; i < NF; i += 5)
    {
        for(k = 0; k < 5; k += 1)
        {
            if (i + k >= NF)
            {
                break;
            }
            getline(ifs, line);
            val[k] = (int)line[0] - 48;
            frm[k] = (int)stoi(line.substr(2, string::npos));
        }
        if (isBob && val[0] == 2 && val[1] == 2 && val[2] == 2 && val[3] == 2 && val[4] == 2)
        {
            if (iClip)
            {
                vec[min(j + 0, nf)] = j + 0;
                vec[min(j + 1, nf)] = j + 1;
                vec[min(j + 2, nf)] = j + 2;
                vec[min(j + 3, nf)] = j + 3;

                isInt[min(j + 0, nf)] = 1;
                isInt[min(j + 1, nf)] = 1;
                isInt[min(j + 2, nf)] = 1;
                isInt[min(j + 3, nf)] = 1;
                j -= 1;
            }
            else
            {
                vec[min(j + 0, nf)] = frm[0] * 2;
                vec[min(j + 1, nf)] = frm[0] * 2 + 1;
                vec[min(j + 2, nf)] = frm[1] * 2;
                vec[min(j + 3, nf)] = frm[1] * 2 + 1;
                vec[min(j + 4, nf)] = frm[2] * 2;
                vec[min(j + 5, nf)] = frm[2] * 2 + 1;
                vec[min(j + 6, nf)] = frm[3] * 2;
                vec[min(j + 7, nf)] = frm[3] * 2 + 1;
                vec[min(j + 8, nf)] = frm[4] * 2;
                vec[min(j + 9, nf)] = frm[4] * 2 + 1;
                j += 5;
            } 
        }
        else if (!val[0] && !val[1] && !val[2] && !val[3] && !val[4])
        {
            if (iClip)
            {
                vec[min(j + 0, nf)] = j + 0;
                vec[min(j + 1, nf)] = j + 1;
                vec[min(j + 2, nf)] = j + 2;
                vec[min(j + 3, nf)] = j + 3;

                isInt[min(j + 0, nf)] = 1;
                isInt[min(j + 1, nf)] = 1;
                isInt[min(j + 2, nf)] = 1;
                isInt[min(j + 3, nf)] = 1;
                j -= 1;
            }
            else
            {
                vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
                vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
                vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
                vec[min(j + 3, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
                vec[min(j + 4, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            }
        }
        else if (val[0] && val[1] && !val[2] && !val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (!val[0] && val[1] && val[2] && !val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (!val[0] && !val[1] && val[2] && val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (!val[0] && !val[1] && !val[2] && val[3] && val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 3, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            j -= 1;
        }
        else if (val[0] && !val[1] && !val[2] && !val[3] && val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 1, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && val[0] && val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 1, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && val[3] && val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 3, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            j -= 1;
        }
        else if (heuristic && val[2] && val[3])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && val[1] && val[2])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && val[0] && val[1])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && val[0] && !val[1] && !val[2] && !val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 1, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && !val[0] && val[1] && !val[2] && !val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && !val[0] && !val[1] && val[2] && !val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && !val[0] && !val[1] && !val[2] && val[3] && !val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 3, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            j -= 1;
        }
        else if (heuristic && !val[0] && !val[1] && !val[2] && !val[3] && val[4])
        {
            vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
            vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
            vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
            vec[min(j + 3, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
            j -= 1;
        }
        else if (!error)
        {
            if (iClip)
            {
                vec[min(j + 0, nf)] = j + 0;
                vec[min(j + 1, nf)] = j + 1;
                vec[min(j + 2, nf)] = j + 2;
                vec[min(j + 3, nf)] = j + 3;

                isInt[min(j + 0, nf)] = 1;
                isInt[min(j + 1, nf)] = 1;
                isInt[min(j + 2, nf)] = 1;
                isInt[min(j + 3, nf)] = 1;
                j -= 1;
            }
            else
            {
                vec[min(j + 0, nf)] = isBob ? frm[0] * 2 + 1 : frm[0];
                vec[min(j + 1, nf)] = isBob ? frm[1] * 2 + 1 : frm[1];
                vec[min(j + 2, nf)] = isBob ? frm[2] * 2 + 1 : frm[2];
                vec[min(j + 3, nf)] = isBob ? frm[3] * 2 + 1 : frm[3];
                vec[min(j + 4, nf)] = isBob ? frm[4] * 2 + 1 : frm[4];
            }
        }
        j += 5;

    }
    ifs.close();
    
    if (isBob)
    {
        vi.SetFPS(vi.fps_numerator * 2 / 5, vi.fps_denominator);
        vi.num_frames = check;
    }
    else
    {
        vi.SetFPS(vi.fps_numerator * 4 / 5, vi.fps_denominator);
        vi.num_frames = check;
    }
}

PVideoFrame __stdcall CDecimate::GetFrame(int n, IScriptEnvironment* env) {

    
    PVideoFrame src = isInt[n] ? iClip->GetFrame(vec[n], env) : child->GetFrame(vec[n], env);
    return src;

}

int CDecimate_StartUp(int nf, bool error, bool heuristic, bool iClip)
{
    int i;
    int j;
    string line;
    int frame[5];
    ifstream ifs("CTelecine.txt");

    ofstream file;
    file.open("VFR.txt", ios::trunc);
    file << "# timecode format v1\n";
    file << "Assume 23.976000\n";

    int frame_count = 0;
    for (i = 0; i + 4 < nf; i += 5)
    {
        for (j = 0; j < 5; j += 1)
        {
            getline(ifs, line);
            frame[j] = (int)line[0] - 48;
        }

        if (!iClip && frame[0] == 2 && frame[1] == 2 && frame[2] == 2 && frame[3] == 2 && frame[4] == 2)
        {
            file << frame_count << "," << frame_count + 9 << "," << "59.940000\n";
            frame_count += 10;
        }
        else if (!iClip && !frame[0] && !frame[1] && !frame[2] && !frame[3] && !frame[4])
        {
            file << frame_count << "," << frame_count + 4 << "," << "29.970000\n";
            frame_count += 5;
        }
        else if ( frame[0] &&  frame[1] && !frame[2] && !frame[3] && !frame[4]
              || !frame[0] &&  frame[1] &&  frame[2] && !frame[3] && !frame[4]
              || !frame[0] && !frame[1] &&  frame[2] &&  frame[3] && !frame[4]
              || !frame[0] && !frame[1] && !frame[2] &&  frame[3] &&  frame[4]
              ||  frame[0] && !frame[1] && !frame[2] && !frame[3] &&  frame[4])
        {
            file << frame_count << "," << frame_count + 3 << "," << "23.976000\n";
            frame_count += 4;
        }
        else if (heuristic && frame[0] && frame[1]
              || heuristic && frame[1] && frame[2]
              || heuristic && frame[2] && frame[3]
              || heuristic && frame[3] && frame[4]
              || heuristic && frame[0] && frame[4])
        {
            file << frame_count << "," << frame_count + 3 << "," << "23.976000\n";
            frame_count += 4;
        }
        else if (heuristic &&  frame[0] && !frame[1] && !frame[2] && !frame[3] && !frame[4]
              || heuristic && !frame[0] &&  frame[1] && !frame[2] && !frame[3] && !frame[4]
              || heuristic && !frame[0] && !frame[1] &&  frame[2] && !frame[3] && !frame[4]
              || heuristic && !frame[0] && !frame[1] && !frame[2] &&  frame[3] && !frame[4]
              || heuristic && !frame[0] && !frame[1] && !frame[2] && !frame[3] &&  frame[4])
        {
            file << frame_count << "," << frame_count + 3 << "," << "23.976000\n";
            frame_count += 4;
        }
        else if (!error && !iClip)
        {
            file << frame_count << "," << frame_count + 4 << "," << "29.970000\n";
            frame_count += 5;
        }
        else if (iClip)
        {
            file << frame_count << "," << frame_count + 3 << "," << "23.976000\n";
            frame_count += 4;
        }
        else
        {
            ifs.close();
            return -i;
        }
    }
    ifs.close();
    return frame_count + (nf - i - 1);
}