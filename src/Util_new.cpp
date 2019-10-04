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
// Return true if an user is recovered successfullyand vice versa
static bool UserRecover(CManager& manager, const UserRecord& acc, const map <ORDER_TICKET, TradeRecord>& orders, ofstream& olog)
{
	bool successed = true;

	if (manager->BackupRestoreUsers(&acc, 1) == RET_OK) {
		PrintLog(&olog, "Restore UserRecord " + to_string(acc.login) + " SUCCESSFULLY!");
	}
	else {
		PrintLog(&olog, "Restore UserRecord " + to_string(acc.login) + " FAILED!");
		successed = false;
		return false;
	}

	for (const auto& order_i : orders)
	{
		int total_orders = 1;
		TradeRestoreResult* bkTradeResult = manager->BackupRestoreOrders(&order_i.second, &total_orders);
		int res = bkTradeResult[0].res;
		manager->MemFree(bkTradeResult);

		if (res != RET_OK) {
			successed = false;
			PrintLog(&olog, "\tRestore order #" + to_string(order_i.second.order) + " FAILED!");
		}
	}

	if (manager->AdmBalanceFix(&acc.login, 1) != RET_OK) {
		successed = false;
		PrintLog(&olog, "\tFix balance of user " + to_string(acc.login) + " FAILED!");
	}

	return successed;
}




