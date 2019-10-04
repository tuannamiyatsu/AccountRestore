#pragma once


class MainTask{

public:
	MainTask(SYSTEMTIME now);
	~MainTask();
};


// trim from start
static inline string &ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
static inline string &rtrim(string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline string &trim(string &s) {
	return ltrim(rtrim(s));
}

static inline vector<string> split(const char* strInput, const char* delim)
{
	vector<string> result;
	char * str = new(nothrow) char[strlen(strInput) + 1];
	if (str == NULL) return result;
	strcpy(str, strInput);

	char* token = strtok(str, delim);
	while (token != NULL) {
		result.push_back(token);
		token = strtok(NULL, delim);
	}
	delete[] str;
	return result;
}


// return true if strVal has the right format
static inline bool Account_List_Format(const string strVal) {
	bool right_format = (strVal.find_first_not_of("0123456789,") == string::npos);
	return right_format;
}


static string ToUpper(const string &s)
{
	string result;
	for (char c : s) {
		if ('a' <= c && c <= 'z') c -= 32;
		result += c;
	}
	return result;
}


static string ToLower(const string &s)
{
	string result;
	for (char c : s) {
		if ('A' <= c && c <= 'Z') c += 32;
		result += c;
	}
	return result;
}


// return 00:00:00 of a day
static SYSTEMTIME start_of_a_day(const SYSTEMTIME &t)
{
	SYSTEMTIME result = t;
	result.wHour = 0;
	result.wMinute = 0;
	result.wSecond = 0;
	result.wMilliseconds = 0;

	return result;
}


// return a string with more 0s at front if length of num < len
static string FillWithZeroesAtFront(int val, int len)
{
	string temp = std::to_string(val);
	string result;

	for (int i = 1; i <= len - (int)temp.size(); i++)
		result += '0';

	return (result += temp);
}


// return a string with more 0s at end if length of num < len
static string FillWithZeroesAtEnd(int val, int len)
{
	string temp = std::to_string(val);
	string result = temp;

	for (int i = 1; i <= len - (int)temp.size(); i++)
		result += '0';

	return result;
}


// e.g. 2016.10.24 12:00:00.000
static string PrintSystemTime(const SYSTEMTIME &t, char date_delim, char hour_delim)
{
	string s = FillWithZeroesAtFront(t.wYear, 4) + date_delim + FillWithZeroesAtFront(t.wMonth + 1, 2) + date_delim + FillWithZeroesAtFront(t.wDay, 2);
	s += " " + FillWithZeroesAtFront(t.wHour, 2) + hour_delim + FillWithZeroesAtFront(t.wMinute, 2) + hour_delim + FillWithZeroesAtFront(t.wSecond, 2);
	s += "." + FillWithZeroesAtFront(t.wMilliseconds, 3);
	return s;
}


static wstring string_to_wstring(string s)
{
	wstring ws(s.begin(), s.end());
	return ws;
}


static string wstring_to_string(wstring ws)
{
	string s(ws.begin(), ws.end());
	return s;
}


template<typename First>
// Combine all elements (must not have '\n') into ofstream separated by '\t'
void _PrintLog(ofstream *ofs, const First &arg) {
	ostringstream oss;
	oss << arg;
	string s(oss.str());

	for (const char &c : s)
		if (c != '\n') (*ofs) << c;
}


template<typename First, typename ... Rest>
// Combine all elements (must not have '\n') into ofstream separated by '\t'
void _PrintLog(ofstream *ofs, const First &arg, const Rest&... rest) {
	ostringstream oss;
	oss << arg;
	string s(oss.str());

	for (const char &c : s)
		if (c != '\n') (*ofs) << c;

	(*ofs) << '\t';
	_PrintLog(ofs, rest...);
}


template<typename ... Rest>
// Print all elements to ofstream in one line with time at the start of the line
// and separated by '\t'. No element should contain '\n'.
static void PrintLog(ofstream *ofs, const Rest&... rest) {
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	(*ofs) << PrintSystemTime(stTime, '.', ':') << "\t";
	_PrintLog(ofs, rest...);
	(*ofs) << endl;
}


template<typename First>
// Combine all elements (must not have '\n') into ostream separated by '\t'
void _PrintLog(ostream *ost, const First &arg) {
	ostringstream oss;
	oss << arg;
	string s(oss.str());

	for (const char &c : s)
		if (c != '\n') (*ost) << c;
}


template<typename First, typename ... Rest>
// Combine all elements (must not have '\n') into ostream separated by '\t'
void _PrintLog(ostream *ost, const First &arg, const Rest&... rest) {
	ostringstream oss;
	oss << arg;
	string s(oss.str());

	for (const char &c : s)
		if (c != '\n') (*ost) << c;

	(*ost) << '\t';
	_PrintLog(ost, rest...);
}


template<typename ... Rest>
// Print all elements to ostream in one line with time at the start of the line
// and separated by '\t'. No element should contain '\n'.
static void PrintLog(ostream *ost, const Rest&... rest) {
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);
	(*ost) << PrintSystemTime(stTime, '.', ':') << "\t";
	_PrintLog(ost, rest...);
	(*ost) << endl;
}

