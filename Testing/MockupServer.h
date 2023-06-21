#pragma once

#include <filesystem>
#include <config.h>


#include <tlhelp32.h>

class VhubMockup {


public:
	static void StartServer() {

		char* processName = (char*)"BmdVideoHub.exe";

		if (CheckProcessRunning(processName) != 0)
		{
			return;
		}

		std::string path = VHUB_MOCK_DIR;
		std::string mock_path = path + "/BmdVideoHub.exe";
		std::string command = "start " + mock_path;

		std::system(command.c_str());
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

private:

	static DWORD CheckProcessRunning(char* processName)
	{
		// strip path

		char* p = strrchr(processName, '\\');
		if (p)
			processName = p + 1;

		PROCESSENTRY32 processInfo;
		processInfo.dwSize = sizeof(processInfo);

		HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (processesSnapshot == INVALID_HANDLE_VALUE)
			return 0;

		Process32First(processesSnapshot, &processInfo);
		if (!strcmp(processName, processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}

		while (Process32Next(processesSnapshot, &processInfo))
		{
			if (!strcmp(processName, processInfo.szExeFile))
			{
				CloseHandle(processesSnapshot);
				return processInfo.th32ProcessID;
			}
		}

		CloseHandle(processesSnapshot);
		return 0;
	}

};