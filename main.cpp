#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <cstdlib>
#include <chrono>
#include <algorithm>

using namespace std;

#define PART_SIZE 500000
#define RAND_ARR_SIZE 2'000'000 

void swap(int* a, int* b) 
{
    int t = *a; 
    *a = *b; 
    *b = t; 
} 

int partition(int*& arr, int low, int high) 
{ 
    int pivot = arr[high];
    int i = (low - 1);
  
    for (int j = low; j <= high - 1; j++) 
    { 
        if (arr[j] <= pivot)
        { 
            i++;
            swap(&arr[i], &arr[j]);
        }
    } 
    swap(&arr[i + 1], &arr[high]); 
    return (i + 1); 
}

void quickSort(int*& arr, int low, int high) 
{ 
    if (low < high) 
    { 
        int pi = partition(arr, low, high); 
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    } 
}

void merge_to_file(const int* arr1, const int* arr2, int sz1, int sz2)
{

    fstream temp;
    const int* first;
    const int* second;

    temp.open("temp_1_file.txt", fstream::out | std::ofstream::trunc);

    if (arr1[0] < arr2[0]) {
        first = arr1;
        second = arr2;
    }else {
        first = arr2;
        second = arr1;
        swap(&sz1, &sz2); // becouse swap arrays;
    }
    
    if (temp.is_open())
    {
        int i = 0;
        int j = 0;
        
        while (i < sz1 && j < sz2){
            if (first[i] < second[j])
                temp << first[i++] << ' ';
            else if (first[i] == second[j])
            {
                temp << first[i++] << ' ';
                temp << second[j++] << ' ';
            }
            else
                temp << second[j++] << ' ';
        }

        while (i < sz1)
            temp << first[i++] << ' ';

        while (j < sz2)
            temp << second[j++] << ' ';

       temp.close();
    }
}

void merge_files()
{
    fstream res;
    fstream temp1;
    fstream temp2;

    temp1.open("temp_1_file.txt", fstream::in);
    res.open("res_file.txt", fstream::in);
    temp2.open("temp_2_file.txt", fstream::out | ofstream::trunc); // open and clear

    if (!temp1.is_open() || !temp2.is_open() || !res.is_open())
        return;

    int temp1_value;
    int res_value;
        
    temp1 >> temp1_value;
    res >> res_value;
    while (!temp1.eof() && !res.eof()) {
        if (temp1_value <= res_value) {
            temp2 << temp1_value << ' ';
            temp1 >> temp1_value;
        }
        else {
            temp2 << res_value << ' ';
            res >> res_value;
        }
    }

    while (!res.eof()) {
        temp2 << res_value << ' ';
        res >> res_value;
    }

    while (!temp1.eof()) {
        temp2 << temp1_value << ' ';
        temp1 >> temp1_value;
    }

    temp1.close();
    temp2.close();
    res.close();
    
    // delete content of file
    res.open("res_file.txt", std::ofstream::out | std::ofstream::trunc);
    if (res.is_open())
        res.close();

    // copy result to result file
    if (!filesystem::copy_file("temp_2_file.txt", "res_file.txt",
        filesystem::copy_options::overwrite_existing))
        return;
}

// return the size of readed part
int read_part_arr(fstream& fs, int*& arr)
{
    arr = new int[PART_SIZE];
    int* tmp_arr;
    int i;
    for (i = 0; i < PART_SIZE && !fs.eof(); i++)
        fs >> arr[i];

    if (i == 1){
        delete[] arr;
        return 0;
    }

    if (i != PART_SIZE){
        tmp_arr = new int[i];
        for (size_t j = 0; j < i; j++)
            tmp_arr[j] = arr[j];

        delete[] arr;
        arr = tmp_arr;   
        return i - 1;
    }

    return PART_SIZE;
}

void sort_func(const string& filename){
    
    fstream fs;
    fs.open(filename, fstream::in);

    if(fs.is_open())
    {
        while (!fs.eof())
        {
            int* part_1;
            int* part_2;

            int size_1 = read_part_arr(fs, part_1);
            int size_2 = read_part_arr(fs, part_2);
            if (size_1 == 0 || size_2 == 0)
                return;
            cout << " size_1 = " << size_1 << " size_2 = " << size_2 << endl;
            quickSort(part_1, 0, size_1 - 1);
            quickSort(part_2, 0, size_2 - 1);          
            merge_to_file(part_1, part_2, size_1, size_2);
            merge_files();
        }
        fs.close();
    }   
}

void write_rand_arr(const string& filename)
{
    fstream fs;

    srand(time(nullptr));
    int lef_border = -100;
    int range_len = 50000; // правая граница = range_len + left_border

    fs.open(filename, fstream::out | ofstream::trunc);
    if(fs.is_open())
    {
       for (int i = 0; i < RAND_ARR_SIZE; i++)
           fs << (lef_border + rand() % range_len) << ' ';

       fs.close();
    }
}


int main(int argc, char const *argv[])
{

    string filename = "array_data.txt";

    write_rand_arr(filename);
    cout << "Genetrating data is done!" << endl;

    fstream res;
    res.open("res_file.txt", fstream::out | ofstream::trunc);
    res.close();

    auto start = chrono::high_resolution_clock::now();
    sort_func(filename);
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    cout << "Elapsed time: " << elapsed.count() << " sec" << endl;

    return 0;
}
