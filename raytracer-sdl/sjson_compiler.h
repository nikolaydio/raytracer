#pragma once

// Implementation of a simplified json compiler. See sample.txt to see its syntax.
// Source is divided in 2 files
// 1. C like interface file - header(sjson_compiler.h)
// 2. C++ Implementation(sjson_compiler.cpp)

// In the cpp file you will find 3 defines that can help you inject error handling based on your needs.
// Default behaviour on error is assert(0);

#define SJSON_ROOT_TABLE_ID 0

struct json_file;
json_file* sjson_compile_source(const char* source_code);
void sjson_free_file(json_file* file);

int sjson_table_count(json_file* file);
int sjson_list_count(json_file* file);
int sjson_table_entry_count(json_file* file, int table);
int sjson_list_entry_count(json_file* file, int list);

typedef enum _VALUE_TYPE {
	AT_NONE,
	AT_INT,
	AT_FLOAT,
	AT_STRING,
	AT_TABLE,
	AT_LIST
}ValueType;

ValueType sjson_table_lookup(json_file* file, int table, const char* key);

int sjson_table_int(json_file* file, int table, const char* key);
float sjson_table_float(json_file* file, int table, const char* key);
const char* sjson_table_string(json_file* file, int table, const char* key);


ValueType sjson_list_lookup(json_file* file, int list, int id);

int sjson_list_int(json_file* file, int list, int id);
float sjson_list_float(json_file* file, int list, int id);
const char* sjson_list_string(json_file* file, int list, int id);
