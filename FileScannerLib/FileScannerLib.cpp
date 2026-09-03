#include "FileScannerLib.h"

std::string md5hash(fs::path path) {
    std::ifstream file;
    file.open(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Не удалось открыть файл");
    }

    MD5 md5;
    std::vector<unsigned char> buffer(4096);

    while (file.good()) {
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0)
            md5.add(buffer.data(), static_cast<size_t>(bytesRead));
    }

    file.close();

    return md5.getHash();
}

void WaitGroup::add(int n) {
    std::unique_lock<std::mutex> lock(mutex);
    count += n;
}

void WaitGroup::done() {
    std::unique_lock<std::mutex> lock(mutex);
    if (--count == 0) {
        cv.notify_all();
    }
}

void WaitGroup::wait() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return count == 0; });
}

    void ThreadPool::worker() {
        std::unordered_map<std::string, OUTPUT_TYPE> ans;

        long long local_error_counter = 0;


        while (true) {
            fs::path cur;
            {
                std::unique_lock<std::mutex> lock(mu);
                cv.wait(lock, [this] { return !q.empty() || !isRunning; });

                if (!isRunning && q.empty()) {
                    break;
                }
                if (q.empty()) {
                    continue;
                }
                else {
                    try {
                        cur = std::move(q.front());
                    }
                    catch (std::exception& e) {
                        error_counter++;
                    }
                    q.pop();
                }
            }
            if (!cur.empty()) {
                try {
                    ans[md5hash(cur)] = cur.wstring();
                }
                catch (std::runtime_error& e) {
                    local_error_counter++;
                }
                catch (std::exception& e) {
                    local_error_counter++;
                }
                catch (...) {
                    local_error_counter++;
                }
            }
        }

        std::unique_lock<std::mutex> lock(mu);
        error_counter += local_error_counter;
        fullAns.insert(ans.begin(), ans.end());
    }

    ThreadPool::ThreadPool(int count) {
        threadCount = count;
        isRunning = 0;
        th.resize(count);
        q;
    }

    void ThreadPool::post(std::filesystem::path p) {
        try {
            {
                std::lock_guard<std::mutex> lock(mu);
                q.push(std::move(p));
            }
            cv.notify_one();
        }
        catch (std::exception& e) {
            error_counter++;
        }
    };

    void ThreadPool::run() {
        std::cout << "File scanning....\n";
        isRunning = 1;
        for (int i = 0; i < threadCount; i++) {
            wg.add(1);
            std::thread([this]() {
                    this->worker();
                    this->wg.done();
                }).detach();
        }
    }

    std::unordered_map<std::string, OUTPUT_TYPE> ThreadPool::stop() {
        std::cout << "File processing....\n";
        isRunning = false;
        cv.notify_all();
        wg.wait();
        return fullAns;
    }

    Logger::Logger(fs::path p, fs::path lp) {
        path = fs::path(p);

        std::ifstream f;
        std::string tmp;

        f.open(path, std::ios::binary);
        if(!f.is_open())
            throw std::exception("Base file was not found\n");
        
        while (f.good()) {
            std::getline(f, tmp);
            if (tmp.size() > 33)
                m[tmp.substr(0, 32)] = tmp.substr(33);
            else
                throw std::exception("Wrong base format\n");
        }

        logPas.open(lp);
        logPas.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    }

    void Logger::log(std::pair<std::string, OUTPUT_TYPE> p) {
        if (m.count(p.first)) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring md = converter.from_bytes(p.first.c_str());
            std::wstring verdict = converter.from_bytes(m[p.first].c_str());


            logPas << p.second << L";" << md << L";" << verdict << '\n';
        }
    }

    FileRunner::FileRunner(fs::path p, fs::path base, fs::path log) {
        path = p;
        l = Logger(base, log);
    }

    void FileRunner::run(ThreadPool& tp) {
        auto p = fs::directory_entry(path);

        q.push(p);
        tp.run();
        long long local_error_counter = 0;

        while (!q.empty()) {
            auto cur = q.front();
            q.pop();
            try {
                for (auto& tmp : fs::directory_iterator(cur, fs::directory_options::skip_permission_denied)) {

                    tmp.path();
                    if (tmp.exists() && tmp.is_regular_file() && !tmp.is_directory())
                        tp.post(tmp.path());
                    else if (tmp.is_directory()) {
                        q.push(tmp);
                    }
                }
            }
            catch (std::exception& e) {
                //std::cout << e.what() << "\n";
                local_error_counter++;
            }
        }

        auto ans = tp.stop();
        for (const auto& i : ans) {
            l.log(i);
        }

        error_counter += local_error_counter;
        std::cout<<"Was scanned: " << ans.size() <<" files\n"<<"Was found "<< error_counter <<" errors\n";
    }