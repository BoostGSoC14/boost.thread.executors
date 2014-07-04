#ifndef SCHEDULER_ADAPTOR
#define SCHEDULER_ADAPTOR

#include <boost/thread/executors/executor.hpp>
#include <boost/thread/executors/scheduled_executor.hpp>

namespace boost{

  template <typename Clock = chrono::steady_clock>
  class scheduler_adpator : public scheduled_executor<Clock>
  {
    executor& _exec;
    thread scheduler;
  public:

    scheduler_adpator(executor& ex)
      : super(),
        _exec(ex),
        scheduler(&scheduler_adpator::scheduler_loop,this) {}

    ~scheduler_adpator()
    {
      this->closed.store(true);
      this->workq.close();
      this->scheduler.join();
    }

  private:
    typedef scheduled_executor<Clock> super;
    void scheduler_loop();
  };

  template<typename Clock>
  void scheduler_adpator<Clock>::scheduler_loop()
  {
    while(!this->closed.load() || !this->workq.empty())
    {
      try
      {
        typename super::work fn = this->workq.pull();
        this->_exec.add(fn);
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

