#include "sjson_compiler.h"

#include <stdio.h>
#include <assert.h>
#include <cstring>

#define SJSON_LEX_ERROR_HANDLE(error_line)  { printf("Lex error at line %i...\n", error_line); assert(0); }
#define SJSON_PARSE_ERROR_HANDLE(error_line)  { printf("Parse error at line %i...\n", error_line); assert(0); }
#define SJSON_ACCESS_ERROR_HANDLE()  { printf("Error accessing sjson data...\n"); assert(0); }

#include <vector>


struct json_value {
	ValueType value_type;
	union {
		int val_int;
		float val_float;
		char* val_string;
	}value_data;
};

struct table_entry {
	const char* key;
	json_value value;
};
struct json_list {
	std::vector<json_value> _list_entries;
};
struct json_table {
	std::vector<table_entry> _table_entries;
};
struct json_file {
	std::vector<json_list> _lists;
	std::vector<json_table> _tables;
};




//---------------------------------------------------------------------------------------
//-------------------------Lexer Code----------------------------------------------------


static bool is_digit(char ch) {
	if(ch >= '0' && ch <= '9') {
		return true;
	}
	return false;
}
static bool is_char(char ch) {
	if(ch >= 'A' && ch <= 'Z')
		return true;
	if(ch >= 'a' && ch <= 'z')
		return true;
	return false;
}
static bool is_white(char ch) {
	if(ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
		return true;
	}
	return false;
}

class Lexer {
	char* source;
	int p;
	int current_line;

	union {
		int number;
		float real;
	} last_data;
	char* str;
	int str_len;
	int str_pos;

	void append_str(char ch)  {
		str[str_pos] = ch;
		str_pos += 1;
		if(str_pos >= str_len) {
			int new_len = str_len * 2;
			char* new_str = new char[new_len];
			memcpy(new_str, str, str_pos * sizeof(char));
			delete [] str;
			str = new_str;
			str_len = new_len;
		}
		if(ch == 0) {
			str_pos = 0;
		}
	}
	void read_str() {
		do{
			append_str(source[p]);
			++p;
		}while(source[p] != '"' && source[p] != 0);
		append_str(0);
	}
public:
	enum L_TOKEN {
		LT_STRING,
		LT_NUMBER,
		LT_REAL,
		LT_EQUALS,
		LT_IDENT,
		LT_CURLY_OPEN,
		LT_CURLY_CLOSE,
		LT_QUAD_OPEN,
		LT_QUAD_CLOSE,
		LT_END,
		LT_INVALID
	};
	L_TOKEN last_token;
	L_TOKEN current_token;

	Lexer(char* source_code) : source(source_code) {
		p = 0;
		str_pos = 0;
		current_line = 1;
		//32 is the startup buffer size. It does not really matter how much will it be
		//since the data will be different every time and it knows how to grow
		str_len = 32;
		str = new char[str_len];
	}
	~Lexer() {
		delete [] str;
	}


	L_TOKEN NextToken_GRAB() {
		while(is_white(source[p])) {
			if(source[p] == '\n')
				current_line++;
			++p;
		}

		//handle idents
		if(is_char(source[p])) {
			do{
				append_str(source[p]);
				++p;
			}while(is_digit(source[p]) || is_char(source[p]));
			append_str(0);
			return LT_IDENT;
		}

		//handle numbers & floats
		if(is_digit(source[p]) || source[p] == '-') {
			bool negative = false;
			if (source[p] == '-') {
				negative = true;
				++p;
			}
			last_data.number = 0;
			do {
				last_data.number *= 10;
				last_data.number += source[p] - '0';
				++p;
			}while(is_digit(source[p]));

			if(source[p] == '.') {
				++p;
				int second_part = 0;
				int second_part_size = 1;
				while(is_digit(source[p])) {
					second_part *= 10;
					second_part_size *= 10;
					second_part += source[p] - '0';
					++p;
				}
				last_data.real = last_data.number + second_part / (float)second_part_size;
				if (negative) last_data.real = -last_data.real;
				return LT_REAL;
			}
			if (negative) last_data.number = -last_data.number;
			return LT_NUMBER;
		}

		//handle strings
		if(source[p] == '"') {
			++p;
			read_str();
			if(source[p] == '"') {
				++p;
				return LT_STRING;
			} else {
				return LT_INVALID;
			}
		}


		++p;
		switch(source[p-1]) {
		case '=':
			return LT_EQUALS;
		case '[':
			return LT_QUAD_OPEN;
		case ']':
			return LT_QUAD_CLOSE;
		case '{':
			return LT_CURLY_OPEN;
		case '}':
			return LT_CURLY_CLOSE;
		default:
			--p;
		}

		if(source[p] == 0)
			return LT_END;

		return LT_INVALID;
	}
	L_TOKEN NextToken() {
		last_token = current_token;
		current_token = NextToken_GRAB();
		return current_token;
	}
	char* GetLastString() {
		return str;
	}
	int GetLastNumber() {
		return last_data.number;
	}
	float GetLastReal() {
		return last_data.real;
	}
	int GetLine() {
		return current_line;
	}
};



