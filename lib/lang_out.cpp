#include "conf_lang.h"

// VERSION: 0.3.6 ALPHA
// LAST UPDATE: 23.03.2016

using namespace std;

bool always_read_lang = false;

unsigned int w_length(string line) {
	setlocale(LC_ALL, "");
	wchar_t *w_temp_c = new wchar_t[line.length() + 1];
	mbstowcs(w_temp_c, line.c_str(), line.length() + 1);
	unsigned int num = wcslen(w_temp_c);
	delete [] w_temp_c;
	return num;
}

string del_end(string str_in) // Óäàëåíèå ñèìâîëà êîíöà ñòðîêè
{
	if (str_in.empty()) return "";
	char *c_st = new char[str_in.length()+1];
	strcpy(c_st,str_in.c_str());
	char *p = strtok(c_st, "\r");
	return(string(p));
}

string del_b(string str_in) // Óäàëåíèå ñèìâîëà îòêàòà êàðåòêè
{
	if (str_in.empty()) return "";
	char *c_st = new char[str_in.length()+1];
	strcpy(c_st,str_in.c_str());
	char *p = strtok(c_st, "\b");
	return(p);
}

int del_bl(string str_in) // Óäàëåíèå ñèìâîëà îòêàòà êàðåòêè è âîçâðàùåíèå êîë-âà ñèìâîëîâ â ñòðîêå
{
	if (str_in.empty()) return 0;
	char *c_st = new char[str_in.length()+1];
	strcpy(c_st,str_in.c_str());
	char *p = strtok(c_st, "\b");
	string p_str;
	p_str.clear();
	p_str = string(p);
	return(p_str.length());
}

string del_new(string str_in) {
	if (!strlen(str_in.c_str())) return "";
	char *c_st = new char[str_in.length()+1];
	strcpy(c_st,str_in.c_str());
	char *p = strtok(c_st, "\n");
	return(string(p));
}

string str(double input) {
	stringstream ss;
	ss << "";
	ss << input;
	string str;
	ss >> str;
	return str;
}

bool save_commit_lang(unsigned int first_pos, string line) {
	for (unsigned int j = first_pos; j < line.length(); j++) {
		if ((line[j] == '#') && (line[j - 1] != '\\')) return true;
		if (line[j] != ' ') return false;
	}
	return false;
}

bool search_value(string line, string parametr, string new_value, string &returned_value) {
	bool only_read = true /*Пока ищется параметр*/, continue_stat = false, find_first_char = false, not_found = true;
	string temp_line, temp_return;
	temp_line.clear(); temp_return.clear();
	for (unsigned int i = 0; i < line.length(); i++) {
		if ((only_read) && (!always_read_lang)) { // Чтение, пока не найдено равно или не активно постоянное чтение
			if (line[i] == ' ') continue;
			if (line[i] == '=') {
				if (temp_line == parametr) { // Если найденный параметр совпал с искомым
					not_found = false;
					only_read = false; continue; // Пропуск, чтобы не добавлять равно в ответ
				} else return false;
			}
			temp_line = temp_line + line[i]; // Накопление параметра
		} else {
			if ((!find_first_char) && (line[i] == ' ')) continue; // Если обнаружены пробелы в самом начале
			else find_first_char = true;
			if (continue_stat) {continue_stat = false; continue;} // Если надо что-то пропустить
			if (line[i] == ' ') if (save_commit_lang(i, line)) break; // Если обнаружен пробел и за ним следует комментарий, то закончить
			if (line[i] == '\\') { // Если обнаружен знак экранирования
				if (line[i + 1] == '#') {temp_return = temp_return + line[i + 1]; continue_stat = true; continue;}
				if (line[i + 1] == '\"') {temp_return = temp_return + line[i + 1]; continue_stat = true; continue;}
				if (line[i + 1] == '%') {temp_return = temp_return + line[i + 1]; continue_stat = true; continue;}
				if (line[i + 1] == 'n') {temp_return = temp_return + '\n'; continue_stat = true; continue;}
			}
			if (line[i] == '#') break;
			if (line[i] == '%') {temp_return = temp_return + " "; continue;}
			// if (line[i] == 'n') {temp_return = temp_return + '\n'; continue;}
			if (line[i] == '\"') { // Если обнаружены кавычки
				if (always_read_lang) { // Если это вторые кавычки
					always_read_lang = false;
					returned_value = temp_return;
					return true;
				} else {
					always_read_lang = true;
					continue;
				}
			}
			temp_return = temp_return + line[i]; // Накопление результата
		}
	}
	returned_value = temp_return;
	if (not_found) return false; // Если искомый параметр не был найден
	return true;
}

string lang(string parametr, vector <string> lang_base) {
	always_read_lang = false;
	string readText, returned_value, always_read_temp;
	returned_value.clear(); always_read_temp.clear();
	bool found_result = false, always_read_bool = false;
	for (unsigned int i = 0; i < lang_base.size(); i++) {
		if (always_read_lang) always_read_bool = true;
		else always_read_bool = false;
		if ((search_value(lang_base[i], parametr, "", returned_value)) && (!always_read_lang)) {
			found_result = true;
			if (always_read_bool) return always_read_temp + returned_value;
			else return returned_value;
		} else if (always_read_lang) always_read_temp = always_read_temp + returned_value;
	}
	if (!found_result) return "NotLoadLang"; // Параметр не найден
	else return returned_value;
}