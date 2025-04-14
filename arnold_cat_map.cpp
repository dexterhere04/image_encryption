#include "arnold_cat_map.h"


vector<vector<unsigned char>> arnoldScramble(const vector<vector<unsigned char>>& image, int iterations) {
    int N = image.size();
    vector<vector<unsigned char>> scrambled = image;

    for (int k = 0; k < iterations; k++) {
        vector<vector<unsigned char>> temp(N, vector<unsigned char>(N));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                int x_new = (i + j) % N;
                int y_new = (i + 2 * j) % N;
                temp[x_new][y_new] = scrambled[i][j];
            }
        }
        scrambled = temp;
    }
    return scrambled;
}
vector<vector<unsigned char>> arnoldUnscramble(const vector<vector<unsigned char>>& image, int iterations) {
    int N = image.size();
    vector<vector<unsigned char>> result = image;

    for (int iter = 0; iter < iterations; iter++) {
        vector<vector<unsigned char>> temp(N, vector<unsigned char>(N));
        for (int x = 0; x < N; x++) {
            for (int y = 0; y < N; y++) {
                int newX = (2 * x - y + N) % N;
                int newY = (-x + y + N) % N;
                temp[newX][newY] = result[x][y];
            }
        }
        result = temp;
    }

    return result;
}