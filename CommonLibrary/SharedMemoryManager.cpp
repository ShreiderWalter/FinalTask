#include "SharedMemoryManager.h"

std::shared_ptr<SharedMemoryManager> SharedMemoryManager::self;

SharedMemoryManager::SharedMemoryManager()
{
}

SharedMemoryManager::SharedMemoryManager(const std::string & name)
{
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	InitializeSecurityAttributesForEverybodyAccess(&sa, &sd);


	m_sharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE,
						&sa,
						PAGE_READWRITE,
						0,
						BUFFER_SIZE,
						name.c_str());

	//m_eventLock = CreateEvent(nullptr, false, false, EVENT_NAME);

	m_eventWrite = CreateEvent(nullptr, false, false, EVENT_WRITE_NAME);

	m_buffer = (LPTSTR) MapViewOfFile(m_sharedMemory, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);

	m_mutex = CreateMutex(nullptr, FALSE, MUTEX_NAME);
}

void SharedMemoryManager::InitializeSecurityAttributesForEverybodyAccess(SECURITY_ATTRIBUTES * pSecurityAttributes, 
		SECURITY_DESCRIPTOR * pSecurityDescriptor)
{
	memset(pSecurityAttributes, NULL, sizeof(SECURITY_ATTRIBUTES));
	pSecurityAttributes->bInheritHandle = true;
	pSecurityAttributes->lpSecurityDescriptor = pSecurityDescriptor;
	pSecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
	InitializeSecurityDescriptor(pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(pSecurityDescriptor, true, nullptr, false);

}

std::shared_ptr<SharedMemoryManager> SharedMemoryManager::create(const std::string &name) 
{
	self = std::shared_ptr<SharedMemoryManager>(new SharedMemoryManager(name));
	return self;
}

std::shared_ptr<SharedMemoryManager> SharedMemoryManager::connect(const std::string &name)
{
	self = std::shared_ptr<SharedMemoryManager>(new SharedMemoryManager);
	self->m_sharedMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, name.c_str());
	self->m_eventWrite = OpenEvent(EVENT_ALL_ACCESS, false, EVENT_WRITE_NAME);
	self->m_buffer = (LPTSTR) MapViewOfFile(self->m_sharedMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, BUFFER_SIZE);
	std::cout << GetLastError();
	self->m_mutex = OpenMutex(FILE_MAP_READ | FILE_MAP_WRITE, false, MUTEX_NAME);

	return self;
}

bool SharedMemoryManager::isEmpty() const
{
	return nullptr == m_sharedMemory;
}

void SharedMemoryManager::close()
{
	CloseHandle(m_sharedMemory);
}

void SharedMemoryManager::write(const char * tmp)
{
	WaitForSingleObject(m_mutex, INFINITE);
	CopyMemory((PVOID)m_buffer, tmp,  (_tcslen(tmp)) * sizeof(TCHAR));
	ReleaseMutex(m_mutex);
}

void SharedMemoryManager::write(unsigned char * buffer, DWORD bufferSize)
{
	WaitForSingleObject(m_mutex, INFINITE);
	CopyMemory((PVOID)m_buffer, buffer, bufferSize);
	FlushViewOfFile(m_buffer, BUFFER_SIZE);
	ReleaseMutex(m_mutex);
}

char * SharedMemoryManager::read()
{
	WaitForSingleObject(m_mutex, INFINITE);
	char * tmp = (char *) m_buffer;
	ReleaseMutex(m_mutex);
	SetEvent(m_eventWrite);

	return tmp;
}

void SharedMemoryManager::wait()
{
	WaitForSingleObject(m_eventWrite, INFINITE);
}