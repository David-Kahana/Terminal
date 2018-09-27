#include "windows.h"
#include <iostream>
#include <atlbase.h>
#include <atlconv.h>
#include <filesystem>
#include "FileUtil.h"


CFileUtil::CFileUtil()
{
}

CFileUtil::~CFileUtil()
{
}

bool CFileUtil::fileExists(const wstring& fileName_in)
{
	DWORD dwAttrib = GetFileAttributes(fileName_in.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool CFileUtil::fileExists(const string& fileName_in)
{
	DWORD dwAttrib = GetFileAttributes(CA2W(fileName_in.c_str()));
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int CFileUtil::writeToFile(const wstring& fileName, const char* buffer, unsigned int size)
{
	return writeToFile(fileName.c_str(), buffer, size);
	//int ret = OK;
	//FILE *file = _wfopen(fileName.c_str(), L"wb");
	//if (file == nullptr)
	//{
	//	return -1;
	//}
	//size_t wsize = fwrite(buffer, 1, size, file);
	//if (wsize != (size_t)size)
	//{
	//	ret = -2;
	//}
	//int cret = fclose(file);
	//if (cret != 0)
	//{
	//	ret = -3;
	//}
	//return ret;
}

int CFileUtil::writeToFile(const string& fileName, const char* buffer, unsigned int size)
{
	return writeToFile(CA2W(fileName.c_str()), buffer, size);
	//int ret = OK;
	//FILE *file = fopen(fileName.c_str(), "wb");
	//if (file == nullptr)
	//{
	//	return -1;
	//}
	//size_t wsize = fwrite(buffer, 1, size, file);
	//if (wsize != (size_t)size)
	//{
	//	ret = -2;
	//}
	//int cret = fclose(file);
	//if (cret != 0)
	//{
	//	ret = -3;
	//}
	//return ret;
}

int CFileUtil::readFromFile(const string& fileName, char** buffer, unsigned int& size)
{
	return readFromFile(CA2W(fileName.c_str()), buffer, size);
}

int CFileUtil::readFromFile(const wstring& fileName, char** buffer, unsigned int& size)
{
	return readFromFile(fileName.c_str(), buffer, size);
}

int CFileUtil::getFilesInDir(const string& dirName, vector<string>& files)
{
	for (auto & p : filesystem::directory_iterator(dirName))
	{
		//string str = p.path().generic_string();
		files.push_back(p.path().filename().generic_string());
		//files.push_back(p.path().generic_string());
		//cout << str << std::endl;
	}
	return OK;
}

int CFileUtil::getFilesInDir(const wstring& dirName, vector<wstring>& files)
{
	for (auto & p : filesystem::directory_iterator(dirName))
	{
		cout << p << std::endl;
	}
	return OK;
}

int CFileUtil::readFromFile(const wchar_t* fileName, char** buffer, unsigned int& size)
{
	*buffer = nullptr;
	HANDLE hfile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	LARGE_INTEGER li;
	BOOL sret = GetFileSizeEx(hfile, &li);
	if (sret == FALSE)
	{
		CloseHandle(hfile);
		return -2;
	}
	LONGLONG bufSize = li.QuadPart;
	if (bufSize > (LONGLONG)MAXDWORD - 1)
	{ //too big, not implemented yet
		CloseHandle(hfile);
		return -3;
	}
	char* inBuf = nullptr;
	inBuf = new char[bufSize + 1];
	if (inBuf == nullptr)
	{
		CloseHandle(hfile);
		return -4;
	}
	DWORD dwBytesRead = 0;
	
	BOOL rret = ReadFile(hfile, inBuf, (DWORD)bufSize, &dwBytesRead, NULL);
	if (rret == FALSE || dwBytesRead != (DWORD)bufSize)
	{
		delete[]inBuf;
		CloseHandle(hfile);
		return -5;
	}
	inBuf[bufSize] = 0;
	*buffer = inBuf;
	size = dwBytesRead + 1;
	CloseHandle(hfile);
	//HANDLE hfile = (HANDLE)_get_osfhandle(_fileno(file));
	return OK;
}

int CFileUtil::writeToFile(const wchar_t* fileName, const char* buffer, unsigned int size)
{
	HANDLE hfile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	DWORD numberOfBytesWritten = 0;
	BOOL wret = WriteFile(hfile, buffer, (DWORD)size, &numberOfBytesWritten, NULL);
	if (wret == FALSE || numberOfBytesWritten != (DWORD)size)
	{
		CloseHandle(hfile);
		return -2;
	}
	CloseHandle(hfile);
	return OK;
}