//----------------------------------------------------------------------------
//----------------------SJSON Parser------------------------------------------

static char* duplicate_string(char* str) {
	size_t len = strlen(str) + 1;
	char* new_str = new char[len];
	memcpy(new_str, str, len);
	return new_str;
}


int parse_table_content(json_file* file, Lexer* lexer);
int parse_list_content(json_file* file, Lexer* lexer);
json_value parse_value(json_file* file, Lexer* lexer) {
	json_value val;
	if(lexer->current_token == Lexer::LT_STRING) {
		val.value_type = AT_STRING;
		val.value_data.val_string = duplicate_string(lexer->GetLastString());
	}else if(lexer->current_token == Lexer::LT_NUMBER) {
		val.value_type = AT_INT;
		val.value_data.val_int = lexer->GetLastNumber();
	}else if(lexer->current_token == Lexer::LT_REAL) {
		val.value_type = AT_FLOAT;
		val.value_data.val_float = lexer->GetLastReal();
	}else if(lexer->current_token == Lexer::LT_CURLY_OPEN) {
		val.value_type = AT_TABLE;
		val.value_data.val_int = parse_table_content(file, lexer);
		if(lexer->current_token != Lexer::LT_CURLY_CLOSE) {
			SJSON_PARSE_ERROR_HANDLE(lexer->GetLine());
		}
	}else if(lexer->current_token == Lexer::LT_QUAD_OPEN) {
		val.value_type = AT_LIST;
		val.value_data.val_int = parse_list_content(file, lexer);
		if(lexer->current_token != Lexer::LT_QUAD_CLOSE) {
			SJSON_PARSE_ERROR_HANDLE(lexer->GetLine());
		}
	}else{
		SJSON_PARSE_ERROR_HANDLE(lexer->GetLine());
	}
	return val;
}

int parse_list_content(json_file* file, Lexer* lexer) {
	int id = file->_lists.size();
	file->_lists.resize(file->_lists.size()+1);
	while(true) {
		lexer->NextToken();
		switch(lexer->current_token) {
		case Lexer::LT_NUMBER:
		case Lexer::LT_REAL:
		case Lexer::LT_QUAD_OPEN:
		case Lexer::LT_CURLY_OPEN:
		case Lexer::LT_STRING:
			file->_lists[id]._list_entries.push_back(parse_value(file, lexer));
			break;
		case Lexer::LT_QUAD_CLOSE:
			return id;
		default:
			SJSON_PARSE_ERROR_HANDLE(lexer->GetLine());
			return id;
		}
	}
	return -1;
}

int parse_table_content(json_file* file, Lexer* lexer) {
	int id = file->_tables.size();
	file->_tables.resize(file->_tables.size()+1);
	while(lexer->NextToken() == Lexer::LT_IDENT) {
		int new_size = file->_tables[id]._table_entries.size() + 1;
		file->_tables[id]._table_entries.resize(new_size);

		int entry = file->_tables[id]._table_entries.size() - 1;
		file->_tables[id]._table_entries[entry].key = duplicate_string(lexer->GetLastString());
		if(lexer->NextToken() != Lexer::LT_EQUALS) {
			SJSON_PARSE_ERROR_HANDLE(lexer->GetLine());
		}
		lexer->NextToken();
		file->_tables[id]._table_entries[entry].value = parse_value(file, lexer);
	}
	return id;
};


//----------------------------------------------------------------------------
//----------------------SJSON Interface---------------------------------------



