#include "pch.h"
#include "ThreadBase.h"

namespace Utility
{
	unsigned int __stdcall CThreadBase::threadFunc(void* arg)
	{
		CThreadBase* thread = (CThreadBase*)arg;
		if (thread == 0)
		{
			return 0; // error.
		}

		try
		{
			thread->run();
		}
		catch (...)
		{
			std::terminate();
		}

		thread->_done();

		return 0;
	}

	CThreadBase::CThreadBase()
		: m_bStarted(false)
		, m_bRunning(false)
		, m_hThread(0)
		, m_dwThreadID(0)
	{
	}

	CThreadBase::~CThreadBase()
	{
	}

	bool CThreadBase::start(size_t stackSize)
	{
		CThreadMutex::Lock guard(m_mtxState);

		if (m_bStarted)
		{
			assert(false);
			return false;
		}

		m_hThread = (HANDLE)_beginthreadex(NULL, (unsigned int)stackSize, threadFunc, this, 0, (unsigned int*)&m_dwThreadID);
		if (0 == m_hThread)
		{
			assert(false);
			return false;
		}

		m_bStarted = true;
		m_bRunning = true;

		return true;
	}

	void CThreadBase::join()
	{
		if (0 == m_hThread)
		{
			return;
		}

		DWORD rc = WaitForSingleObject(m_hThread, INFINITE);
		if (rc != WAIT_OBJECT_0)
		{
			assert(false);
		}

		detach();
	}

	void CThreadBase::detach()
	{
		if (0 == m_hThread)
		{
			return;
		}

		CloseHandle(m_hThread);
	}

	void CThreadBase::sleep(const CJPTime& timeout)
	{
		::Sleep(static_cast<DWORD>(timeout.toMilliSeconds()));
	}

	void CThreadBase::yield()
	{
		::SwitchToThread();
	}

	DWORD CThreadBase::getThreadId() const
	{
		CThreadMutex::Lock guard(m_mtxState);
		return m_dwThreadID;
	}

	bool CThreadBase::isAlive() const
	{
		CThreadMutex::Lock guard(m_mtxState);
		return m_bRunning;
	}

	void CThreadBase::_done()
	{
		CThreadMutex::Lock guard(m_mtxState);
		m_bRunning = false;
	}

}// namespace Utility