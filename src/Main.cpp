

#pragma once

#include "StdAfx.h"
#include "Cmanager.h"
#include "HttpRequest.h"
#include "Util.h"


//----------------------------------------GLOBAL VARIABLES----------------------------------------
string appName = "AccountRestore";

// Config parameters
string server;
string server2;
int id;
string password;
string account_list;
long long interval_m;
bool Search_All_Order_Files;
bool Search_Orders_From_Certain_Files;
wstring http_URL;
//wstring proxy_server;
//wstring proxy_user;
//wstring proxy_user;
//wstring proxy_password;
//---------------------------

//----------------------------------------END OF GLOBAL VARIABLES----------------------------------------


void PrintCommandGuide()
{
	
	PrintLog(&cout, "Command format: \"" + appName + " -p <password>\"");
	PrintLog(&cout, "Other parameters must be filled in \"" + appName + ".cfg\"");
}


void Process(SYSTEMTIME current, int *running) {

	time_t start_time = time(NULL);
	PrintLog(&cout, "New task has just been created");
	MainTask mainTask(current);
	PrintLog(&cout, "The task was created at " + PrintSystemTime(current, '.', ':') + " has done (took " + to_string(time(NULL) - start_time) + " seconds)");
	*running = false;
}


//----------------------------------------MAIN----------------------------------------
int main(int argc, char* argv[]) {

	_mkdir("Logs");

	// Get parameters
	{
		string infile_name = appName + ".cfg";
		ifstream ifs(infile_name);
		if (ifs.good() == false) {
			PrintLog(&cout, "Could not find " + infile_name);
			// Create file
			ofstream ofs(infile_name);
			ofs << "SERVER=\n";
			ofs << "SERVER2=\n";
			ofs << "LOGIN=\n";
			ofs << "HTTP_URL=\n";
			ofs << "ACCOUNT_LIST=\n";
			ofs << "INTERVAL(MINUTE)=\n";
			ofs << "SEARCH_ALL_ORDER_FILES(on,off)=on\n";
			ofs << "SEARCH_ORDERS_FROM_CERTAIN_FILES(on,off)=off\n";
			return -1;
		}

		map <string, string> param_map;		// name, value
		string s;
		while (getline(ifs, s)) {
			/*
			vector <string> name_value = split(s.c_str(), "=");
			if (name_value.size() == 1) param_map[trim(name_value[0])] = "";
			if (name_value.size() < 2) continue;
			param_map[trim(name_value[0])] = trim(name_value[1]);
			*/
			if (s.find("=") != std::string::npos) {
				param_map[trim(s.substr(0, s.find("=")))] = trim(s.substr(s.find("=") + 1));
				PrintLog(&cout, trim(s.substr(0, s.find("="))) + "=" + trim(s.substr(s.find("=") + 1)));
			}
			else {
				continue;
			}
		}

		map <string, string>::const_iterator p_m_it;
		
		p_m_it = param_map.find("SERVER");
		if (p_m_it != param_map.end()) server = p_m_it->second;

		p_m_it = param_map.find("SERVER2");
		if (p_m_it != param_map.end()) server2 = p_m_it->second;

		p_m_it = param_map.find("LOGIN");
		if (p_m_it != param_map.end()) id = atoi(p_m_it->second.c_str());

		p_m_it = param_map.find("HTTP_URL");
		if (p_m_it != param_map.end()) http_URL = string_to_wstring(p_m_it->second);

		p_m_it = param_map.find("ACCOUNT_LIST");
		if (p_m_it != param_map.end()) account_list = p_m_it->second;
		
		interval_m = RUN_ONE_TIME;
		p_m_it = param_map.find("INTERVAL(MINUTE)");
		if (p_m_it != param_map.end() && p_m_it->second.size() > 0) {
			if (p_m_it->second.size() > 9) {
				PrintLog(&cout, "INTERVAL must be in a range of 1 -> 1440 (1 day)");
				return -1;
			}
			interval_m = atoi(p_m_it->second.c_str());
			if (interval_m < INTERVAL_M_MIN || INTERVAL_M_MAX < interval_m) {
				PrintLog(&cout, "INTERVAL must be in a range of " + to_string(INTERVAL_M_MIN) + " -> " + to_string(INTERVAL_M_MAX));
				return -1;
			}
		}

		Search_All_Order_Files = true;
		p_m_it = param_map.find("SEARCH_ALL_ORDER_FILES(on,off)");
		if (p_m_it != param_map.end() && (p_m_it->second == "on" || p_m_it->second == "off"))
			Search_All_Order_Files = (p_m_it->second == "on");
		else {
			PrintLog(&cout, "SEARCH_ALL_ORDER_FILES can only take \"on\" or \"off\" as a value");
			return -1;
		}

		Search_Orders_From_Certain_Files = false;
		p_m_it = param_map.find("SEARCH_ORDERS_FROM_CERTAIN_FILES(on,off)");
		if (p_m_it != param_map.end() && (p_m_it->second == "on" || p_m_it->second == "off"))
			Search_Orders_From_Certain_Files = (p_m_it->second == "on");
		else {
			PrintLog(&cout, "SEARCH_ORDERS_FROM_CERTAIN_FILES can only take \"on\" or \"off\" as a value");
			return -1;
		}
	}

	// Get password from console
	{
		map <string, string> console_param_map;
		for (int param_i = 1; param_i < argc - 1; param_i += 2) {
			console_param_map[argv[param_i]] = argv[param_i + 1];
		}

		map <string, string>::const_iterator c_p_m_it;
		c_p_m_it = console_param_map.find("-p");
		if (c_p_m_it != console_param_map.end()) password = c_p_m_it->second;
		else {
			PrintCommandGuide();
			return -1;
		}
	}

	// Loop of processing
	SYSTEMTIME last_run;
	time_t _last_run;
	vector <int> Is_Running;	// = true if thread[i] is running
	set <int> running_thread;	// all running threads
	vector <thread*> task_v;
	do {
		Is_Running.emplace_back(true);
		running_thread.insert(Is_Running.size() - 1);
		GetLocalTime(&last_run);
		time(&_last_run);
		task_v.emplace_back(new thread(Process, last_run, &Is_Running[Is_Running.size()-1]));
		
		// if interval_m is not set
		if (interval_m == RUN_ONE_TIME) break;

		while (((long long)time(NULL) - (long long)_last_run) / 60 < interval_m) {
			for (int thread_i : running_thread) {
				if (Is_Running[thread_i] == false) {
					task_v[thread_i]->join();
					running_thread.erase(thread_i);
				}
			}
			Sleep(100);
		}
	} while (1);

	// Destroy running threads
	while (!running_thread.empty()) {
		for (int thread_i : running_thread) {
			if (Is_Running[thread_i] == false) {
				task_v[thread_i]->join();
				running_thread.erase(thread_i);
			}
		}
		Sleep(100);
	}

	return 0;
}
