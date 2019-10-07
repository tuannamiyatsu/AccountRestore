#pragma once

#include "StdAfx.h"
#include "Util.h"
#include "MT4ManagerAPI.h"
#include "Cmanager.h"
#include "HttpRequest.h"


typedef int ORDER_TICKET;

// User info and all orders from this user
typedef pair <UserRecord, map <ORDER_TICKET, TradeRecord>> User_Trade_Record;


extern string appName;

// Config parameters
extern string server;
extern string server2;
extern int id;
extern string password;
extern string account_list;
extern wstring http_URL;
extern bool Search_All_Order_Files;
extern bool Search_Orders_From_Certain_Files;
//-----------------------------------------------


// Return true if an user is recovered successfully and vice versa
static bool UserRecover(CManager &manager, const UserRecord &acc, const map <ORDER_TICKET, TradeRecord> &orders, ofstream &olog)
{
	bool successed = true;

	if (manager->BackupRestoreUsers(&acc, 1) == RET_OK) {
		PrintLog(&olog, "Server " + server  +" Restore UserRecord " + to_string(acc.login) + " SUCCESSFULLY!");
	}
	else {
		PrintLog(&olog, "Server " + server + " Restore UserRecord " + to_string(acc.login) + " FAILED!");
		successed = false;
		return false;
	}

	for (const auto &order_i : orders)
	{
		int total_orders = 1;
		TradeRestoreResult *bkTradeResult = manager->BackupRestoreOrders(&order_i.second, &total_orders);
		int res = bkTradeResult[0].res;
		manager->MemFree(bkTradeResult);

		if (res != RET_OK) {
			successed = false;
			PrintLog(&olog, "\tServer " + server + " Restore order #" + to_string(order_i.second.order) + " FAILED!");
		}
	}

	if (manager->AdmBalanceFix(&acc.login, 1) != RET_OK) {
		successed = false;
		PrintLog(&olog, "\tServer " + server + " Fix balance of user " + to_string(acc.login) + " FAILED!");
	}

	return successed;
}

// Return true if an user is recovered successfully and vice versa
static bool UserRecoverNew(CManager& manager_new, const UserRecord& acc, const map <ORDER_TICKET, TradeRecord>& orders, ofstream& olog)
{
	bool successed = true;

	if (manager_new->BackupRestoreUsers(&acc, 1) == RET_OK) {
		PrintLog(&olog, "Server " + server2 + " Restore UserRecord " + to_string(acc.login) + " SUCCESSFULLY!");
	}
	else {
		PrintLog(&olog, "Server " + server2 + " Restore UserRecord " + to_string(acc.login) + " FAILED!");
		successed = false;
		return false;
	}

	for (const auto& order_i : orders)
	{
		int total_orders = 1;
		TradeRestoreResult* bkTradeResult = manager_new->BackupRestoreOrders(&order_i.second, &total_orders);
		int res = bkTradeResult[0].res;
		manager_new->MemFree(bkTradeResult);

		if (res != RET_OK) {
			successed = false;
			PrintLog(&olog, "\tServer " + server2 + " Restore order #" + to_string(order_i.second.order) + " FAILED!");
		}
	}

	if (manager_new->AdmBalanceFix(&acc.login, 1) != RET_OK) {
		successed = false;
		PrintLog(&olog, "\tServer " + server2 + " Fix balance of user " + to_string(acc.login) + " FAILED!");
	}

	return successed;
}

MainTask::~MainTask() {}

void GetUserRecord(CManager &manager, const string &file, const string &search_query, map <int, pair <bool, bool>> &requested_account_list, map <int, User_Trade_Record> &requested_account_data, ofstream &olog)
{
	int total_users = 0;
	time_t start_restoring = time(NULL);
	UserRecord *users = manager->BackupRequestUsers(file.c_str(), search_query.c_str(), &total_users);
	PrintLog(&olog, "\tServer "+ server +" Seeking data in \"" + file + "\" (took " + to_string(time(NULL) - start_restoring) + " seconds)");
	PrintLog(&olog, "\tServer " + server + " Search value: \"" + search_query + "\"");
	map <int, UserRecord> user_map;
	for (int user_i = 0; user_i < total_users; user_i++)
		user_map[users[user_i].login] = users[user_i];
	manager->MemFree(users);

	for (const auto &u_m_i : user_map)
	{
		requested_account_data[u_m_i.first].first = u_m_i.second;
		PrintLog(&olog, "\t\tServer " + server + " Found UserRecord for User " + to_string(u_m_i.first));

		map <int, pair <bool, bool>>::iterator r_acc_l_i = requested_account_list.find(u_m_i.first);
		r_acc_l_i->second.first = true;
	}
}

