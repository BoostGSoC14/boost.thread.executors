#include <functional>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/sync_timed_queue.hpp>

namespace boost
{
  template<typename Clock>
  class scheduled_executor
  {
  public:
    typedef std::function<void()> work;
    typedef typename Clock::duration duration;
    typedef typename Clock::time_point time_point;
  private:
    atomic<bool> closed;
    thread scheduler;
    sync_timed_queue<work> workq;
  public:
      scheduled_executor()
        : scheduler(&scheduled_executor::scheduler_loop,this) {}
      ~scheduled_executor();
      void close();
      void submit(work);
      void submit_at(work, time_point);
      void submit_after(work, duration);

  private:
      void scheduler_loop();
  };

  template<typename Clock>
  void scheduled_executor<Clock>::scheduler_loop()
  {
    while(!this->closed.load())
    {
      try
      {
        std::function<void()> fn = this->workq.pull();
        fn();
      }
      catch(std::exception&)
      {
        return;
      }
    }
  }

  template<typename Clock>
  void scheduled_executor<Clock>::close()
  {
    this->closed.store(true);
    scheduler.join();
  }

  template<typename Clock>
  void scheduled_executor<Clock>::submit(work w)
  {
    this->workq.push(w,Clock::now());
  }
  template<typename Clock>
  void scheduled_executor<Clock>::submit_at(work w, time_point tp)
  {
    this->workq.push(w,tp);
  }

  template<typename Clock>
  void scheduled_executor<Clock>::submit_after(work w, duration dura)
  {
    this->workq(w,dura);
  }
}
