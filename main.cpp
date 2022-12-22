#include <iostream>
#include <iomanip>
#include <vector>
#include <future>

std::chrono::high_resolution_clock::time_point timeNow()
{
    return std::chrono::high_resolution_clock::now();
}
double timeSpent(std::chrono::high_resolution_clock::time_point timeStart)
{
    std::chrono::duration<double, std::milli> duration_ms = timeNow() - timeStart;
    return duration_ms.count();
}


// Получаем число одновременно работающих потоков.
unsigned int getKernelsCount()
{
    unsigned int kernels_count = std::thread::hardware_concurrency();
    if (kernels_count < 1)
    {
        kernels_count = 1;
        std::cout << "Невозможно определить число ядер. Принимаем за 1." << std::endl;
    }
    std::cout << "Число ядер: " << kernels_count << std::endl;
    return kernels_count;
}


void generateRandomData(int* arr, int dataLength)
{
    for (int i = 0; i < dataLength; i++)
        arr[i] = rand() % dataLength;
}


// Исходная функция слияния из модуля 11.4
void merge(int* arr, int begin, int middle, int end)
{
    int nl = middle - begin + 1;
    int nr = end - middle;

    // создаем временные массивы
    int* left = new int[nl];
    int* right = new int[nr];

    // копируем данные во временные массивы
    for (int i = 0; i < nl; i++)
        left[i] = arr[begin + i];
    for (int j = 0; j < nr; j++)
        right[j] = arr[middle + 1 + j];

    int i = 0, j = 0;
    int k = begin;  // начало левой части

    while (i < nl && j < nr) {
        // записываем минимальные элементы обратно во входной массив
        if (left[i] <= right[j]) {
            arr[k] = left[i];
            i++;
        }
        else {
            arr[k] = right[j];
            j++;
        }
        k++;
    }
    // записываем оставшиеся элементы левой части
    while (i < nl) {
        arr[k] = left[i];
        i++;
        k++;
    }
    // записываем оставшиеся элементы правой части
    while (j < nr) {
        arr[k] = right[j];
        j++;
        k++;
    }
    delete[] left;
    delete[] right;
}


// Исходная функция разбиения из модуля 11.4
void merge(int* arr, int start, int stop)
{
    if (start >= stop)
        return;

    int middle = (start + stop) / 2;
    merge(arr, start, middle);
    merge(arr, middle + 1, stop);
    merge(arr, start, middle, stop);
}


// Функция слияния кусков массива, отсортированных разными потоками
void finalMerge(int* arr, const std::vector<int>& stopIndexes)
{
    int i, j, k, leftBegin, leftEnd, leftLength, rightBegin, rightEnd, rightLength;

    for (int threadsNum = 1; threadsNum < stopIndexes.size(); ++threadsNum)
    { 
        leftBegin = 0;
        leftEnd = stopIndexes[threadsNum - 1];
        leftLength = leftEnd - leftBegin + 1;
        rightBegin = leftEnd + 1;
        rightEnd = stopIndexes[threadsNum];
        rightLength = rightEnd - rightBegin + 1;

        int* left = new int[leftLength];
        int* right = new int[rightLength];
      
        for (int i = 0; i < leftLength; i++)
            left[i] = arr[i + leftBegin];
        for (int j = 0; j < rightLength; j++)
            right[j] = arr[j + rightBegin];

        k = 0;
        i = 0;
        j = 0;
        while ((i < leftLength) && (j < rightLength))
        {
            if (left[i] <= right[j]) {
                arr[k] = left[i];
                i++;
            }
            else {
                arr[k] = right[j];
                j++;
            }
            k++;
        }
        while (i < leftLength) {
            arr[k] = left[i];
            i++;
            k++;
        }
        while (j < rightLength) {
            arr[k] = right[j];
            j++;
            k++;
        }
        delete[] left;
        delete[] right;
    }

}


// функция разбиения массива на части для вызова отдельных потоков
void mergeParallel(int* arr, int length, int threadsNum)
{
    int lengthToProcess = length;
    int lengthInThread;
    int start = 0, stop;
    std::vector<int> stopIndexes; // Для хранения границ подмассивов
    std::vector<std::future<void>> futures; // Для получения уведомлений о завершении функции и получения результата
    for (int i = 0; i < threadsNum; ++i)
    {
        // Разбиение исходного массива на части одинаковой длины (+/-1)
        lengthInThread = lengthToProcess / (threadsNum - i);
        lengthToProcess = lengthToProcess - lengthInThread;
        stop = start + lengthInThread - 1;
        auto handle = [](int* arr, const int& start, const int& stop)
            { merge(arr, start, stop); }; // Лямбда-функция для выполнения потоками
        std::packaged_task<void(int*, const int&, const int&)> task1(handle); // Задача для получения future
        futures.push_back(std::move(task1.get_future()));
        std::thread thread(std::move(task1), arr, start, stop); // Запускаем потоки
        thread.detach();

        stopIndexes.push_back(stop);
        start = stop + 1;
    }
    for (int i = 0; i < threadsNum; ++i)
        futures[i].get();	// Дожидаемся завершения всех потоков
    finalMerge(arr, stopIndexes); // Склеиваем куски массива
}


int main()
{
	setlocale(LC_ALL, "");
    unsigned int threadsCount = getKernelsCount();

    double tempTime = 0.0;
    std::chrono::high_resolution_clock::time_point startTime;
    srand(0);

    int dataLength = 10000000; // Размер массива

    int* arr = new int[dataLength];
    generateRandomData(arr, dataLength);

    std::cout << "==== Обработка в однопоточном режиме ====" << std::endl;
    startTime = timeNow();
    merge(arr, 0, dataLength - 1);
    tempTime = timeSpent(startTime);
    std::cout << "Сортировка заняла: " << std::setprecision(3) << tempTime / 1000 << " с" << std::endl;

    generateRandomData(arr, dataLength);

    std::cout << "==== Обработка в многопоточном режиме ====" << std::endl;
    startTime = timeNow();
    mergeParallel(arr, dataLength, threadsCount);
    tempTime = timeSpent(startTime);
    std::cout << "Сортировка заняла: " << std::setprecision(3) << tempTime / 1000 << " с" << std::endl;


    // Проверка правильности сортировки
    bool wrongFlag = false;
    for (int i = 0; i < dataLength - 1; ++i)
        if (arr[i] > arr[i + 1])
        {
            wrongFlag = true;
            break;
        }
    if (wrongFlag)
        std::cout << "Ошибка сортировки!" << std::endl;
    else
        std::cout << "Сортировка выполнена успешно." << std::endl;

    if (dataLength <= 500) 
    {
        for (int i = 0; i < dataLength; ++i)
            std::cout << arr[i] << " ";
        std::cout << std::endl;
    }

    delete[] arr;
	return 0;
}