void GetUserRecordNew(CManager& manager_new, const string& file, const string& search_query_new, map <int, pair <bool, bool>>& requested_account_list_new, map <int, User_Trade_Record>& requested_account_data_new, ofstream& olog)
{
	int total_users = 0;
	time_t start_restoring = time(NULL);
	UserRecord* users = manager_new->BackupRequestUsers(file.c_str(), search_query_new.c_str(), &total_users);
	PrintLog(&olog, "\tServer " + server2 + " Seeking data in \"" + file + "\" (took " + to_string(time(NULL) - start_restoring) + " seconds)");
	PrintLog(&olog, "\tServer " + server2 + " Search value: \"" + search_query_new + "\"");
	map <int, UserRecord> user_map;
	for (int user_i = 0; user_i < total_users; user_i++)
		user_map[users[user_i].login] = users[user_i];
	manager_new->MemFree(users);

	for (const auto& u_m_i : user_map)
	{
		requested_account_data_new[u_m_i.first].first = u_m_i.second;
		PrintLog(&olog, "\t\tServer " + server2 + " Found UserRecord for User " + to_string(u_m_i.first));

		map <int, pair <bool, bool>>::iterator r_acc_l_i = requested_account_list_new.find(u_m_i.first);
		r_acc_l_i->second.first = true;
	}
}

void GetOrders(CManager &manager, const string &file, const string &search_query, map <int, pair <bool, bool>> &requested_account_list, map <int, User_Trade_Record> &requested_account_data, ofstream &olog)
{
	int total_orders = 0;
	time_t start_restoring = time(NULL);
	TradeRecord *orders = manager->BackupRequestOrders(file.c_str(), search_query.c_str(), &total_orders);
	PrintLog(&olog, "\tServer " + server + " Seeking data in \"" + file + "\" (took " + to_string(time(NULL) - start_restoring) + " seconds)");
	PrintLog(&olog, "\tServer " + server + " Search value: \"" + search_query + "\"");
	map <int, vector <TradeRecord>> order_map;		// user, orders
	for (int order_i = 0; order_i < total_orders; order_i++) {
		order_map[orders[order_i].login].emplace_back(orders[order_i]);
	}
	manager->MemFree(orders);

	for (const auto &o_m_i : order_map)
	{
		int new_order_cnt = 0;
		map <ORDER_TICKET, TradeRecord> &r_a_d_i_order_map = requested_account_data[o_m_i.first].second;
		for (const TradeRecord &order_i : o_m_i.second) {
			// If this order is already added -> skipped
			if (r_a_d_i_order_map.find(order_i.order) != r_a_d_i_order_map.end())
				continue;

			r_a_d_i_order_map[order_i.order] = order_i;
			new_order_cnt++;
		}

		map <int, pair <bool, bool>>::iterator r_acc_l_i = requested_account_list.find(o_m_i.first);
		if (new_order_cnt > 0) {
			PrintLog(&olog, "\t\tServer " + server + " Found " + to_string(new_order_cnt) + " new orders for User " + to_string(o_m_i.first));
			r_acc_l_i->second.second = true;
		}
	}
}

void GetOrdersNew(CManager& manager_new, const string& file, const string& search_query_new, map <int, pair <bool, bool>>& requested_account_list_new, map <int, User_Trade_Record>& requested_account_data_new, ofstream& olog)
{
	int total_orders = 0;
	time_t start_restoring = time(NULL);
	TradeRecord* orders = manager_new->BackupRequestOrders(file.c_str(), search_query_new.c_str(), &total_orders);
	PrintLog(&olog, "\tServer " + server2 + " Seeking data in \"" + file + "\" (took " + to_string(time(NULL) - start_restoring) + " seconds)");
	PrintLog(&olog, "\tServer " + server2 + " Search value: \"" + search_query_new + "\"");
	map <int, vector <TradeRecord>> order_map;		// user, orders
	for (int order_i = 0; order_i < total_orders; order_i++) {
		order_map[orders[order_i].login].emplace_back(orders[order_i]);
	}
	manager_new->MemFree(orders);

	for (const auto& o_m_i : order_map)
	{
		int new_order_cnt = 0;
		map <ORDER_TICKET, TradeRecord>& r_a_d_i_order_map = requested_account_data_new[o_m_i.first].second;
		for (const TradeRecord& order_i : o_m_i.second) {
			// If this order is already added -> skipped
			if (r_a_d_i_order_map.find(order_i.order) != r_a_d_i_order_map.end())
				continue;

			r_a_d_i_order_map[order_i.order] = order_i;
			new_order_cnt++;
		}

		map <int, pair <bool, bool>>::iterator r_acc_l_i = requested_account_list_new.find(o_m_i.first);
		if (new_order_cnt > 0) {
			PrintLog(&olog, "\t\tServer " + server2 + " Found " + to_string(new_order_cnt) + " new orders for User " + to_string(o_m_i.first));
			r_acc_l_i->second.second = true;
		}
	}
}

