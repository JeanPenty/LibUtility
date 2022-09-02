#include "pch.h"
#include "TaskTimerThread.h"

namespace Utility
{
	CTaskTimerThread::CTaskTimerThread(const std::string sThreadName, int nMaxLen)
		:m_sThreadName(sThreadName), m_nMaxLen(nMaxLen)
	{
		terminated_ = false;
		timeout_ = CJPTime::milliSeconds(10);
		exit_task_ = NULL;
	};

	CTaskTimerThread::~CTaskTimerThread()
	{
		vt_new_timer_.clear();
		list_timer_.clear();
	};

	void CTaskTimerThread::terminate()
	{
		terminated_ = true;
		MonitorT<CThreadMutex>::Lock guard(monitor_);
		monitor_.notify();
	};

	int CTaskTimerThread::add_task(const TaskPtr& task)
	{
		MonitorT<CThreadMutex>::Lock guard(monitor_);

		int size = (int)deque_task_.size();
		if (size >= m_nMaxLen)
		{
			return  size;
		}

		if (deque_task_.empty())
			monitor_.notify();
		deque_task_.push_back(task);
		size = (int)deque_task_.size();
		return size;
	};

	int CTaskTimerThread::set_exit_task(const TaskPtr& task)
	{
		exit_task_ = task;
		return 0;
	}

	int CTaskTimerThread::get_task_size()
	{
		MonitorT<CThreadMutex>::Lock guard(monitor_);
		int size = (int)deque_task_.size();

		return size;
	}

	void CTaskTimerThread::create_timer(const TimerPtr& timer)
	{
		CThreadMutex::Lock guard(lock_new_timer_);
		vt_new_timer_.push_back(timer);
	};

	void CTaskTimerThread::destory_timer(const TimerPtr& timer)
	{
		CThreadMutex::Lock guard(lock_del_timer_);
		vt_del_timer_.push_back(timer);
	};

	void CTaskTimerThread::run()
	{
		now_ = CJPTime::now();
		timestamp_ = now_;

		TaskPtr task;

		while (!terminated_)
		{
			beforeProcessTask();

			{
				/* 注意Lock的作用范围，防止重复加锁 */
				MonitorT<CThreadMutex>::Lock guard(monitor_);
				if (deque_task_.empty())
				{
					monitor_.timedWait(timeout_);
				}

				now_ = CJPTime::now();
				if (!deque_task_.empty())
				{
					task = deque_task_.front();
					deque_task_.pop_front();
				}
			}

			if (terminated_)
				break;

			if (task != NULL)
			{
				try
				{
					TaskExPtr taskEx = TaskExPtr::__dynamic_cast(task);
					if (taskEx != NULL)
					{
						process(taskEx);
					}
					else
					{
						task->dotask();
					}
				}
				catch (...)
				{
				}
				task = NULL;
			}

			afterProcessTask();

			try
			{
				ProcessTimer();
			}
			catch (...)
			{
			}
		}

		if (exit_task_ != NULL)
		{
			try
			{
				TaskExPtr taskEx = TaskExPtr::__dynamic_cast(exit_task_);
				if (taskEx != NULL)
				{
					process(taskEx);
				}
				else
				{
					exit_task_->dotask();
				}
			}
			catch (...)
			{
			}
			exit_task_ = NULL;
		}

	};

	bool CTaskTimerThread::ProcessTimer()
	{
		if (now_ <= timestamp_)
		{
			timestamp_ = now_;
			return false;
		}

		if (!vt_new_timer_.empty())
		{
			CThreadMutex::Lock guard(lock_new_timer_);

			for (std::vector<TimerPtr>::iterator it = vt_new_timer_.begin(); it != vt_new_timer_.end(); ++it)
			{
				if ((*it) != NULL)
				{
					list_timer_.push_back(*it);
				}
			}
			vt_new_timer_.clear();
		}

		if (!vt_del_timer_.empty())
		{
			CThreadMutex::Lock guard(lock_del_timer_);

			for (std::vector<TimerPtr>::iterator it = vt_del_timer_.begin(); it != vt_del_timer_.end(); ++it)
			{
				list_timer_.remove(*it);
			}
			vt_del_timer_.clear();
		}

		now_ = CJPTime::now();

		for (std::list<TimerPtr>::iterator it = list_timer_.begin(); it != list_timer_.end(); ++it)
		{
			if (NULL != (*it))
			{
				if ((*it)->is_terminate())
				{
					destory_timer(*it);
					continue;
				}
				(*it)->inc_elapse(now_ - timestamp_);
				CJPTime realInterval = (*it)->is_delayed() ? (*it)->get_interval() : (*it)->get_delay();
				if ((*it)->get_elapse() >= realInterval)
				{
					(*it)->timeout();
					if (false == (*it)->is_delayed())
					{
						(*it)->set_delayed(true);
					}
					if ((*it)->is_loop())
					{
						(*it)->resume();
					}
					else
					{
						(*it)->terminate();
					}
				}
			}
		}
		timestamp_ = now_;

		return true;
	};

}// namespace Utility