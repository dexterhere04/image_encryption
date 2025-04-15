#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;
struct PaillierKey {
    long long n, g, lambda, mu;
};
long long paillierDecrypt(long long c, PaillierKey key) {
    return ((c - 1) / key.n) % key.n;
}
void savePGM(const string& filename, const vector<vector<unsigned char>>& image) {
    int height = image.size();
    int width = image[0].size();
    ofstream file(filename, ios::binary);
    
    if (!file) {
        cerr << "Error: Cannot save file!" << endl;
        exit(1);
    }

    file << "P5\n" << width << " " << height << "\n255\n";
    for (const auto& row : image) {
        file.write(reinterpret_cast<const char*>(row.data()), row.size());
    }
    file.close();
}

void displayPGM(const string& filename) {
    Mat image = imread(filename, IMREAD_GRAYSCALE);
    if (image.empty()) {
        cerr << "Error: Cannot open PGM file!" << endl;
        return;
    }

    imshow("Processed PGM Image", image);
    waitKey(0);  
    destroyAllWindows();
}
vector<vector<long long>> decryptImage(const vector<vector<long long>>& encryptedImage, PaillierKey key) {
    int height = encryptedImage.size();
    int width = encryptedImage[0].size();
    vector<vector<long long>> decryptedImage(height, vector<long long>(width));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if ((i + j) % 4 == 0) {  
                decryptedImage[i][j] = paillierDecrypt(encryptedImage[i][j], key);
            } else {
                decryptedImage[i][j] = encryptedImage[i][j];  // Non-encrypted pixels stay as-is
            }
        }
    }
    return decryptedImage;
}
vector<vector<unsigned char>> convertToUnsignedChar(const vector<vector<long long>>& image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<unsigned char>> result(height, vector<unsigned char>(width));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            result[i][j] = static_cast<unsigned char>(image[i][j]);
        }
    }
    return result;
}
vector<vector<long long>> loadEncryptedImage(const string& filename, int height, int width) {
    vector<vector<long long>> image(height, vector<long long>(width));
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open encrypted file!" << endl;
        exit(1);
    }

    for (auto& row : image) {
        file.read(reinterpret_cast<char*>(row.data()), row.size() * sizeof(long long));
    }
    file.close();
    return image;
}
PaillierKey loadKey(const string& filename) {
    PaillierKey key;
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Cannot open key file!" << endl;
        exit(1);
    }
    file >> key.n >> key.g >> key.lambda >> key.mu;
    file.close();
    return key;
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
int main(int argc,char* argv[]){
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << "<directory>" << endl;
        return 1;
    }
    string directory = argv[1];   
    PaillierKey key = loadKey(directory+"/key.txt");
    string encryptedFile = directory+"/encrypted.bin";
    int newWidth = 128, newHeight = 128;
    int iterations=10;
    vector<vector<long long>> loadedEncrypted = loadEncryptedImage(encryptedFile, newHeight, newWidth);
    vector<vector<long long>> decryptedLong = decryptImage(loadedEncrypted, key);
    vector<vector<unsigned char>> decryptedImage = convertToUnsignedChar(decryptedLong);
    string decryptedFile = "decrypted.pgm";
    savePGM(decryptedFile, decryptedImage);
    cout << "Decryption completed successfully!" << endl;
    displayPGM(decryptedFile);
    auto unscrambled = arnoldUnscramble(decryptedImage, iterations);
    savePGM("unscrambled.pgm", unscrambled);
    displayPGM("unscrambled.pgm");
    return 0;
}