//+------------------------------------------------------------------+
//|                                         MetaTrader Server Plugin |
//|								  Copyright c 2016, Miyatsu Viet Nam |
//|                                            http://www.miyatsu.vn |
//|		        							   @Author: ThanhVX      |
//+------------------------------------------------------------------+
#include "StdAfx.h"
#include "HttpRequest.h"
#include "verbalexepressions.hpp"

string parseDate2PatternYmd(time_t time);
string parseD2Str(const double dInput);

int HttpRequest::m_randomControl = 1;

HttpRequest::HttpRequest(const wstring &URL)
: m_URL(URL),
m_HttpResponseHeader(L""),
m_HttpResponse(L""),
m_charset(L""),
m_host(L""),
m_resolveTimeout(3000),
m_connectTimeout(3000),
m_sendTimeout(3000),
m_receiveTimeout(3000) {
}

HttpRequest::HttpRequest() {
};

bool HttpRequest::SetProxy(const wstring &proxy) {
	m_proxy = proxy;
	return true;
}

bool HttpRequest::SetUserName(const wstring &username) {
	m_usr = username;
	return true;
}

bool HttpRequest::SetPassword(const wstring &password) {
	m_pwd = password;
	return true;
}

void HttpRequest::setURL(const wstring &URL) {
	m_URL = URL;
}
wstring HttpRequest::getURL() {
	return m_URL;
}

