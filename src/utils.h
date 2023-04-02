#ifndef UTILS_H
#define UTILS_H

#include "mdelogo.h"

class Utils
{
public:
    Utils();
};

float similar(string wo, string pat);
size_t lev_edit_distance ( size_t len1  , const char* string1,
                           size_t len2  , const char* string2,
                           int    xcost );

//Calculate edit distance netween two words
size_t edit_distance(const string& A, const string& B);
size_t min(size_t x, size_t y, size_t z);
bool   isRepetitive(const string& s);
bool   sort_by_lenght(const string &a, const string &b);
//Draw ER's in an image via floodFill
void   er_draw(vector<Mat> &channels, vector<vector<ERStat> > &regions, vector<Vec2i> group, Mat& segmentation);
string shell(string a2);
string utos(long long a2);
string file2string(string fn);
vector<string> splits(string str, string delim);
string trim(string sStr);
int stou(string v1);

#endif // UTILS_H
