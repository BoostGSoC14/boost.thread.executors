#ifndef BOOST_THREAD_SYNC_PRIORITY_QUEUE
#define BOOST_THREAD_SYNC_PRIORITY_QUEUE

#include <queue>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <boost/config/abi_prefix.hpp>

namespace boost
{

  template <typename ValueType>
  class sync_priority_queue
  {
  public:
    sync_priority_queue() : closed_(false) {}
    ~sync_priority_queue() {}
    BOOST_THREAD_NO_COPYABLE(sync_priority_queue)

    bool empty();
    std::size_t size();
    ValueType pull();
    void push(const ValueType&);

    bool try_push(const ValueType&);
    bool try_pull(ValueType& dest);

  private:
    atomic<bool> closed_;
    mutex q_mutex_;
    condition_variable is_empty_;
    std::priority_queue<ValueType> pq_;
  }; //end class

  template<typename ValueType>
  bool sync_priority_queue<ValueType>::empty()
  {
    lock_guard<mutex> lk(q_mutex_);
    return pq_.empty();
  }

  template<typename ValueType>
  size_t sync_priority_queue<ValueType>::size()
  {
    //lock_guard<mutex> lk(q_mutex_);
    return pq_.size();
  }

  template<typename ValueType>
  ValueType sync_priority_queue<ValueType>::pull()
  {
    unique_lock<mutex> lk(q_mutex_);
    while(pq_.empty())
    {
      is_empty_.wait(lk);
    }
    ValueType first = pq_.top();
    pq_.pop();
    return first;
  }

  template<typename ValueType>
  void sync_priority_queue<ValueType>::push(const ValueType & elem)
  {
    lock_guard<mutex> lk(q_mutex_);
    pq_.push(elem);
    is_empty_.notify_one();
  }

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
