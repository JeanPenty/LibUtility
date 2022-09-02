#include "pch.h"
#include "ThreadRecMutex.h"

#ifdef JP_WIN32

Utility::CThreadRecMutex::CThreadRecMutex() :
	_count(0)
{
	InitializeCriticalSection(&_mutex);
}

Utility::CThreadRecMutex::~CThreadRecMutex()
{
	assert(_count == 0);
	DeleteCriticalSection(&_mutex);
}

void
Utility::CThreadRecMutex::lock() const
{
	EnterCriticalSection(&_mutex);
	if (++_count > 1)
	{
		LeaveCriticalSection(&_mutex);
	}
}

bool
Utility::CThreadRecMutex::tryLock() const
{
	if (!TryEnterCriticalSection(&_mutex))
	{
		return false;
	}
	if (++_count > 1)
	{
		LeaveCriticalSection(&_mutex);
	}
	return true;
}

void
Utility::CThreadRecMutex::unlock() const
{
	if (--_count == 0)
	{
		LeaveCriticalSection(&_mutex);
	}
}

void
Utility::CThreadRecMutex::unlock(LockState& state) const
{
	state.count = _count;
	_count = 0;
	LeaveCriticalSection(&_mutex);
}

void
Utility::CThreadRecMutex::lock(LockState& state) const
{
	EnterCriticalSection(&_mutex);
	_count = state.count;
}

#endif

bool
Utility::CThreadRecMutex::willUnlock() const
{
	return _count == 1;
}
