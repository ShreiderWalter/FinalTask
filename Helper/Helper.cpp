#include "Helper.h"

Helper::Helper()
{
	m_manager = std::shared_ptr<SharedMemoryManager>
		(SharedMemoryManager::create(MAPPING_OBJECT_NAME).get());
}

Helper::~Helper()
{
}

void Helper::threadProgress()
{
	while(true)
	{
		BYTE * buffer = nullptr;
		int bufferSize = 0;
		buffer = ScreenUtils::myGetDibBits24(bufferSize);
		std::string msg = std::to_string(bufferSize);
		std::cout << "Size : " << msg << "\n";

		m_manager->wait();
		m_manager->write(msg.c_str());
		m_manager->wait();
		m_manager->write(buffer, bufferSize);
		delete buffer;
	}
}

void Helper::run()
{
	std::thread thread(&Helper::threadProgress, this);
	thread.join();
}