#include "lib/conf_lang.h"
#include "header/port_control.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <wait.h>
#include <errno.h>
#include <string.h>

#define CONF_FILE "/etc/serv_fan.conf"
#define PID_FILE "/var/run/serv_fan.pid"

#define MAX_ITERATION 100000

//sudo ps | grep "sensors <defunct>" | sudo kill -9 $(cut -d " " -f1)

using namespace std;

const string SF_VERSION = _sf_version;

struct local_time {
	int hours, min, sec, mon, day, year;
};

int		led_port,
		port_rear_fan,
		port_front_fan,
		port_cpu_fan,
		port_ps_fan,
		max_cpu_temp_alarm,
		max_cpu_temp,
		min_cpu_temp,
		system_mode;

unsigned int	scan_delay,
				count_max_temp,
				temperature_monitoring,
				blow_delay,
				blow_duration;

local_time	time_force_on,
			time_force_off;

string LOG_FILE;

vector <string> conf_base;

void alarm_sound() {
	for (int i	= 0; i < 4; i++, system("beep -f 500 -l 500"), sleep(1));
}

local_time get_time_now() {
	local_time time_temp;
	time_t t;
	t = time(NULL);
	struct tm* now = localtime(&t);

	time_temp.hours = now->tm_hour;
	time_temp.min = now->tm_min;
	time_temp.sec = now->tm_sec;
	time_temp.mon = now->tm_mon + 1;
	time_temp.day = now->tm_mday;
	time_temp.year = now->tm_year + 1900;
	return time_temp;
}

int get_temperature_now() {
	int				pipefd[2],
					temper;

	char			buf[512];

	unsigned int	count;

	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	dup2(pipefd[0],STDIN_FILENO);

	pid_t cpid	= fork();

	if(cpid == 0) {// Дочерний
		close(pipefd[0]);
		dup2(pipefd[1],STDOUT_FILENO);
		execlp("sensors","",NULL);
		_exit(0);
	} else
		close(pipefd[1]);

	waitpid(cpid,NULL,WUNTRACED);

	count	= 0;

	do {
		scanf("%s",buf);
		if (count >= MAX_ITERATION) {
			strcpy(buf, "+0.0");
			break;
		}
		if ((buf[0] == 'C') && (buf[1] == 'o') && (buf[2] == 'r') && (buf[3] == 'e')) {
			scanf("%s",buf);
			scanf("%s",buf);
			break;
		}
		count++;
	} while (true);

	sscanf("+%s.*", buf);
	temper	= atoi(buf);

	fflush(stdout);

	return temper;
}