bool HttpRequest::SendHttpRequest(const wstring &httpVerb) {
	// Make verb uppercase.
	wstring verb = httpVerb;
	bool bRetVal = true;
	HINTERNET hSession = NULL;
	
	hSession = ::WinHttpOpen(SZ_AGENT,
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
		
	if(hSession == NULL) {
		return false;
	}
	
	::WinHttpSetTimeouts(hSession,
		m_resolveTimeout,
		m_connectTimeout,
		m_sendTimeout,
		m_receiveTimeout);

	wchar_t szHostName[MAX_PATH] = L"";
	wchar_t szURLPath[MAX_PATH * 5] = L"";
	URL_COMPONENTS urlComp;
	memset(&urlComp, 0, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);
	urlComp.lpszHostName = szHostName;
	urlComp.dwHostNameLength = MAX_PATH;
	urlComp.lpszUrlPath = szURLPath;
	urlComp.dwUrlPathLength = MAX_PATH * 5;
	urlComp.dwSchemeLength = -1;

	if(::WinHttpCrackUrl(m_URL.c_str(), m_URL.size(), 0, &urlComp)) {
		m_host = szHostName;
		HINTERNET hConnect = NULL;
		hConnect = ::WinHttpConnect(hSession, szHostName, urlComp.nPort, 0);
		if(hConnect != NULL) {
			DWORD dwOpenRequestFlag = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
			HINTERNET hRequest = NULL;
			hRequest = ::WinHttpOpenRequest(hConnect,
				verb.c_str(),
				urlComp.lpszUrlPath,
				NULL,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				dwOpenRequestFlag);
			if(hRequest != NULL) {
				bool bGetReponseSucceed = false;
				int iRetryTimes = -1;

				// Retry for several times if fails.
				while(!bGetReponseSucceed && iRetryTimes++ < INT_RETRYTIMES) {
					
					if (iRetryTimes > 0) {
						m_randomControl = (m_randomControl + 3) % 10000000;
						srand(time(NULL) + m_randomControl);						
						int totalDelay = 10 + (rand() % (int)(300 - 10 + 1));//delay from 10 to 600 mili seconds
						Sleep(totalDelay);
					};
					
					if(m_proxy.size() > 0) {
						WINHTTP_PROXY_INFO proxyInfo;
						memset(&proxyInfo, 0, sizeof(proxyInfo));
						proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
						wchar_t szProxy[MAX_PATH] = L"";
						wcscpy_s(szProxy, MAX_PATH, m_proxy.c_str());
						proxyInfo.lpszProxy = szProxy;

						if(!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo))) {
							return false;
						}

						if(m_usr.length() > 0) {
							if(!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID) m_usr.c_str(), m_usr.size() * sizeof(wchar_t))) {
								return false;
							}

						}
						if(m_pwd.length() > 0) {
							if(!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID) m_pwd.c_str(), m_pwd.size() * sizeof(wchar_t))) {
								return false;
							}
						}
					}
					// Send HTTP requeest.
					if(::WinHttpSendRequest(hRequest,
						WINHTTP_NO_ADDITIONAL_HEADERS,
						0,
						WINHTTP_NO_REQUEST_DATA,
						0,
						0,
						NULL)) {
						if(::WinHttpReceiveResponse(hRequest, NULL)) {
							DWORD dwSize = 0;

							// Get the buffer size of the HTTP response header.
							BOOL bResult = ::WinHttpQueryHeaders(hRequest,
								WINHTTP_QUERY_RAW_HEADERS_CRLF,
								WINHTTP_HEADER_NAME_BY_INDEX,
								NULL,
								&dwSize,
								WINHTTP_NO_HEADER_INDEX);
							if(bResult || (!bResult && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER))) {
								wchar_t *szHeader = new wchar_t[dwSize];
								if(szHeader != NULL) {
									memset(szHeader, 0, dwSize* sizeof(wchar_t));

									// Get HTTP response header.
									if(::WinHttpQueryHeaders(hRequest,
										WINHTTP_QUERY_RAW_HEADERS_CRLF,
										WINHTTP_HEADER_NAME_BY_INDEX,
										szHeader,
										&dwSize,
										WINHTTP_NO_HEADER_INDEX)) {
										m_HttpResponseHeader.assign(szHeader);
										_wcslwr(szHeader);
										wstring lwrHeader = szHeader;

										// Try to get charset from HTTP reponse header.
										int iCharsetIndex = lwrHeader.find(L"charset=");
										if(iCharsetIndex != wstring::npos) {
											iCharsetIndex += 8;     // "charset=" has 8 characters.
											int iCharsetLength = 0;
											for(int i = iCharsetIndex; i < lwrHeader.size(); i++) {
												if(i == lwrHeader.size() - 1
													|| lwrHeader[i] == ' '
													|| lwrHeader[i] == ';'
													|| lwrHeader[i] == '\n') {
													iCharsetLength = i - iCharsetIndex;
													break;
												}
											}
											m_charset = lwrHeader.substr(iCharsetIndex, iCharsetLength);
										}
										delete[] szHeader;

										wstring resource = L"";
										do {
											dwSize = 0;
											if(::WinHttpQueryDataAvailable(hRequest, &dwSize)) {
												BYTE *pResponse = new BYTE[dwSize];
												if(pResponse != NULL) {
													memset(pResponse, 0, dwSize);
													DWORD dwRead = 0;
													if(::WinHttpReadData(hRequest,
														pResponse,
														dwSize,
														&dwRead)) {
														// If the web page is encoded in UTF-8, it needs to be takn care of or garbage characters in the string.
														if(m_charset.find(L"utf-8") != wstring::npos) {
															int iLength = ::MultiByteToWideChar(CP_UTF8,
																MB_ERR_INVALID_CHARS,
																(LPCSTR) pResponse,
																dwSize,
																NULL,
																0);
															if(iLength > 0) {
																wchar_t *wideChar = new wchar_t[iLength + 1];
																if(wideChar != NULL) {
																	memset(wideChar, 0, iLength * sizeof(wchar_t));
																	iLength = ::MultiByteToWideChar(CP_UTF8,
																		MB_ERR_INVALID_CHARS,
																		(LPCSTR) pResponse,
																		dwSize,
																		wideChar,
																		iLength);
																	if(iLength > 0) {
																		wchar_t oldChar = wideChar[iLength];
																		wideChar[iLength] = 0;
																		resource.append(wideChar);
																		wideChar[iLength] = oldChar;    // Hack.  Set back the original char or the delete operation leads a crash.
																	}
																	delete[] wideChar;
																}
															}
														} else {
															int iLength = ::MultiByteToWideChar(CP_ACP,
																MB_PRECOMPOSED,
																(LPCSTR) pResponse,
																dwSize,
																NULL,
																0);
															if(iLength > 0) {
																wchar_t *wideChar = new wchar_t[iLength + 1];
																if(wideChar != NULL) {
																	memset(wideChar, 0, iLength * sizeof(wchar_t));
																	iLength = ::MultiByteToWideChar(CP_ACP,
																		MB_PRECOMPOSED,
																		(LPCSTR) pResponse,
																		dwSize,
																		wideChar,
																		iLength);
																	if(iLength > 0) {
																		wchar_t oldChar = wideChar[iLength];
																		wideChar[iLength] = 0;
																		resource.append(wideChar);
																		wideChar[iLength] = oldChar;    // Hack.  Set back the original char or the delete operation leads a crash.
																	}
																	delete[] wideChar;
																}
															}
														}
													}
													delete[] pResponse;
												}
											}
										} while(dwSize > 0);

										/* ThanhVX 2016/03/17: implement adjustment.
										if(resource.size() > 0) {
										bGetReponseSucceed = true;
										m_HttpResponse = resource;
										};*/
										const std::regex pattern("200");
										std::string tmpHeadResult(m_HttpResponseHeader.begin(), m_HttpResponseHeader.end());
										if(std::regex_search(tmpHeadResult, pattern)) {
											bGetReponseSucceed = true;
											m_HttpResponse = resource;
										};
										// End adjustment.
									}
								}
							}
						}
					}
				}
				if(!bGetReponseSucceed) {
					bRetVal = false;
				}

				::WinHttpCloseHandle(hRequest);
			}
			::WinHttpCloseHandle(hConnect);
		}

	}

	::WinHttpCloseHandle(hSession);

	return bRetVal;
}

