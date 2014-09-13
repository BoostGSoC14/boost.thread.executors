#ifndef SCHEDULING_ADAPTOR_HPP
#define SCHEDULING_ADAPTOR_HPP

#include <boost/thread/detail/scheduled_executor_base.hpp>

namespace boost{

  template <typename Executor>
  class scheduling_adpator : public detail::scheduled_executor_base
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

    Executor& underlying_executor()
    {
        return _exec;
    }

  private:
    typedef scheduled_executor_base super;
    void scheduler_loop();
  }; //end class

  template<typename Executor>
  void scheduling_adpator<Executor>::scheduler_loop()
  {
    while(!super::_workq.is_closed() || !super::_workq.empty())
    {
      try
      {
        super::work fn = super::_workq.pull();
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
