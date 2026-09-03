#include <iostream>
#include <iomanip>
#include <FileScannerLib.h>
#include <chrono>
#include <Windows.h>

int main(int argc, char* argv[]) {
	auto start = std::chrono::steady_clock::now();
	//setlocale(LC_ALL, "ru_RU.UTF-8");
	setlocale(LC_ALL, "");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	fs::path path, base, log;

	if (argc == 4) {
		path = argv[1];
		base = argv[2];
		log = argv[3];
	}
	else if (argc == 7	) {
		for (int i = 1; i <= 6; i += 2) {
			if (!strcmp( argv[i],"--base")) {
				base = argv[i + 1];
			}
			else if (!strcmp(argv[i], "--log")) {
				log = argv[i + 1];
			}
			else if (!strcmp(argv[i], "--path")) {
				path = argv[i + 1];
			}
		}
	}

	std::cout <<"Base: " << base.string() << "\n" 
		<< "Path: " << path.string() << "\n"
		<< "Log: " << log.string() << "\n";

	if (path.empty() || base.empty() || log.empty()) {
		std::cout << "Wrong input\n";
		return -1;
	}

	try {
		FileRunner f(path, base, log);
		ThreadPool tp(std::thread::hardware_concurrency());
		f.run(tp);
	}
	catch(std::exception& e){
		std::cout << "\n" << e.what() << "\n";
	}

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = end - start;

	std::cout<< "Время выполнения: " << elapsed.count() << " секунд\n";

	return  0;
}