bool HttpRequest::isPostSuccessed(const wstring headerResult) {
	string header(headerResult.begin(), headerResult.end());

	if(header.length() <= 0) { return false; }

	const std::regex pattern("200");
	return std::regex_search(header, pattern);
}

wstring HttpRequest::GetHttpResponseHeader(void) {
	return m_HttpResponseHeader;
}

wstring HttpRequest::GetHttpResponse(void) {
	return m_HttpResponse;
}

wstring HttpRequest::GetCharset(void) {
	return m_charset;
}

wstring HttpRequest::GetHost(void) {
	return m_host;
}

string HttpRequest::doGet(const wstring &URL) {
	HttpRequest client;
	client.setURL(URL);
	//client.SetProxy(L"172.16.9.11:3128");
	if(client.SendHttpRequest(HTTP_GET)) {
		wstring httpResponseHeader = client.GetHttpResponseHeader();
		wstring httpResponse = client.GetHttpResponse();

		//string headerConv(httpResponseHeader.begin(), httpResponseHeader.end());
		string contentConv(httpResponse.begin(), httpResponse.end());
		return contentConv;
	} else {
		return "";
	}
}

bool HttpRequest::doPost(const wstring &URL) {
	HttpRequest client;
	client.setURL(URL);
	//client.SetProxy(L"172.16.9.11:3128");	
	if (client.SendHttpRequest(HTTP_POST)) {
		wstring httpResponseHeader = client.GetHttpResponseHeader();
		//wstring httpResponse = client.GetHttpResponse();

		string headerConv(httpResponseHeader.begin(), httpResponseHeader.end());
		//string contentConv(httpResponse.begin(), httpResponse.end());
		/*
		if (strlen(contentConv.c_str()) > 0)
			return true;
		else
			return false;
		*/
		const std::regex pattern("200");
		//std::string tmpHeadResult(m_HttpResponseHeader.begin(), m_HttpResponseHeader.end());
		if (std::regex_search(headerConv, pattern)) {
			return true;
		}
		else {
			return false;
		};
	} else {
		return false;
	};
}

bool HttpRequest::isValidURL(const string inputUrl) {
	if(strlen(inputUrl.c_str()) <= 0) return false;

	verex::verex expr = verex::verex()
		.search_one_line()
		.start_of_line()
		.then("http")
		.maybe("s")
		.then("://")
		.maybe("www.")
		.anything_but(" ")
		.then(".")
		.anything_but(" ")
		.end_of_line();

	expr.isParseUrl2regexParttern(inputUrl);
	return expr.isMatched(inputUrl);
}

bool HttpRequest::isValidProxy(const string proxy) {
	if(strlen(proxy.c_str()) <= 0) return false;

	const std::tr1::regex pattern("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5})$");
	return std::tr1::regex_search(proxy, pattern);
}

bool HttpRequest::isValidProxyAuth(const string proxAuth) {
	verex::verex expr = verex::verex()
		.search_one_line()
		.start_of_line()
		.anything_but(" ")
		.end_of_line();

	return expr.isMatched(proxAuth);
}

wstring HttpRequest::getFollowerURL(const int followingLogin, const string baseURL) {
	stringstream ss("");
	//ss.clear();
	//ss.flush();
	ss << baseURL << "?account=" << followingLogin;
	string tmpRes = ss.str();
	wstring resVal(tmpRes.begin(), tmpRes.end());
	ss.clear();
	ss.flush();
	return resVal;
}

string parseDate2PatternYmd(time_t cTime) {
	stringstream strs;
	//std::time_t t = std::time(&cTime);   // get time now
	//struct tm * now = gmtime(&t);
	//time_t displayTime = cTime - timezone * 3600;
	//time_t displayTime = cTime - 0 * 3600;
	time_t displayTime = cTime;//nếu không gán lại thì luôn bị convert theo GMT+0
	struct tm * timeStruct = gmtime(&displayTime);
	//ExtTradeCopyTask.log(CmdOK, LOG_DEBUG, "parseDate2PatternYmd, timezone: %d", timezone);
	int year = timeStruct->tm_year + 1900;
	int day = timeStruct->tm_mday;
	int month = timeStruct->tm_mon + 1;
	int hour = timeStruct->tm_hour;
	int min = timeStruct->tm_min;
	int sec = timeStruct->tm_sec;
	strs << year << ".";
	(month < 10) ? (strs << "0" << month << ".") : (strs << month << ".");
	(day < 10) ? (strs << "0" << day << " ") : (strs << day << " ");
	(hour < 10) ? (strs << "0" << hour << ":") : (strs << hour << ":");
	(min < 10) ? (strs << "0" << min << ":") : (strs << min << ":");
	(sec < 10) ? (strs << "0" << sec) : (strs << sec);

	string output = strs.str();
	return output;
}

string parseD2Str(const double dInput) {
	std::ostringstream strs;
	strs << dInput;
	std::string output = strs.str();
	return output;
}