json_file* sjson_compile_source(const char* source_code) {
	json_file* file = new json_file;

	Lexer lexer((char*)source_code);
	if(parse_table_content(file, &lexer) != SJSON_ROOT_TABLE_ID) {
		SJSON_ACCESS_ERROR_HANDLE();
	}

	if(lexer.current_token != Lexer::LT_END) {
		SJSON_ACCESS_ERROR_HANDLE();
	}

	return file;
}
void sjson_free_file(json_file* file) {
	for(size_t i = 0; i < file->_tables.size(); ++i) {
		auto& table = file->_tables[i]._table_entries;
		for(size_t u = 0; u < table.size(); ++u) {
			if(table[u].value.value_type == AT_STRING) {
				delete [] table[u].value.value_data.val_string;
			}
			delete [] table[u].key;
		}
	}
	for(size_t i = 0; i < file->_lists.size(); ++i) {
		auto& list = file->_lists[i]._list_entries;
		for(size_t u = 0; u < list.size(); ++u) {
			if(list[u].value_type == AT_STRING) {
				delete [] list[u].value_data.val_string;
			}
		}
	}
	delete file;
}

int sjson_table_count(json_file* file) {
	return file->_tables.size();
}
int sjson_list_count(json_file* file) {
	return file->_lists.size();
}
int sjson_table_entry_count(json_file* file, int table) {
	return file->_tables[table]._table_entries.size();
}
int sjson_list_entry_count(json_file* file, int list) {
	return file->_lists[list]._list_entries.size();
}
static json_value* sjson_table_search(json_file* file, int table, const char* key) {
	for(size_t i = 0; i < file->_tables[table]._table_entries.size(); ++i) {
		auto* entry = &file->_tables[table]._table_entries[i];
		if(strcmp(entry->key, key) == 0) {
			return &entry->value;
		}
	}
	return 0;
}

ValueType sjson_table_lookup(json_file* file, int table, const char* key) {
	json_value* entry = sjson_table_search(file, table, key);
	if(entry) {
		return entry->value_type;
	}
	return AT_NONE;
}

int sjson_table_int(json_file* file, int table, const char* key) {
	json_value* entry = sjson_table_search(file, table, key);
	if(entry) {
		if(	(entry->value_type == AT_INT) ||
			(entry->value_type == AT_LIST) ||
			(entry->value_type == AT_TABLE)) {
				return entry->value_data.val_int;
		}else if(entry->value_type == AT_FLOAT) {
			return (int)entry->value_data.val_float;
		}
	}
	SJSON_ACCESS_ERROR_HANDLE();
	return 0;
}
float sjson_table_float(json_file* file, int table, const char* key) {
	json_value* entry = sjson_table_search(file, table, key);
	if(entry && entry->value_type == AT_FLOAT) {
		return entry->value_data.val_float;
	}
	SJSON_ACCESS_ERROR_HANDLE();
	return 0;
}
const char* sjson_table_string(json_file* file, int table, const char* key) {
	json_value* entry = sjson_table_search(file, table, key);
	if(entry && entry->value_type == AT_STRING) {
		return entry->value_data.val_string;
	}
	SJSON_ACCESS_ERROR_HANDLE();
	return 0;
}

static json_value* sjson_list_search(json_file* file, int list, int id) {
	if(id >= (int)file->_lists[list]._list_entries.size()) {
		return 0;
	}
	return &file->_lists[list]._list_entries[id];
}
ValueType sjson_list_lookup(json_file* file, int list, int id) {
	if(id >= (int)file->_lists[list]._list_entries.size()) {
		return AT_NONE;
	}
	return file->_lists[list]._list_entries[id].value_type;
}

int sjson_list_int(json_file* file, int list, int id) {
	json_value* entry = sjson_list_search(file, list, id);
	if(entry) {
		if(	(entry->value_type == AT_INT) ||
			(entry->value_type == AT_LIST) ||
			(entry->value_type == AT_TABLE)) {
				return entry->value_data.val_int;
		}else if(entry->value_type == AT_FLOAT) {
			return (int)entry->value_data.val_float;
		}
	}
	SJSON_ACCESS_ERROR_HANDLE();
	return 0;
}
float sjson_list_float(json_file* file, int list, int id) {
	json_value* entry = sjson_list_search(file, list, id);
	if(entry && entry->value_type == AT_FLOAT) {
		return entry->value_data.val_float;
	}
	else if (entry && entry->value_type == AT_INT) {
		return (float)entry->value_data.val_float;
	}
	SJSON_ACCESS_ERROR_HANDLE();
	return 0;
}
const char* sjson_list_string(json_file* file, int list, int id) {
	json_value* entry = sjson_list_search(file, list, id);
	if(entry && entry->value_type == AT_STRING) {
		return entry->value_data.val_string;
	}
	SJSON_ACCESS_ERROR_HANDLE();
	return 0;
}