bool InitWorkValues() {
	string	temp_v;

	temp_v	= conf("led_port", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found led_port!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] led_port -		OK! - " + temp_v);
		led_port = atoi(temp_v.c_str());
	}

	temp_v	= conf("port_rear_fan", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found port_rear_fan!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] port_rear_fan -	OK! - " + temp_v);
		port_rear_fan = atoi(temp_v.c_str());
	}

	temp_v	= conf("port_front_fan", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found port_front_fan!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] port_front_fan -	OK! - " + temp_v);
		port_front_fan = atoi(temp_v.c_str());
	}

	temp_v	= conf("port_cpu_fan", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found port_cpu_fan!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] port_cpu_fan -		OK! - " + temp_v);
		port_cpu_fan = atoi(temp_v.c_str());
	}

	temp_v	= conf("port_ps_fan", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found port_ps_fan!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] port_ps_fan -		OK! - " + temp_v);
		port_ps_fan = atoi(temp_v.c_str());
	}

	temp_v	= conf("time_force_on_h", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found time_force_on_h!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] time_force_on_h -	OK! - " + temp_v);
		time_force_on.hours = atoi(temp_v.c_str());
	}

	temp_v	= conf("time_force_on_m", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found time_force_on_m!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] time_force_on_m -	OK! - " + temp_v);
		time_force_on.min = atoi(temp_v.c_str());
	}

	temp_v	= conf("time_force_off_h", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found time_force_off_h!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] time_force_off_h -	OK! - " + temp_v);
		time_force_off.hours = atoi(temp_v.c_str());
	}

	temp_v	= conf("time_force_off_m", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found time_force_off_m!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] time_force_off_m -	OK! - " + temp_v);
		time_force_off.min = atoi(temp_v.c_str());
	}

	temp_v	= conf("scan_delay", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found scan_delay!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] scan_delay -		OK! - " + temp_v);
		scan_delay = atoi(temp_v.c_str());
	}

	temp_v	= conf("max_cpu_temp_alarm", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found max_cpu_temp_alarm!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] max_cpu_temp_alarm -	OK! - " + temp_v);
		max_cpu_temp_alarm = atoi(temp_v.c_str());
	}

	temp_v	= conf("count_max_temp", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found count_max_temp!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] count_max_temp -	OK! - " + temp_v);
		count_max_temp = atoi(temp_v.c_str());
	}

	temp_v	= conf("max_cpu_temp", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found max_cpu_temp!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] max_cpu_temp -		OK! - " + temp_v);
		max_cpu_temp = atoi(temp_v.c_str());
	}

	temp_v	= conf("min_cpu_temp", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found min_cpu_temp!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] min_cpu_temp -		OK! - " + temp_v);
		min_cpu_temp = atoi(temp_v.c_str());
	}

	temp_v	= conf("temperature_monitoring", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found temperature_monitoring!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] temperature_monitoring-OK! - " + temp_v);
		temperature_monitoring = atoi(temp_v.c_str());
	}

	temp_v	= conf("blow_delay", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found blow_delay!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] blow_delay -		OK! - " + temp_v);
		blow_delay = atoi(temp_v.c_str());
	}

	temp_v	= conf("blow_duration", conf_base);
	if (temp_v == "0x1") {
		add_to_file(LOG_FILE, "[CONFIG] Not found blow_duration!");
		return false;
	} else {
		add_to_file(LOG_FILE, "[CONFIG] blow_duration -	OK! - " + temp_v);
		blow_duration = atoi(temp_v.c_str());
	}
	
	add_to_file(LOG_FILE, "[CONFIG] All values loaded!!!\n");

	system("beep -f 200 -l 500");
	system("beep -f 400 -l 500");
	
	return true;
}

