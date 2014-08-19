#ifndef SCHEDULED_THREAD_POOL_HPP
#define SCHEDULED_THREAD_POOL_HPP

#include <boost/thread/executors/scheduled_executor.hpp>

namespace boost
{
  class scheduled_thread_pool : public scheduled_executor
  {
  private:
    thread_group _workers;
  public:

    scheduled_thread_pool(size_t num_threads) : super()
    {
      for(size_t i = 0; i < num_threads; i++)
      {
        _workers.create_thread(bind(&scheduled_thread_pool::worker_loop, this));
      }
    }

    ~scheduled_thread_pool()
    {
      this->close();
      _workers.join_all();
    }

  private:
    typedef scheduled_executor super;
    void worker_loop();
  }; //end class

  void scheduled_thread_pool::worker_loop()
  {
    while(!super::_workq.is_closed() || !super::_workq.empty())
    {
      try
      {
        super::work fn = super::_workq.pull();
        fn();
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