// Process main tasks
MainTask::MainTask(SYSTEMTIME now)
{
	ofstream olog("Logs\\" + PrintSystemTime(now, '.', '.') + ".txt");

	// A list contains all requested accounts,
	// the first bool value indicates whether UserRecord for this account is gotten (true)
	// the second bool value indicates whether orders for this account is gotten (true)
	map <int, pair <bool, bool>> requested_account_list;
	map <int, pair <bool, bool>> requested_account_list_new;

	// All data needs to be restored and recovered
	map <int, User_Trade_Record> requested_account_data;
	map <int, User_Trade_Record> requested_account_data_new;
	CManager manager;
	CManager manager_new;

	// Login
	{
		// Check dll file
		if (!manager.IsValid())
			return;

		//--- connect
		if (manager->Connect(server.c_str()) != RET_OK ||
			manager->Login(id, password.c_str()) != RET_OK)
		{
			PrintLog(&olog, "Login Server "+server+" FAILED!");
			return;
		}

		//--- connect server 2
		if (manager_new->Connect(server2.c_str()) != RET_OK ||
			manager_new->Login(id, password.c_str()) != RET_OK)
		{
			PrintLog(&olog, "Login Server " + server2 + " FAILED!");
			return;
		}

		PrintLog(&olog, "Connect to " + server + " as " + to_string(id) + " successfully");
		PrintLog(&olog, "Connect to " + server2 + " as " + to_string(id) + " successfully");
		PrintLog(&olog, "");
	}

	// Get account list from config file
	/*PrintLog(&olog, "Account list from config file:");
	if (!Account_List_Format(account_list)) {
		PrintLog(&olog, "\tValue \"" + account_list + "\" has the wrong format! -> skipped (only 0123456789 and comma are allowed)");
		PrintLog(&olog, "");
	}
	else {*/
		vector <string> acc_list_without_comma = split(account_list.c_str(), ",");
		for (const string &acc_s_i : acc_list_without_comma) {
			int acc_i = atoi(acc_s_i.c_str());
			int total_rRecord = 1;
			UserRecord *p_users = manager->UserRecordsRequest(&acc_i, &total_rRecord);

			if (to_string(acc_i).length() == 4 && to_string(acc_i)[0] == '9') {
				UserRecord* p_users = manager->UserRecordsRequest(&acc_i, &total_rRecord);

				// If user exists -> skip
				if (p_users != NULL) {
					PrintLog(&olog, "\tServer" + server + " Account " + to_string(acc_i) + " already EXISTED!");
					manager->MemFree(p_users);
					continue;
				}

				PrintLog(&olog, "\t" + to_string(acc_i));
				requested_account_list[acc_i] = mp(false, false);
			}
			if (to_string(acc_i).length() == 9 && to_string(acc_i)[0] == '8') {
				UserRecord* p_users = manager_new->UserRecordsRequest(&acc_i, &total_rRecord);

				// If user exists -> skip
				if (p_users != NULL) {
					PrintLog(&olog, "\tServer" + server2 + " Account " + to_string(acc_i) + " already EXISTED!");
					manager_new->MemFree(p_users);
					continue;
				}

				PrintLog(&olog, "\t" + to_string(acc_i));
				requested_account_list_new[acc_i] = mp(false, false);
			}

		}
		PrintLog(&olog, "");
	//}
	
	//// Get account list from URL
	//if (wstring_to_string(http_URL) != "") {
	//	PrintLog(&olog, "Getting requested accounts from: " + wstring_to_string(http_URL));

	//	HttpRequest acc_list_http(http_URL);
	//	string temp_s = acc_list_http.doGet(http_URL);
	//	{
	//		stringstream temp_ss(temp_s);
	//		string line;
	//		while (getline(temp_ss, line)) {
	//			if (!Account_List_Format(line)) {
	//				PrintLog(&olog, "\tValue \"" + line + "\" has the wrong format! -> skipped (only 0123456789 and comma are allowed)");
	//				continue;
	//			}
	//			vector <string> line_v = split(line.c_str(), ",");
	//			for (const string &acc_s_i : line_v) {
	//				int acc_i = atoi(acc_s_i.c_str());
	//				int total_rRecord = 1;
	//				UserRecord *p_users = manager->UserRecordsRequest(&acc_i, &total_rRecord);

	//				// If user exists -> skip
	//				if (p_users != NULL) {
	//					PrintLog(&olog, "\tAccount " + to_string(acc_i) + " already EXISTED!");
	//					manager->MemFree(p_users);
	//					continue;
	//				}

	//				PrintLog(&olog, "\t" + to_string(acc_i));
	//				requested_account_list[acc_i] = mp(false, false);
	//			}
	//			PrintLog(&olog, "");
	//		}
	//	}
	//}

	if (requested_account_list.size() == 0 && requested_account_list_new.size() == 0) {
		PrintLog(&olog, "There's no account need to be restored.");
		return;
	}

	if (requested_account_list.size() != 0) {
		// Get UserRecord for requested account
		{
			// Get backup user file names
			set <string> bkUsers_file;
			{
				int total_bkUsers = 0;
				BackupInfo* bkUsers = manager->BackupInfoUsers(BACKUPS_ALL, &total_bkUsers);
				for (int user_i = 0; user_i < total_bkUsers; user_i++)
					bkUsers_file.insert(bkUsers[user_i].file);
				manager->MemFree(bkUsers);
			}
			PrintLog(&olog, "Total user backup files = " + to_string(bkUsers_file.size()));

			for (set <string>::const_reverse_iterator f_i = bkUsers_file.rbegin(); f_i != bkUsers_file.rend(); f_i++)
			{
				string file_i = *f_i;

				vector <int> inqueue_acc;	// Contains all accounts haven't got their's UserRecords
				for (const auto& r_acc_l_i : requested_account_list) {
					// If UserRecord for this account is already gotten -> skip
					if (r_acc_l_i.second.first) continue;

					inqueue_acc.emplace_back(r_acc_l_i.first);
				}

				if (inqueue_acc.empty()) break;	// If all accounts have gotten UserRecords -> break

				string search_query;
				for (int acc_i = 0; acc_i < inqueue_acc.size(); acc_i++) {
					string temp = search_query + to_string(inqueue_acc[acc_i]);
					if (temp.size() > QUERY_SIZE_MAX) {
						GetUserRecord(manager, file_i, search_query, requested_account_list, requested_account_data, olog);
						search_query = "";
						PrintLog(&olog, "");
					}
					search_query += to_string(inqueue_acc[acc_i]) + ",";

					if (acc_i == inqueue_acc.size() - 1) {
						GetUserRecord(manager, file_i, search_query, requested_account_list, requested_account_data, olog);
						search_query = "";
						PrintLog(&olog, "");
					}
				}
			}
			PrintLog(&olog, "");
		}

		// Check if any account hasn't had UserRecord
		{
			int accounts_has_UserRecord = 0;
			for (auto& r_acc_l_i : requested_account_list) {
				if (r_acc_l_i.second.first == false) {
					PrintLog(&olog, "UserRecord for Account " + to_string(r_acc_l_i.first) + ": NOT FOUND -> skipped!");
					requested_account_list.erase(r_acc_l_i.first);	// Delete accounts which haven't gotten UserRecord
					continue;
				}
				else {
					PrintLog(&olog, "UserRecord for Account " + to_string(r_acc_l_i.first) + ": FOUND");
					accounts_has_UserRecord++;
				}
			}
			if (accounts_has_UserRecord == 0) {
				PrintLog(&olog, "All accounts haven't gotten their UserRecords! -> Stop processing");
				return;
			}
			PrintLog(&olog, "");
		}

		// Get orders for requested accounts
		{
			// Get backup order file names
			set <string> bkOrders_file;
			if (Search_Orders_From_Certain_Files) {
				ifstream c_o_f_ifs(appName + ".orderfiles");
				if (c_o_f_ifs.good() == false) {
					ofstream c_o_f_ofs(appName + ".orderfiles");	// create file
				}
				else {
					string orderFileName;
					while (c_o_f_ifs >> orderFileName) {
						bkOrders_file.insert(orderFileName);
					}
				}
			}
			else {
				{
					int total_bkOrders = 0;
					BackupInfo* bkOrders = manager->BackupInfoOrders(BACKUPS_ALL, &total_bkOrders);
					for (int order_i = 0; order_i < total_bkOrders; order_i++)
						bkOrders_file.insert(bkOrders[order_i].file);
					manager->MemFree(bkOrders);
				}
			}
			PrintLog(&olog, "Total order backup files = " + to_string(bkOrders_file.size()));

			for (set <string>::const_reverse_iterator f_i = bkOrders_file.rbegin(); f_i != bkOrders_file.rend(); f_i++)
			{
				string file_i = *f_i;

				vector <int> inqueue_acc;	// Contains all accounts need to get orders
				for (const auto& r_acc_l_i : requested_account_list) {
					if (Search_All_Order_Files == false && r_acc_l_i.second.second == true)
						continue;

					inqueue_acc.emplace_back(r_acc_l_i.first);
				}

				string search_query;
				for (int acc_i = 0; acc_i < inqueue_acc.size(); acc_i++) {
					string temp = search_query + to_string(inqueue_acc[acc_i]);
					if (temp.size() > QUERY_SIZE_MAX) {
						GetOrders(manager, file_i, search_query, requested_account_list, requested_account_data, olog);
						search_query = "";
						PrintLog(&olog, "");
					}
					search_query += to_string(inqueue_acc[acc_i]) + ",";

					if (acc_i == inqueue_acc.size() - 1) {
						GetOrders(manager, file_i, search_query, requested_account_list, requested_account_data, olog);
						search_query = "";
						PrintLog(&olog, "");
					}
				}
			}
			PrintLog(&olog, "");
		}

		// Recover accounts and orders
		for (const auto& r_acc_d_i : requested_account_data)
		{
			PrintLog(&olog, "Restoring account " + to_string(r_acc_d_i.first));
			int total_rRecord = 1;
			UserRecord* p_users = manager->UserRecordsRequest(&r_acc_d_i.first, &total_rRecord);

			// If user doesn't exist -> recover
			if (p_users == NULL) {
				int res = UserRecover(manager, r_acc_d_i.second.first, r_acc_d_i.second.second, olog);
				if (res == false) {
					PrintLog(&olog, "Restore account " + to_string(r_acc_d_i.first) + " FAILED!");
				}
				else PrintLog(&olog, "Restore account " + to_string(r_acc_d_i.first) + " SUCCESSFULLY! (" + to_string(r_acc_d_i.second.second.size()) + " orders)");
			}
			else {
				PrintLog(&olog, "Account " + to_string(r_acc_d_i.first) + " already EXISTED!");
				manager->MemFree(p_users);
			}
			PrintLog(&olog, "");
		}


	}

	if (requested_account_list_new.size() != 0) {
		// Get UserRecord for requested account
		{
			// Get backup user file names
			set <string> bkUsers_file_new;
			{
				int total_bkUsers = 0;
				BackupInfo* bkUsers = manager_new->BackupInfoUsers(BACKUPS_ALL, &total_bkUsers);
				for (int user_i = 0; user_i < total_bkUsers; user_i++)
					bkUsers_file_new.insert(bkUsers[user_i].file);
				manager_new->MemFree(bkUsers);
			}
			PrintLog(&olog, "Total user backup files = " + to_string(bkUsers_file_new.size()));

			for (set <string>::const_reverse_iterator f_i = bkUsers_file_new.rbegin(); f_i != bkUsers_file_new.rend(); f_i++)
			{
				string file_i = *f_i;

				vector <int> inqueue_acc;	// Contains all accounts haven't got their's UserRecords
				for (const auto& r_acc_l_i : requested_account_list_new) {
					// If UserRecord for this account is already gotten -> skip
					if (r_acc_l_i.second.first) continue;

					inqueue_acc.emplace_back(r_acc_l_i.first);
				}

				if (inqueue_acc.empty()) break;	// If all accounts have gotten UserRecords -> break

				string search_query_new;
				for (int acc_i = 0; acc_i < inqueue_acc.size(); acc_i++) {
					string temp = search_query_new + to_string(inqueue_acc[acc_i]);
					if (temp.size() > QUERY_SIZE_MAX) {
						GetUserRecordNew(manager_new, file_i, search_query_new, requested_account_list_new, requested_account_data_new, olog);
						search_query_new = "";
						PrintLog(&olog, "");
					}
					search_query_new += to_string(inqueue_acc[acc_i]) + ",";

					if (acc_i == inqueue_acc.size() - 1) {
						GetUserRecordNew(manager_new, file_i, search_query_new, requested_account_list_new, requested_account_data_new, olog);
						search_query_new = "";
						PrintLog(&olog, "");
					}
				}
			}
			PrintLog(&olog, "");
		}

		// Check if any account hasn't had UserRecord
		{
			int accounts_has_UserRecord = 0;
			for (auto& r_acc_l_i : requested_account_list_new) {
				if (r_acc_l_i.second.first == false) {
					PrintLog(&olog, "UserRecord for Account " + to_string(r_acc_l_i.first) + ": NOT FOUND -> skipped!");
					requested_account_list_new.erase(r_acc_l_i.first);	// Delete accounts which haven't gotten UserRecord
					continue;
				}
				else {
					PrintLog(&olog, "UserRecord for Account " + to_string(r_acc_l_i.first) + ": FOUND");
					accounts_has_UserRecord++;
				}
			}
			if (accounts_has_UserRecord == 0) {
				PrintLog(&olog, "All accounts haven't gotten their UserRecords! -> Stop processing");
				return;
			}
			PrintLog(&olog, "");
		}

		// Get orders for requested accounts
		{
			// Get backup order file names
			set <string> bkOrders_file;
			if (Search_Orders_From_Certain_Files) {
				ifstream c_o_f_ifs(appName + ".orderfiles");
				if (c_o_f_ifs.good() == false) {
					ofstream c_o_f_ofs(appName + ".orderfiles");	// create file
				}
				else {
					string orderFileName;
					while (c_o_f_ifs >> orderFileName) {
						bkOrders_file.insert(orderFileName);
					}
				}
			}
			else {
				{
					int total_bkOrders = 0;
					BackupInfo* bkOrders = manager_new->BackupInfoOrders(BACKUPS_ALL, &total_bkOrders);
					for (int order_i = 0; order_i < total_bkOrders; order_i++)
						bkOrders_file.insert(bkOrders[order_i].file);
					manager_new->MemFree(bkOrders);
				}
			}
			PrintLog(&olog, "Total order backup files = " + to_string(bkOrders_file.size()));

			for (set <string>::const_reverse_iterator f_i = bkOrders_file.rbegin(); f_i != bkOrders_file.rend(); f_i++)
			{
				string file_i = *f_i;

				vector <int> inqueue_acc;	// Contains all accounts need to get orders
				for (const auto& r_acc_l_i : requested_account_list_new) {
					if (Search_All_Order_Files == false && r_acc_l_i.second.second == true)
						continue;

					inqueue_acc.emplace_back(r_acc_l_i.first);
				}

				string search_query_new;
				for (int acc_i = 0; acc_i < inqueue_acc.size(); acc_i++) {
					string temp = search_query_new + to_string(inqueue_acc[acc_i]);
					if (temp.size() > QUERY_SIZE_MAX) {
						GetOrdersNew(manager_new, file_i, search_query_new, requested_account_list_new, requested_account_data_new, olog);
						search_query_new = "";
						PrintLog(&olog, "");
					}
					search_query_new += to_string(inqueue_acc[acc_i]) + ",";

					if (acc_i == inqueue_acc.size() - 1) {
						GetOrdersNew(manager_new, file_i, search_query_new, requested_account_list_new, requested_account_data_new, olog);
						search_query_new = "";
						PrintLog(&olog, "");
					}
				}
			}
			PrintLog(&olog, "");
		}

		// Recover accounts and orders
		for (const auto& r_acc_d_i : requested_account_data_new)
		{
			PrintLog(&olog, "Restoring account " + to_string(r_acc_d_i.first));
			int total_rRecord = 1;
			UserRecord* p_users = manager_new->UserRecordsRequest(&r_acc_d_i.first, &total_rRecord);

			// If user doesn't exist -> recover
			if (p_users == NULL) {
				int res = UserRecover(manager_new, r_acc_d_i.second.first, r_acc_d_i.second.second, olog);
				if (res == false) {
					PrintLog(&olog, "Restore account " + to_string(r_acc_d_i.first) + " FAILED!");
				}
				else PrintLog(&olog, "Restore account " + to_string(r_acc_d_i.first) + " SUCCESSFULLY! (" + to_string(r_acc_d_i.second.second.size()) + " orders)");
			}
			else {
				PrintLog(&olog, "Account " + to_string(r_acc_d_i.first) + " already EXISTED!");
				manager_new->MemFree(p_users);
			}
			PrintLog(&olog, "");
		}
	
	}
	

}

