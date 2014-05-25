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
    sync_priority_queue() {}
    ~sync_priority_queue() {}
    BOOST_THREAD_NO_COPYABLE(sync_priority_queue)

    bool empty() const;
    std::size_t size() const;
    ValueType pull();
    void push(const ValueType& elem);

    bool try_pull(ValueType& dest); //Time point wait on mutex?
    bool try_pull_no_wait(ValueType& dest);
    bool try_push(const ValueType& elem);

  protected:
    mutable mutex q_mutex_;
    condition_variable is_empty_;
    std::priority_queue<ValueType> pq_;
  }; //end class

  template<typename ValueType>
  bool sync_priority_queue<ValueType>::empty() const
  {
    lock_guard<mutex> lk(q_mutex_);
    return pq_.empty();
  }

  template<typename ValueType>
  size_t sync_priority_queue<ValueType>::size() const
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

  template<typename ValueType>
  bool sync_priority_queue<ValueType>::try_pull(ValueType & dest)
  {
    unique_lock<mutex> lk(q_mutex_, try_to_lock);
    if(lk.owns_lock())
    {
      while(pq_.empty())
      {
        is_empty_.wait(lk);
      }
      dest = pq_.top();
      pq_.pop();
      return true;
    }
    return false;
  }

  template<typename ValueType>
  bool sync_priority_queue<ValueType>::try_pull_no_wait(ValueType & dest)
  {
    unique_lock<mutex> lk(q_mutex_, try_to_lock);
    if(lk.owns_lock() && !pq_.empty())
    {
      dest = pq_.top();
      pq_.pop();
      return true;
    }
    return false;
  }

  template<typename ValueType>
  bool sync_priority_queue<ValueType>::try_push(const ValueType & elem)
  {
    if(q_mutex_.try_lock())
    {
      pq_.push(elem);
      q_mutex_.unlock();
      is_empty_.notify_one();
      return true;
    }
    return false;
  }

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
