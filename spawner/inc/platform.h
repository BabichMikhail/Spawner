#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef _WIN32

#include <Windows.h>

typedef HANDLE thread_t;

#define thread_return_t DWORD WINAPI
#define thread_param_t LPVOID

#define COMPLETION_KEY 1

#define JOB_OBJECT_MSG_PROCESS_WRITE_LIMIT 11
#define SECOND_COEFF 10000

typedef PROCESS_INFORMATION process_info_t;
typedef HANDLE pipe_t;

void CloseHandleSafe(HANDLE &handle);
#else

#endif//_WIN32

#endif//_PLATFORM_H_