int TempWatch() {
	int				cycle,
					temp_now;

	unsigned int	scan_delay_count,
					temperature_monitoring_count,
					blow_delay_count,
					blow_duration_count,
					count_max_temp_count;

	local_time		t_n;

	if (!InitWorkValues()) {
		alarm_sound();
		local_time	t_n;
		add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer... " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
		unlink(PID_FILE);
		system("reboot");
		exit(-1);
	}

	cycle							= 1;
	system_mode						= 0;
	scan_delay_count				= scan_delay; // Задержка на считывание состояния системы
	temperature_monitoring_count 	= temperature_monitoring; // Задержка на считывание температуры в лог
	blow_delay_count				= 0; // Задержка продувки
	blow_duration_count				= 0; // Сколько секунд будет происходить продувка
	count_max_temp_count			= 0; // Задержка высокой температуры
	t_n								= get_time_now(); // Получение текущего времени

	add_to_file(LOG_FILE, "[DAEMON] Daemon successfully launched! " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));

	/*system_mode
	0 - Ожидание счётчика
	1 - Продувка
	2 - Очень высокая температура
	3 - Высокая температура
	4 - Полная продувка
	*/

	while (cycle) {

		if ((system_mode == 1) && (blow_duration_count < blow_duration)) { // Продувка
			blow_duration_count++;
		} else if ((system_mode == 1) && (blow_duration_count >= blow_duration)) {
			t_n			= get_time_now(); // Получение текущего времени
			temp_now	= get_temperature_now(); // Получение текущей температуры
			add_to_file(LOG_FILE, "[DAEMON] Blow finished at " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
			add_to_file(LOG_FILE, "[DAEMON] Temperature now: " + str(temp_now) + "C");
			
			blow_duration_count = 0;
			system_mode			= 0;

			if ((!pin_write(port_rear_fan, LOG_FILE)) || (!pin_write(port_front_fan, LOG_FILE))) { // Отключение продувки
				add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
				unlink(PID_FILE);
				system("shutdown -h now");
				exit(-1);
			}

		}

		
		if (scan_delay_count < scan_delay) { // Ожидание
			sleep(1);
			pin_write(led_port, LOG_FILE);
			scan_delay_count++;
			continue;
		} else {
			scan_delay_count	= 0;
		}

		t_n			= get_time_now(); // Получение текущего времени
		temp_now	= get_temperature_now(); // Получение текущей температуры

		/*Логирование температуры начало*/
		if (temperature_monitoring_count < temperature_monitoring) { // Вывод температуры в лог
			temperature_monitoring_count++;
		} else {
			temperature_monitoring_count	= 0;
			add_to_file(LOG_FILE, "[DAEMON] Temperature now: " + str(temp_now) + "C " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
		}
		/*Логирование температуры конец*/
		
		/*Очень высокая температура начало*/
		if ((system_mode != 2) && (temp_now >= max_cpu_temp_alarm)) { // Если зафиксирована очень высокая температура

			add_to_file(LOG_FILE, "[DAEMON] [!!!ALARM!!!] Alarm temp - " + str(temp_now) + "C! " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));

			switch (system_mode) {
				case 0:	if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_ps_fan, LOG_FILE)) || (!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
							add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
							unlink(PID_FILE);
							system("shutdown -h now");
							exit(-1);
						}
						break;

				case 1:	if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_ps_fan, LOG_FILE))) {
							add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
							unlink(PID_FILE);
							system("shutdown -h now");
							exit(-1);
						}
						break;

				case 3:	if ((!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_ps_fan, LOG_FILE))) {
							add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
							unlink(PID_FILE);
							system("shutdown -h now");
							exit(-1);
						}
						break;
			}

			system_mode				= 2;
			count_max_temp_count	= 0;

			continue;
		}

		if ((system_mode == 2) && (temp_now >= max_cpu_temp_alarm) && (count_max_temp_count < count_max_temp)) {
			count_max_temp_count++;
		} else if ((system_mode == 2) && (temp_now >= max_cpu_temp_alarm) && (count_max_temp_count >= count_max_temp)) { // Не смог выдержать очень высокую температуру
			add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer... So hot! " + str(temp_now) + "C! " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
			unlink(PID_FILE);
			system("shutdown -h now");
			exit(-1);
		}
		/*Очень высокая температура конец*/
		
		/*Высокая температура начало*/
		if ((system_mode < 2) && (temp_now >= max_cpu_temp)) {

			add_to_file(LOG_FILE, "[DAEMON] [WARNING] High temp - " + str(temp_now) + "C! " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));

			switch (system_mode) {
				case 0:	if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
							add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
							unlink(PID_FILE);
							system("shutdown -h now");
							exit(-1);
						}
						break;

				case 1:	if ((!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_cpu_fan, LOG_FILE))) {
							add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
							unlink(PID_FILE);
							system("shutdown -h now");
							exit(-1);
						}
						break;
			}

			system_mode				= 3;
			count_max_temp_count	= 0;

			continue;
		}

		if ((system_mode == 3) && (temp_now >= max_cpu_temp) && (count_max_temp_count < count_max_temp)) {
			count_max_temp_count++;
		} else if ((system_mode == 3) && (temp_now >= max_cpu_temp_alarm) && (count_max_temp_count >= count_max_temp)) { // Не смог выдержать высокую температуру
			count_max_temp_count	= 0;
			system_mode				= 2;
			add_to_file(LOG_FILE, "[DAEMON] [!!!ALARM!!!] So hot!!! " + str(temp_now) + "C! " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
			if ((!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_ps_fan, LOG_FILE))) {
				add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
				unlink(PID_FILE);
				system("shutdown -h now");
				exit(-1);
			}
		}
		/*Высокая температура конец*/
		
		/*Полная продувка начало*/
		/*Полная продувка конец*/

		/*Продувка начало*/
		if ((system_mode == 0) && (blow_delay_count < blow_delay)) {
			blow_delay_count++;
		} else if ((system_mode == 0) && (blow_delay_count >= blow_delay)) {
			add_to_file(LOG_FILE, "[DAEMON] Blow started at " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
			add_to_file(LOG_FILE, "[DAEMON] Temperature now: " + str(temp_now) + "C");
			
			system_mode			= 1;
			blow_delay_count	= 0;
			blow_duration_count	= 0;
			
			if ((!pin_write(port_rear_fan, LOG_FILE)) || (!pin_write(port_front_fan, LOG_FILE))) { // Включение продувки
				add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
				unlink(PID_FILE);
				system("shutdown -h now");
				exit(-1);
			}

		}
		/*Продувка конец*/
		
		/*Нормальная температура начало*/
		if (((system_mode == 2) || (system_mode == 3)) && (temp_now <= min_cpu_temp)) { // Температура в норме
			add_to_file(LOG_FILE, "[DAEMON] All OK! Temperature - " + str(temp_now) + "C " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
			if (system_mode == 2) {
				if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_ps_fan, LOG_FILE)) || (!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
					add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
					unlink(PID_FILE);
					system("shutdown -h now");
					exit(-1);
				}
			} else {
				if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
					add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer...");
					unlink(PID_FILE);
					system("shutdown -h now");
					exit(-1);
				}
			}
			system_mode	= 0;
		}
		/*Нормальная температура конец*/
	}
	return 0;
}

