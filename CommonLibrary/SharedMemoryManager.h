#ifndef SHARED_MEMORY_MANAGER
#define SHARED_MEMORY_MANAGER

#include <Windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <tchar.h>
#include <Sddl.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\Pipe")
#define BUFFER_SIZE 4403200
#define MAPPING_OBJECT_NAME TEXT("Global\\MyMappingObject")
#define EVENT_NAME TEXT("Global\\AcceptLock")
#define EVENT_WRITE_NAME TEXT("Global\\WriteLock")
#define MUTEX_NAME TEXT("Global\\MutexLock")

class SharedMemoryManager
{
private:
	SharedMemoryManager();
	SharedMemoryManager(const std::string & name);
	HANDLE m_sharedMemory;
	HANDLE m_eventLock;
	HANDLE m_eventWrite;
	LPTSTR m_buffer;
	HANDLE m_mutex;
	static std::shared_ptr<SharedMemoryManager> self;

	static void InitializeSecurityAttributesForEverybodyAccess(SECURITY_ATTRIBUTES * pSecurityAttributes, 
		SECURITY_DESCRIPTOR * pSecurityDescriptor);

public:
	static std::shared_ptr<SharedMemoryManager> create(const std::string & name);
	static std::shared_ptr<SharedMemoryManager> connect(const std::string & name);
	void close();

	char * read();
	void write(const char * tmp);
	void write(unsigned char * buffer, DWORD bufferSize);
	void wait();
	bool isEmpty() const;
};

#endif // SHARED_MEMORY_MANAGER