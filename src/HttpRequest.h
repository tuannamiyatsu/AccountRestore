//+------------------------------------------------------------------+
//|                                         MetaTrader Server Plugin |
//|								  Copyright c 2016, Miyatsu Viet Nam |
//|                                            http://www.miyatsu.vn |
//|		        							   @Author: ThanhVX      |
//+------------------------------------------------------------------+

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H


using namespace std;

static const int INT_RETRYTIMES = 3;
static const int SIGNAL_MODE = 1;
static const int FOLLOWER_MODE = 2;
static const int COPY_MODE = 3;
static const int BASE_MODE = 4;
static wchar_t *SZ_AGENT = L"HttpRequest";
const wstring HTTP_GET = L"GET";
const wstring HTTP_POST = L"POST";


class HttpRequest
{
public:
	HttpRequest(const wstring &URL);
	HttpRequest();
	bool doPost(const wstring &URL);
	string doGet(const wstring &URL);
	wstring GetHttpResponseHeader(void);
	wstring GetHttpResponse(void);
	wstring GetCharset(void);
	wstring GetHost(void);
	bool SetProxy(const wstring &proxy);
	bool SetUserName(const wstring &username);
	bool SetPassword(const wstring &password);
	bool SetAdditionalDataToSend(BYTE *data, unsigned int dataSize);
	void setURL(const wstring &URL);
	wstring getURL();
	bool SendHttpRequest(const wstring &httpVerb = L"GET");
	static unordered_map<int, double> split(string &s, const char delim) {
		stringstream ss(s);
		std::string item;
		unordered_map<int, double> map;
		unordered_map<int, double>::iterator it = map.begin();
		while (getline(ss, item, delim)) {
			stringstream ss1(item);
			std::string item1;
			int i = 0;
			int login = NULL;
			double magfi = NULL;
			while (getline(ss1, item1, ','))
			{
				if (i == 0) {
					string::size_type sz;
					login = stoi(item1, &sz);
					i++;
				}
				else
				{
					//string::size_type sz;
					magfi = stod(item1.c_str(), NULL);
					map.insert(it, std::make_pair(login, magfi));
					i = 0;
				}
			}
		}
		return map;
	}
	bool isValidURL(const string URL);
	bool isValidProxy(const string proxy);
	bool isValidProxyAuth(const string proxAuth);
	bool isPostSuccessed(const wstring headerResult);

	// End adjustment
	static wstring getFollowerURL(const int followingLogin, const string baseURL);

private:
	HttpRequest(const HttpRequest &other);
	HttpRequest &operator =(const HttpRequest &other);
	// It is a synchronized method and may take a long time to finish.


	wstring m_URL;
	wstring m_HttpResponseHeader;
	wstring m_HttpResponse;
	wstring m_charset;
	wstring m_host;
	wstring m_proxy;
	wstring m_usr;
	wstring m_pwd;

	unsigned int m_resolveTimeout;
	unsigned int m_connectTimeout;
	unsigned int m_sendTimeout;
	unsigned int m_receiveTimeout;
	static   int m_randomControl;
};
#endif // HTTPREQUEST_H
