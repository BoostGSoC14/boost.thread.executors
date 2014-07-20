#ifndef SCHEDULED_EXECUTOR_HPP
#define SCHEDULED_EXECUTOR_HPP

#include <functional>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/executors/work.hpp>
#include <boost/thread/sync_timed_queue.hpp>

namespace boost
{

  template<typename Clock = chrono::steady_clock>
  class scheduled_executor
  {
  public:
    typedef std::function<void()> work;
    typedef typename Clock::duration duration;
    typedef typename Clock::time_point time_point;
  protected:
    atomic<bool> _closed;
    sync_timed_queue<work,Clock> _workq;

    scheduled_executor() : _closed(false) {}
  public:

    ~scheduled_executor() //virtual?
    {
      if(!_closed.load())
      {
        this->close();
      }
    }

    void close()
    {
      _closed.store(true);
      _workq.close();
    }

    void submit(work w)
    {
      _workq.push(w, Clock::now());
    }

    void submit_at(work w, const time_point& tp)
    {
      _workq.push(w, tp);
    }

    void submit_after(work w, const duration& dura)
    {
      _workq.push(w, dura);
    }
  }; //end class
} //end namespace

#endif
