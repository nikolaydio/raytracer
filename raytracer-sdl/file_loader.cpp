#include "file_loader.h"
#include <fstream>

namespace rt {
	namespace sdl {
		void* FileLoader::load_raw_file(const char* file_name, int* file_size) {
			std::ifstream file;

			for (auto& dir : _lookup_directories) {
				std::string new_name = dir + file_name;
				file = std::ifstream(new_name);
				if (file.is_open()) {
					break;
				}
			}

			if (!file.is_open()) {
				return 0;
			}

			file.seekg(0, std::ios::end);
			*file_size = file.tellg();
			file.seekg(0, std::ios::beg);
			
			char* data = new char[*file_size];
			file.read(data, *file_size);

			file.close();
			return data;
		}
		bool FileLoader::load_string_file(const char* file_name, std::string* str) {
			std::ifstream file;

			for (auto& dir : _lookup_directories) {
				std::string new_name = dir + file_name;
				file = std::ifstream(new_name);
				if (file.is_open()) {
					break;
				}
			}

			if (!file.is_open()) {
				return false;
			}

			file.seekg(0, std::ios::end);
			str->reserve(file.tellg());
			file.seekg(0, std::ios::beg);

			
			str->assign((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

			file.close();
			return true;
		}
		bool FileLoader::file_exists(const char* file_name) {
			std::ifstream file;

			for (auto& dir : _lookup_directories) {
				std::string new_name = dir + file_name;
				file = std::ifstream(new_name);
				if (file.is_open()) {
					break;
				}
			}

			if (!file.is_open()) {
				return false;
			}
			file.close();
			return true;
		}

		void FileLoader::add_directory(const char* directory) {
			_lookup_directories.push_back(directory);
		}
	}
}