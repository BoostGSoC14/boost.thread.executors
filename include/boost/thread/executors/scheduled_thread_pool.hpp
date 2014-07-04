#ifndef SCHEDULER_ADAPTOR
#define SCHEDULER_ADAPTOR

#include <boost/thread/executors/scheduled_executor.hpp>

namespace boost{

  template <typename Clock = chrono::steady_clock>
  class scheduled_thread_pool : public scheduled_executor<Clock>
  {
    std::vector<thread> _workers;
  public:

    scheduled_thread_pool(size_t num_threads) : super()
    {
      for(size_t i = 0; i < num_threads; i++)
      {
        this->_workers.push_back(
          boost::move(thread(&scheduled_thread_pool::worker_loop,this))
        );
      }
    }

    ~scheduled_thread_pool()
    {
      this->closed.store(true);
      this->workq.close();
      for(int i = 0; i < _workers.size(); i++)
      {
        this->_workers[i].join();
      }
    }

  private:
    typedef scheduled_executor<Clock> super;
    void worker_loop();
  };

  template<typename Clock>
  void scheduled_thread_pool<Clock>::worker_loop()
  {
    while(!this->closed.load() || !this->workq.empty())
    {
      try
      {
        typename super::work fn = this->workq.pull();
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