int write_pid() {
	int fdpid;
	fflush(stdout);								// Сброс буфера обязателен!

	if((fdpid = open (PID_FILE,O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0666)) == -1) { // Открываем файл на запись
		fprintf(stderr,"Error! Other fan deamon is working!\n");	 // и убеждаемся что он существует
		return -1;							 // в единтсвенном экземпляре, иначе выходим
	}

	dup2(fdpid,STDOUT_FILENO);

	fprintf(stdout,"%d",(int) getpid());
	fflush(stdout);								// Сброс буфера обязателен!

	close(fdpid);

	return 0;
}

void sighandler (int signo) {
	if (signo == SIGTERM) {
		#ifdef _DEBUG
		add_to_file(LOG_FILE, "\n=============////=============");
		add_to_file(LOG_FILE, "=============DEBUG============");
		add_to_file(LOG_FILE, "=============////=============\n");
		add_to_file(LOG_FILE, "[DEBUG] Last state: " + str(system_mode));
		
		#endif
		add_to_file(LOG_FILE, "=============////=============");
		add_to_file(LOG_FILE, "====SERV_FAN DAEMON STOPED====");
		add_to_file(LOG_FILE, "=============////=============");
		switch (system_mode) {
			case 1:	if ((!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
						alarm_sound();
					}
					break;

			case 3:	if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
						alarm_sound();
					}
					break;

			default:if ((!pin_write(port_cpu_fan, LOG_FILE)) || (!pin_write(port_ps_fan, LOG_FILE)) || (!pin_write(port_front_fan, LOG_FILE)) || (!pin_write(port_rear_fan, LOG_FILE))) {
						alarm_sound();
					}
					break;
		}
		system("beep -f 400 -l 500");
		system("beep -f 200 -l 500");
		if(unlink(PID_FILE) == -1)
			perror("unlink:");
		_exit(0);
	}

	if (signo == SIGUSR1) {							// Если отправлен сигнал на чтение конфигов
		sigset_t sigset, oldset;
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGUSR1);
		sigprocmask(SIG_BLOCK, &sigset, &oldset);

		add_to_file(LOG_FILE, "=============////=============");
		add_to_file(LOG_FILE, "=====RELOAD CONFIGURATION=====");
		add_to_file(LOG_FILE, "=============////=============");
		if (!load_to_vector(CONF_FILE, conf_base))
			add_to_file(LOG_FILE, "[DAEMON] RELOAD FAILED!!!");
		else {
			if (!InitWorkValues()) {
				alarm_sound();
				local_time	t_n;
				t_n = get_time_now(); // Получение времени
				add_to_file(LOG_FILE, "[DAEMON] Sorry... I must shut down computer... " + str(t_n.day) + "." + str(t_n.mon) + "." + str(t_n.year) + " " + str(t_n.hours) + ":" + str(t_n.min) + ":" + str(t_n.sec));
				unlink(PID_FILE);
				system("shutdown -h now");
				exit(-1);
			}
		}

		sigprocmask(SIG_SETMASK, &oldset, NULL);
		return;
	}
}

void how_to(char *prg_name) {
	printf("How to use: %s [key]\n", prg_name);
	printf("Keys:\n");
	printf("	-s - status daemon\n");
	printf("	-d - start daemon\n");
	printf("	-t - terminate daemon\n");
	printf("	-r - reload configuration\n");
	printf("	-l - display log\n");
	printf("	-c - remove log\n");
	printf("	-v - version of serv_fan\n");
}

