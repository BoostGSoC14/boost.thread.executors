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
    atomic<bool> closed;
    sync_timed_queue<work> workq;
  public:

    ~scheduled_executor() //virtual?
    {
      if(!this->closed.load())
      {
        this->close();
      }
    }

    void close();
    void submit(work);
    void submit_at(work, const time_point&);
    void submit_after(work, const duration&);

  protected:
    scheduled_executor() : closed(false) {}
  };


  template<typename Clock>
  void scheduled_executor<Clock>::close()
  {
    this->closed.store(true);
    this->workq.close();
  }

  template<typename Clock>
  void scheduled_executor<Clock>::submit(work w)
  {
    this->workq.push(w,Clock::now());
  }
  template<typename Clock>
  void scheduled_executor<Clock>::submit_at(work w, const time_point& tp)
  {
    this->workq.push(w,tp);
  }

  template<typename Clock>
  void scheduled_executor<Clock>::submit_after(work w, const duration& dura)
  {
    this->workq.push(w,dura);
  }
}

#endif
