#include "utils.h"

Utils::Utils()
{

}

size_t min(size_t x, size_t y, size_t z)
{
    return x < y ? min(x,z) : min(y,z);
}

size_t edit_distance(const string& A, const string& B)
{
    size_t NA = A.size();
    size_t NB = B.size();

    vector< vector<size_t> > M(NA + 1, vector<size_t>(NB + 1));

    for (size_t a = 0; a <= NA; ++a)
        M[a][0] = a;

    for (size_t b = 0; b <= NB; ++b)
        M[0][b] = b;

    for (size_t a = 1; a <= NA; ++a)
        for (size_t b = 1; b <= NB; ++b)
        {
            size_t x = M[a-1][b] + 1;
            size_t y = M[a][b-1] + 1;
            size_t z = M[a-1][b-1] + (A[a-1] == B[b-1] ? 0 : 1);
            M[a][b] = min(x,y,z);
        }

    return M[A.size()][B.size()];
}

bool isRepetitive(const string& s)
{
    int count = 0;
    for (int i=0; i<(int)s.size(); i++)
    {
        if ((s[i] == 'i') ||
                (s[i] == 'l') ||
                (s[i] == 'I'))
            count++;
    }
    if (count > ((int)s.size()+1)/2)
    {
        return true;
    }
    return false;
}


void er_draw(vector<Mat> &channels, vector<vector<ERStat> > &regions, vector<Vec2i> group, Mat& segmentation)
{
    for (int r=0; r<(int)group.size(); r++)
    {
        ERStat er = regions[group[r][0]][group[r][1]];
        if (er.parent != NULL) // deprecate the root region
        {
            int newMaskVal = 255;
            int flags = 4 + (newMaskVal << 8) + FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
            floodFill(channels[group[r][0]],segmentation,Point(er.pixel%channels[group[r][0]].cols,er.pixel/channels[group[r][0]].cols),
                    Scalar(255),0,Scalar(er.level),Scalar(0),flags);
        }
    }
}

bool   sort_by_lenght(const string &a, const string &b){return (a.size()>b.size());}

size_t lev_edit_distance ( size_t len1  , const char* string1,
                           size_t len2  , const char* string2,
                           int    xcost )
{
    size_t i;
    size_t *row;  /* we only need to keep one row of costs */
    size_t *end;
    size_t half;

    /* strip common prefix */
    while (len1 > 0 && len2 > 0 && *string1 == *string2)
    {
        len1--;
        len2--;
        string1++;
        string2++;
    }

    /* strip common suffix */
    while (len1 > 0 && len2 > 0 && string1[len1-1] == string2[len2-1]) {
        len1--;
        len2--;
    }

    /* catch trivial cases */
    if (len1 == 0)
        return len2;
    if (len2 == 0)
        return len1;

    /* make the inner cycle (i.e. string2) the longer one */
    if (len1 > len2) {
        size_t nx = len1;
        const char* sx = string1;
        len1 = len2;
        len2 = nx;
        string1 = string2;
        string2 = sx;
    }
    /* check len1 == 1 separately */
    if (len1 == 1) {
        if (xcost)
            return len2 + 1 - 2*(memchr(string2, *string1, len2) != NULL);
        else
            return len2 - (memchr(string2, *string1, len2) != NULL);
    }
    len1++;
    len2++;
    half = len1 >> 1;

    /* initalize first row */
    row = (size_t*)malloc(len2*sizeof(size_t));
    if (!row)
        return (size_t)(-1);
    end = row + len2 - 1;
    for (i = 0; i < len2 - (xcost ? 0 : half); i++)
        row[i] = i;

    /* go through the matrix and compute the costs.  yes, this is an extremely
   * obfuscated version, but also extremely memory-conservative and relatively
   * fast.  */
    if (xcost) {
        for (i = 1; i < len1; i++) {
            size_t *p = row + 1;
            const char char1 = string1[i - 1];
            const char *char2p = string2;
            size_t D = i;
            size_t x = i;
            while (p <= end) {
                if (char1 == *(char2p++))
                    x = --D;
                else
                    x++;
                D = *p;
                D++;
                if (x > D)
                    x = D;
                *(p++) = x;
            }
        }
    }
    else {
        /* in this case we don't have to scan two corner triangles (of size len1/2)
     * in the matrix because no best path can go throught them. note this
     * breaks when len1 == len2 == 2 so the memchr() special case above is
     * necessary */
        row[0] = len1 - half - 1;
        for (i = 1; i < len1; i++) {
            size_t *p;
            const char char1 = string1[i - 1];
            const char *char2p;
            size_t D, x;
            /* skip the upper triangle */
            if (i >= len1 - half) {
                size_t offset = i - (len1 - half);
                size_t c3;

                char2p = string2 + offset;
                p = row + offset;
                c3 = *(p++) + (char1 != *(char2p++));
                x = *p;
                x++;
                D = x;
                if (x > c3)
                    x = c3;
                *(p++) = x;
            }
            else {
                p = row + 1;
                char2p = string2;
                D = x = i;
            }
            /* skip the lower triangle */
            if (i <= half + 1)
                end = row + len2 + i - half - 2;
            /* main */
            while (p <= end) {
                size_t c3 = --D + (char1 != *(char2p++));
                x++;
                if (x > c3)
                    x = c3;
                D = *p;
                D++;
                if (x > D)
                    x = D;
                *(p++) = x;
            }
            /* lower triangle sentinel */
            if (i <= half) {
                size_t c3 = --D + (char1 != *char2p);
                x++;
                if (x > c3)
                    x = c3;
                *p = x;
            }
        }
    }

    i = *end;
    free(row);
    return i;
}