int main(int argc, char** argv) {
	if ((argc == 1) || (argc > 2)) {
		how_to(argv[0]);
		return 0;
	}

	if (!strcmp(argv[1], "-v")) {
		printf("SERV_FAN\n");
		printf("VERSION: %s %s %s\n", SF_VERSION.c_str(), __TIME__, __DATE__);
		printf("CONF_LANG_LIB VERSION: %s\n", cl_version().c_str());
		printf(":DV company 2016\n");
		return 0;
	} else if (!strcmp(argv[1], "-r")) {
		FILE	*pidf;
		pid_t	num_pid;
		pidf	= fopen(PID_FILE, "r");
		fscanf(pidf, "%d", &num_pid);
		fclose(pidf);
		if (kill(num_pid, SIGUSR1) == -1) {
			perror("ALL BAD");
		} else
			printf("Refreshed!\n");
		return 0;
	} else if (!strcmp(argv[1], "-t")) {
		FILE	*pidf;
		pid_t	num_pid;
		pidf	= fopen(PID_FILE, "r");
		fscanf(pidf, "%d", &num_pid);
		fclose(pidf);
		if (kill(num_pid, SIGTERM) == -1) {
			perror("ALL BAD");
		} else
			printf("Stopped!\n");
		return 0;
	} else if (!strcmp(argv[1], "-s")) {
		if (fopen(PID_FILE, "r") == NULL)
			printf("Stopped\n");
		else
			printf("Running\n");
		return 0;
	} else if (!strcmp(argv[1], "-l")) {
		FILE	*logf;
		char	log_out[256];

		load_to_vector(CONF_FILE, conf_base);
		LOG_FILE	= conf("log_file", conf_base);
		logf		= fopen(LOG_FILE.c_str(), "r");

		while (fgets(log_out,sizeof(log_out),logf))
			printf("%s", log_out);
		fclose(logf);

		return 0;
	} else if (!strcmp(argv[1], "-c")) {
		load_to_vector(CONF_FILE, conf_base);
		LOG_FILE	= conf("log_file", conf_base);
		if(remove(LOG_FILE.c_str())) {
			perror("Error");
			return 1;
		}
		printf("Successful!!!\n");
		return 0;
	} else if (!strcmp(argv[1], "-d")) {
		printf("Starting...\n");
	} else {
		how_to(argv[0]);
		return 0;
	}

	int pid;
	// загружаем файл конфигурации
	if (!load_to_vector(CONF_FILE, conf_base)) {
		printf("Error!!! CONFIGURATION FILE NOT READED!!!\n");
		return -1;
	}

	if ((LOG_FILE = conf("log_file", conf_base)) == "0x1") {
		printf("[CONFIG] Not found log_file!!!\n");
		return -1;
	}

	// создаем потомка
	pid = fork();

	if (pid == -1) { // если не удалось запустить потомка
		add_to_file(LOG_FILE, "Start Daemon Error: " + (string)strerror(errno));
		return -1;
	} else if (!pid) {// если это потомок
			// данный код уже выполняется в процессе потомка
			// разрешаем выставлять все биты прав на создаваемые файлы,
			// иначе у нас могут быть проблемы с правами доступа
			// umask(0);
			if(write_pid() == -1)	// Если другой такой демон уже запущен - прекращение работы
				return -1;
			// создаём новый сеанс, чтобы не зависеть от родителя
			setsid();
			// переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
			// к примеру с размантированием дисков
			chdir("/");

			// закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			signal(SIGTERM,&sighandler);
			signal(SIGUSR1,&sighandler);

			add_to_file(LOG_FILE, "=============/////=============");
			add_to_file(LOG_FILE, "====SERV_FAN DAEMON STARTED====");
			add_to_file(LOG_FILE, "=============/////=============");
			
			add_to_file(LOG_FILE, "\nSERV_FAN VERSION: " + SF_VERSION + " " + (string)__TIME__ + " " + (string)__DATE__);
			add_to_file(LOG_FILE, "CONF_LANG_LIB VERSION: " + cl_version() + "\n");
			int to_back = TempWatch();
			add_to_file(LOG_FILE, "[DAEMON] Returned code: " + str(to_back));
			add_to_file(LOG_FILE, "=============////=============");
			add_to_file(LOG_FILE, "====SERV_FAN DAEMON STOPED====");
			add_to_file(LOG_FILE, "=============////=============");
			if(unlink(PID_FILE) == -1)
				perror("unlink:");
			return to_back;
		} else { // если это родитель
			return 0;
		}
	return 0;
}
