#ifndef SCHEDULED_THREAD_POOL_HPP
#define SCHEDULED_THREAD_POOL_HPP

#include <boost/thread/executors/scheduled_executor.hpp>

namespace boost{

  template <typename Clock = chrono::steady_clock>
  class scheduled_thread_pool : public scheduled_executor<Clock>
  {
  private:
    std::vector<thread> _workers;
  public:

    scheduled_thread_pool(size_t num_threads) : super()
    {
      for(size_t i = 0; i < num_threads; i++)
      {
        this->_workers.push_back(
              boost::move(thread(&scheduled_thread_pool::worker_loop, this))
              );
      }
    }

    ~scheduled_thread_pool()
    {
      this->close();
      for(int i = 0; i < _workers.size(); i++)
      {
        _workers[i].join();
      }
    }

  private:
    typedef scheduled_executor<Clock> super;
    void worker_loop();
  }; //end class

  template<typename Clock>
  void scheduled_thread_pool<Clock>::worker_loop()
  {
    while(!super::_workq.is_closed() || !super::_workq.empty())
    {
      try
      {
        typename super::work fn = super::_workq.pull();
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

