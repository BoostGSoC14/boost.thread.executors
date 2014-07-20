#ifndef SCHEDULING_ADAPTOR_HPP
#define SCHEDULING_ADAPTOR_HPP

#include <boost/thread/executors/executor.hpp>
#include <boost/thread/executors/scheduled_executor.hpp>

namespace boost{

  template <typename Clock = chrono::steady_clock>
  class scheduling_adpator : public scheduled_executor<Clock>
  {
    executor& _exec;
    thread _scheduler;
  public:

    scheduling_adpator(executor& ex)
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

  template<typename Clock>
  void scheduling_adpator<Clock>::scheduler_loop()
  {
    while(!super::_workq.is_closed() || !super::_workq.empty())
    {
      try
      {
        typename super::work fn = this->workq.pull();
        _exec.add(fn);
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

