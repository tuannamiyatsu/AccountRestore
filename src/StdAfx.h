#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <time.h>
#include <conio.h>
#include <process.h>
#include <Windows.h>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <direct.h>
#include <Winhttp.h>
#include <functional>
#include <thread>
#include <chrono>
#include <queue>
#include <vector>


using namespace std;


#define CERR(a)				cerr << #a << " = " << a << "\n";
#define mp(a, b)			make_pair(a, b)

#define RUN_ONE_TIME		0
#define INTERVAL_M_MIN		1
#define INTERVAL_M_MAX		(24 * 60)
#define QUERY_SIZE_MAX		64
