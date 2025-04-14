#ifndef ARNOLD_CAT_MAP_H
#define ARNOLD_CAT_MAP_H

#include <vector>

using namespace std;

// Apply Arnold's Cat Map scrambling
vector<vector<unsigned char>> arnoldScramble(const vector<vector<unsigned char>>& image, int iterations);
vector<vector<unsigned char>> arnoldUnscramble(const vector<vector<unsigned char>>& image, int iterations);

#endif
