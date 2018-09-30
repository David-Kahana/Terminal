#pragma once
#include <string>
#include <vector>

using namespace std;

#define OK 1

class CFileUtil
{
public:
	CFileUtil();
	~CFileUtil();
	static bool fileExists(const wstring& fileName_in);
	static bool fileExists(const string& fileName_in);
	static int writeToFile(const wstring& fileName, const char* buffer, unsigned int size);
	static int writeToFile(const string& fileName, const char* buffer, unsigned int size);
	static int readFromFile(const wstring& fileName, char** buffer, unsigned int& size);
	static int readFromFile(const string& fileName, char** buffer, unsigned int& size);
	static int getFilesInDir(const string& dirName, vector<string>& files);
	static int getFilesInDir(const wstring& dirName, vector<wstring>& files);
	static void uSleep(const uint32_t millis);
private:
	static int readFromFile(const wchar_t* fileName, char** buffer, unsigned int& size);
	static int writeToFile(const wchar_t* fileName, const char* buffer, unsigned int size);
};