float similar(string wo, string pat)
{
    size_t l1 = wo.length();
    size_t l2 = pat.length();
    size_t lsum = l1 + l2;
    if (lsum == 0) {
        return 1;
    }


    size_t distance = lev_edit_distance(l1, wo.c_str(), l2, pat.c_str(), 1);
    return ((double)lsum - distance) / (lsum);
}


string shell(string a2)
{
    int v3; // eax@1
    int v64; // [sp+1Ch] [bp-18960h]@8

    string v2; // ebx@1
    string v40; // edx@49
    string v94; // [sp+188FCh] [bp-80h]@1
    string v95; // [sp+18900h] [bp-7Ch]@1
    string v96; // [sp+18904h] [bp-78h]@1
    string v97; // [sp+18908h] [bp-74h]@1
    string v98; // [sp+1890Ch] [bp-70h]@2
    string v99; // [sp+18910h] [bp-6Ch]@2
    string v100; // [sp+18914h] [bp-68h]@2
    string v102; // [sp+1891Ch] [bp-60h]@2
    string v103; // [sp+18920h] [bp-5Ch]@7
    string v107; // [sp+18930h] [bp-4Ch]@46

    v2 = a2;
    v94 = std::string("");
    v40 = v94;
    v95 = "";
    v3 = getpid();
    v97 = utos(v3);
    v96 =  "/tmp/shell_o" + v97;
    if (!v97.empty())
    {
        cout << "";
    }

    v98 = std::string(v2);
    v98.append(" > ",3);
    v99 = v98 + v96;
    v100 = std::string(v99);
    v100.append(" 2>", 3);
    v102 = v100;
    v2.assign(v102);

    v103 = "Executing " + v2;

    v64 = system(a2.c_str());
    std::ifstream t(v96.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    string ret = buffer.str();
    string::iterator s = ret.end()-1;
    if((*s) == '\n')
    {
        if (ret.size () > 0)  ret.resize (ret.size () - 1);
    }
    v94 = ret;

    if ( v64 )
    {
        v40 = std::string("ERROR");
    }
    else
    {
        v107 = std::string("Successfully executed");
        v40 = std::string(v94);

    }

    return v40;
}

string utos(long long a2)
{
    std::ostringstream buff;
    long long number = 0;
    number = a2;
    buff << number;
    return buff.str();
}

string file2string(string fn)
{
    std::ifstream t(fn.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    string ret = buffer.str();
    string::iterator s = ret.end()-1;
    if((*s) == '\n')
    {
        if (ret.size () > 0)  ret.resize (ret.size () - 1);
    }
    return trim(ret);
}

vector<string> splits(string str, string delim) {
    std::vector<std::string> tokens;
    if (str.size() > 0)
    {
        if (delim.size() > 0)
        {
            std::string::size_type currPos = 0, prevPos = 0;
            while ((currPos = str.find(delim, prevPos)) != std::string::npos)
            {
                string item = str.substr(prevPos, currPos - prevPos);
                if (item.size() > 0)
                {
                    tokens.push_back(item);
                }
                prevPos = currPos + 1;
            }
            tokens.push_back(str.substr(prevPos));
        }
        else
        {
            tokens.push_back(str);
        }
    }
    return tokens;
}

string trim(string sStr)
{
    int nSize = sStr.size();
    int nSPos = 0, nEPos = 1, i;
    for(i = 0; i< nSize; ++i) {
        if( !isspace( sStr[i] ) ) {
            nSPos = i ;
            break;
        }
    }
    for(i = nSize -1 ; i >= 0 ; --i) {
        if( !isspace( sStr[i] ) ) {
            nEPos = i;
            break;
        }
    }
    return string(sStr, nSPos, nEPos - nSPos + 1);
}

int stou(string v1)
{
    return strtol(v1.c_str(), NULL,10);
}
