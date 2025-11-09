#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <cmath>


void bubble_sort(int arr[], int n) {
    int temp;

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}


void median(int filter_size, int height, int width, int** copied, int** temp) {
    int offset = filter_size / 2, arr_size = filter_size * filter_size;
    int* filter_arr = new int[arr_size];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = 0;

            for (int x = -offset; x <= offset; x++) {
                for (int y = -offset; y <= offset; y++) {
                    int nx = i + x;
                    int ny = j + y;
                    if (nx >= 0 && nx < height && ny >= 0 && ny < width) {
                        filter_arr[index] = copied[nx][ny];
                        index++;
                    }
                }
            }
            bubble_sort(filter_arr, index);
            temp[i][j] = filter_arr[index / 2];
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            copied[i][j] = temp[i][j];
        }
    }

    delete[] filter_arr;
}


double compare(int** image, int** copied, int height, int width) {
    double size = height * width, match = 0.0, result;

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            if(abs(image[i][j] - copied[i][j]) <= 10) {
                match++;
            }
        }
    }

    result = match / size * 100;
    return round(result);
}


bool read_pgm(const std::string& filename, int**& image, int& width, int& height, int& brightness) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) return false;

    std::string format;
    file >> format;
    if (format != "P2") return false;

    while (file.peek() == '#' || file.peek() == '\n' || file.peek() == '\r')
        file.ignore(1000, '\n');

    file >> width >> height >> brightness;

    image = new int*[height];
    for (int i = 0; i < height; i++)
        image[i] = new int[width];

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            file >> image[i][j];

    file.close();
    return true;
}


void save_pgm(const std::string& filename, int** image, int width, int height, int brightness) {
    std::ofstream file(filename.c_str());
    file << "P2\n" << width << " " << height << "\n" << brightness << "\n";
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            file << image[i][j] << " ";
        }
        file << "\n";
    }
    file.close();
}


int main() {
    std::ofstream csv("results.csv");
    csv << "File, Filter, PSNR\n";
    std::cout << "Which 5 files would you like to noise up?\nWrite filenames with .pgm format one by one" << std::endl;
    const int num_files = 5;
    std::string files[num_files];

    for (int i = 0; i < num_files; i++) {
        std::cin >> files[i];
    }

    int filter_sizes[3] = {3, 5, 7};

    srand(time(NULL));
    std::cout << "There will be 3 versions of each picture noised up, so we can compare different median filters" << std::endl;
    for (int f = 0; f < num_files; f++) {
        std::string filename = files[f];
        std::cout << "\nWorking on file: " << filename << std::endl;

        int width = 0, height = 0, brightness = 0;
        int** image = nullptr;

        if (!read_pgm(filename, image, width, height, brightness)) {
            std::cout << "Error occured while processing file " << filename << std::endl;
            std::cout << "Will try with the next one..."<< std::endl;
            continue;
        }

        int** copy = new int*[height];
        int** temp = new int*[height];
        for (int i = 0; i < height; i++) {
            copy[i] = new int[width];
            temp[i] = new int[width];
            for (int j = 0; j < width; j++) {
                copy[i][j] = image[i][j];
            }
        }

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                double prob = (rand() % 1000) / 1000.0;
                if (prob > 0.9) { 
                    copy[i][j] = rand() % brightness;
                }
            }
        }

        save_pgm("noised-up_" + filename, copy, width, height, brightness);

        for (int i = 0; i < 3; i++) {
            int size = filter_sizes[i];
            median(size, height, width, copy, temp);

            std::string new_name = "filtered_" + filename + "__filter_size=" + std::to_string(size);
            save_pgm(new_name, copy, width, height, brightness);

            double similarity = compare(image, copy, height, width);
            std::cout << "Filter " << size << "x" << size << ": similarity = " << similarity << " %" << std::endl;
            csv << filename << "," << size << "x" << size << "," << similarity << " %" << "\n";
        }

        for (int i = 0; i < height; i++) {
            delete[] image[i];
            delete[] copy[i];
            delete[] temp[i];
        }
        delete[] image;
        delete[] copy;
        delete[] temp;
    }

    csv.close();
    std::cout << "\nResults saved in results.csv\n";
    return 0;
}
