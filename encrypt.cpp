#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <vector>
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
void saveKey(const string& filename, const PaillierKey& key) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error: Cannot save key file!" << endl;
        exit(1);
    }
    file << key.n << " " << key.g << " " << key.lambda << " " << key.mu << endl;
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
int main(int argc, char* argv[]){
    //string filename="input.jpg";
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <image_path> <key_output_path>" << endl;
        return 1;
    }

    string filename = argv[1];
    string keyPath = argv[2];
    Mat image = imread(filename, IMREAD_COLOR);
    if (image.empty()) {
        cerr << "Error: Cannot open image!" << endl;
        return -1;
    }

    
    vector<vector<Vec3b>> imgVec = matToVector(image);
    
    vector<vector<unsigned char>> grayscale = convertToGrayscale(imgVec);
    savePGM("grayscale.pgm", grayscale);
    displayPGM("grayscale.pgm");
   
    int newWidth = 128, newHeight = 128;
    vector<vector<unsigned char>> resizedImage = resizeImage(grayscale, newWidth, newHeight);
    savePGM("resized.pgm", resizedImage);
    displayPGM("resized.pgm");
    int iterations = 10; 
    vector<vector<unsigned char>> scrambledImage = arnoldScramble(resizedImage, iterations);
    savePGM("scrambled.pgm",scrambledImage);
    displayPGM("scrambled.pgm");
    PaillierKey key = generatePaillierKeys(61, 53);
    vector<vector<long long>> encryptedImage = encryptImage(scrambledImage, key);
    string encryptedFile = keyPath+"/encrypted.bin";
    saveKey(keyPath+"/key.txt", key);
    saveEncryptedImage(encryptedFile, encryptedImage);
    
}