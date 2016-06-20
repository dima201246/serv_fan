/*
	VERSION: 2.0
	
	:DVcompany 2016
*/
#ifndef CONF_LANG_H
#define CONF_LANG_H
	#include <locale.h>
	#include <fstream>
	#include <vector>
	#include <sstream>
	#include <cstdlib>
	#include <string>
	#include <string.h>
	#include <stdio.h>
	
	/*Lang*/
	unsigned int w_length(std::string); // Длина строки со 2 битовыми знаками
	std::string lang(std::string/*Искомый параметр*/, std::vector<std::string>/*Вектор со словарём*/); // Возврат строки из словаря
	std::string str(double); // Перевод из числа в строку
	std::string del_end(std::string); // Удаление "\r"
	std::string del_b(std::string); // Удаление "\b"
	int del_bl(std::string); // Длина строки без "\b"
	std::string del_new(std::string); // Удаление знака конца строки

	/*Configurator*/
	std::string cl_version(); // Версия библиотеки
	std::string configurator(std::string /*Путь к файлу*/, std::string /*Искомый параметр*/, std::string /*Новое значение, если нужна перезапись*/, bool /*Перезапись значения или нет*/); // Поиск в файле нужного параметра и возврат его значения
	void add_to_file(std::string /*Путь к файлу*/, std::string /*Строка, которая будет добавлена*/); // Запись в конец файла
	std::string conf(std::string /*Искомый параметр*/, std::vector<std::string> /*вектор со значениями*/); //Поиск в векторе нужного параметра и возврат его значения
	bool load_to_vector(std::string /*Имя файла*/, std::vector<std::string>&  /*Ссылка на вектор*/); // Загрузка файла в вектор
#endif
