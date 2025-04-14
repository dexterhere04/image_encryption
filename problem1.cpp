#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <vector>
#include "arnold_cat_map.h"  

using namespace std;
using namespace cv;
struct PaillierKey {
    long long n, g, lambda, mu;
};

PaillierKey generatePaillierKeys(long long p, long long q) {
    PaillierKey key;
    key.n = p * q;
    key.g = key.n + 1;
    long long phi = (p - 1) * (q - 1);
    key.lambda = phi;
    key.mu = 1;  
    return key;
}

long long paillierEncrypt(long long m, PaillierKey key) {
    return (1 + (m * key.n)) % (key.n * key.n);
}
long long paillierDecrypt(long long c, PaillierKey key) {
    return ((c - 1) / key.n) % key.n;
}
vector<vector<Vec3b>> matToVector(const Mat& image) {
    vector<vector<Vec3b>> imgVec(image.rows, vector<Vec3b>(image.cols));
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            imgVec[i][j] = image.at<Vec3b>(i, j);
        }
    }
    return imgVec;
}
vector<vector<unsigned char>> convertToGrayscale(const vector<vector<Vec3b>>& image) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<unsigned char>> grayscale(height, vector<unsigned char>(width));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            Vec3b pixel = image[i][j];
            grayscale[i][j] = static_cast<unsigned char>(
                0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]
            );
        }
    }
    return grayscale;
}
vector<vector<unsigned char>> resizeImage(const vector<vector<unsigned char>>& image, int newWidth, int newHeight) {
    int oldWidth = image[0].size();
    int oldHeight = image.size();
    vector<vector<unsigned char>> resized(newHeight, vector<unsigned char>(newWidth));

    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            int srcX = j * oldWidth / newWidth;
            int srcY = i * oldHeight / newHeight;
            resized[i][j] = image[srcY][srcX];  
        }
    }
    return resized;
}
vector<vector<long long>> encryptImage(const vector<vector<unsigned char>>& image, PaillierKey key) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<long long>> encryptedImage(height, vector<long long>(width));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if ((i + j) % 4 == 0) {  
                encryptedImage[i][j] = paillierEncrypt(image[i][j], key);
            } else {
                encryptedImage[i][j] = image[i][j];  
            }
        }
    }
    return encryptedImage;
}

void saveEncryptedImage(const string& filename, const vector<vector<long long>>& image) {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot save file!" << endl;
        exit(1);
    }

    for (const auto& row : image) {
        file.write(reinterpret_cast<const char*>(row.data()), row.size() * sizeof(long long));
    }
    file.close();
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
// decryption

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


int main() {
    string filename = "input.jpg"; 
    Mat image = imread(filename, IMREAD_COLOR);

    if (image.empty()) {
        cerr << "Error: Cannot open image!" << endl;
        return -1;
    }

    
    vector<vector<Vec3b>> imgVec = matToVector(image);

    
    vector<vector<unsigned char>> grayscale = convertToGrayscale(imgVec);

   
    int newWidth = 128, newHeight = 128;
    vector<vector<unsigned char>> resizedImage = resizeImage(grayscale, newWidth, newHeight);
    string resizedFile = "grey_resized.pgm";
    savePGM(resizedFile,resizedImage);
    displayPGM(resizedFile);
   
    int iterations = 10; 
    vector<vector<unsigned char>> scrambledImage = arnoldScramble(resizedImage, iterations);


    string scrambledFile = "scrambled.pgm";
    savePGM(scrambledFile, scrambledImage);

    cout << "Image scrambling completed successfully!" << endl;
    displayPGM(scrambledFile);

    PaillierKey key = generatePaillierKeys(61, 53);
    vector<vector<long long>> encryptedImage = encryptImage(scrambledImage, key);
    string encryptedFile = "encrypted.bin";
    saveEncryptedImage(encryptedFile, encryptedImage);
    vector<vector<long long>> loadedEncrypted = loadEncryptedImage(encryptedFile, newHeight, newWidth);
    vector<vector<long long>> decryptedLong = decryptImage(loadedEncrypted, key);
    vector<vector<unsigned char>> decryptedImage = convertToUnsignedChar(decryptedLong);
    string decryptedFile = "decrypted.pgm";
    savePGM(decryptedFile, decryptedImage);
    cout << "Decryption completed successfully!" << endl;
    displayPGM(decryptedFile);
    cout << "Encryption completed successfully!" << endl;
    auto unscrambled = arnoldUnscramble(decryptedImage, iterations);
    savePGM("unscrambled.pgm", unscrambled);
    displayPGM("unscrambled.pgm");
    return 0;
}
