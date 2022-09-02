#include "pch.h"
#include "JPTime.h"

#ifdef JP_WIN32
#   include <sys/timeb.h>
#   include <time.h>
#endif

namespace Utility
{

	CJPTime::CJPTime() :
		_msec(0)
	{
	}

	CJPTime
		CJPTime::now()
	{
#ifdef JP_WIN32
		struct _timeb tb;
		_ftime(&tb);
#endif
		return CJPTime(static_cast<int64>(tb.time) * JP_INT64(1000) + tb.millitm);
	}

	CJPTime
		CJPTime::makeTime(int year, int month, int day,
			int hour, int minute, int second, int milliSecond)
	{
		struct tm tmLocal;
		memset(&tmLocal, 0, sizeof(tmLocal));
		tmLocal.tm_sec = second;
		tmLocal.tm_min = minute;
		tmLocal.tm_hour = hour;
		tmLocal.tm_mday = day;
		tmLocal.tm_mon = month - 1;
		tmLocal.tm_year = year - 1900;
		time_t elapSecond = mktime(&tmLocal);

		return CJPTime(static_cast<int64>(elapSecond) * JP_INT64(1000) + milliSecond);
	}

	CJPTime
		CJPTime::seconds(int64 t)
	{
		return CJPTime(t * JP_INT64(1000));
	}

	CJPTime
		CJPTime::milliSeconds(int64 t)
	{
		return CJPTime(t);
	}

	int64
		CJPTime::toSeconds() const
	{
		return _msec / 1000;
	}

	int64
		CJPTime::toMilliSeconds() const
	{
		return _msec;
	}

	std::string
		CJPTime::toDateTime() const
	{
		time_t time = static_cast<long>(_msec / 1000);

		struct tm* t;
#ifdef JP_WIN32
		t = localtime(&time);
#endif

		char buf[32];
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);

		std::ostringstream os;
		os << buf;
		return os.str();
	}

	std::string
		CJPTime::toDateTimeAccuracy() const
	{
		time_t time = static_cast<long>(_msec / 1000);

		struct tm* t;
#ifdef JP_WIN32
		t = localtime(&time);
#endif

		char buf[32];
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);

		std::ostringstream os;
		os << buf << ".";
		os.fill('0');
		os.width(3);
		os << static_cast<long>(_msec % 1000);
		return os.str();
	}

	CJPTime::CJPTime(int64 usec) :
		_msec(usec)
	{
	}

}
