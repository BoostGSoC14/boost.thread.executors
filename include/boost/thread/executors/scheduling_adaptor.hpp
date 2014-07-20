#ifndef SCHEDULING_ADAPTOR_HPP
#define SCHEDULING_ADAPTOR_HPP

#include <boost/thread/executors/scheduled_executor.hpp>

namespace boost{

  template <typename Executor, typename Clock = chrono::steady_clock>
  class scheduling_adpator : public scheduled_executor<Clock>
  {
  private:
    Executor& _exec;
    thread _scheduler;
  public:

    scheduling_adpator(Executor& ex)
      : super(),
        _exec(ex),
        _scheduler(&scheduling_adpator::scheduler_loop, this) {}

    ~scheduling_adpator()
    {
      this->close();
      _scheduler.join();
    }

  private:
    typedef scheduled_executor<Clock> super;
    void scheduler_loop();
  }; //end class

  template<typename Executor, typename Clock>
  void scheduling_adpator<Executor,Clock>::scheduler_loop()
  {
    while(!super::_workq.is_closed() || !super::_workq.empty())
    {
      try
      {
        typename super::work fn = super::_workq.pull();
        _exec.submit(fn);
      }
      catch(std::exception& err)
      {
        // debug std::err << err.what() << std::endl;
        return;
      }
    }
  }
} //end boost
#endif
