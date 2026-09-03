#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <queue>
#include <unordered_map>
#include <md5.h>
#include <locale>
#include <codecvt>
#include <iostream>

#define OUTPUT_TYPE std::wstring

namespace fs = std::filesystem;

static long long error_counter = 0;

std::string md5hash(fs::path path);

class WaitGroup {
private:
    std::mutex mutex;
    std::condition_variable cv;
    int count = 0;

public:
    void add(int n);

    void done();

    void wait();
};

class ThreadPool {
public:
    std::vector<std::thread> th;
    std::queue<fs::path> q;

    int threadCount = 4;
    std::atomic<bool> isRunning = 0;
    std::unordered_map<std::string, OUTPUT_TYPE> fullAns;

    std::condition_variable cv;
    std::mutex mu;
    WaitGroup wg;
public:
    void worker();
public:
    ThreadPool(int count);

    void post(std::filesystem::path p);

    void run();

    std::unordered_map<std::string, OUTPUT_TYPE> stop();
};

class Logger {
public:
    fs::path path;
    std::unordered_map<std::string, std::string> m;
    std::wofstream logPas;

    Logger() = default;

    Logger(fs::path p, fs::path lp);

    void log(std::pair<std::string, OUTPUT_TYPE> p);
};

class FileRunner {
public:
    fs::path path;
    std::queue<std::filesystem::directory_entry> q;
    Logger l;

    FileRunner(fs::path p, fs::path base, fs::path log);

    void run(ThreadPool& tp);
};
