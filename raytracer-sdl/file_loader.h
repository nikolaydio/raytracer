#pragma once

#include <vector>
#include <string>

namespace rt {
	namespace sdl {
		class FileLoader {
			std::vector<std::string> _lookup_directories;
		public:
			void* load_raw_file(const char* file_name, int* file_size);
			bool load_string_file(const char* file_name, std::string* str);

			bool file_exists(const char* file_name);

			void add_directory(const char* directory);
		};
	}
}