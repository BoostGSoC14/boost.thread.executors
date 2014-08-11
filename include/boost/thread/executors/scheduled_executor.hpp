#ifndef SCHEDULED_EXECUTOR_HPP
#define SCHEDULED_EXECUTOR_HPP

#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/executors/work.hpp>
#include <boost/thread/detail/sync_timed_queue.hpp>

namespace boost
{
  class scheduled_executor
  {
  public:
    typedef function<void()> work;
    typedef typename chrono::steady_clock clock;
    typedef typename clock::duration duration;
    typedef typename clock::time_point time_point;
  protected:
    sync_timed_queue<work> _workq;

    scheduled_executor() {}
  public:

    ~scheduled_executor() //virtual?
    {
      if(!_workq.is_closed())
      {
        this->close();
      }
    }

    void close()
    {
      _workq.close();
    }

    void submit(work w)
    {
      _workq.push(w, clock::now());
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
