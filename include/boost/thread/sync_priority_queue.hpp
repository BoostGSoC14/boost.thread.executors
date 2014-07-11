#ifndef BOOST_THREAD_SYNC_PRIORITY_QUEUE
#define BOOST_THREAD_SYNC_PRIORITY_QUEUE

#include <queue>
#include <exception>

#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <boost/chrono/duration.hpp>
#include <boost/chrono/time_point.hpp>

#include <boost/optional.hpp>

#include <boost/config/abi_prefix.hpp>

namespace boost
{

  template <typename ValueType>
  class sync_priority_queue
  {
  public:
    sync_priority_queue() : closed(false) {}
    ~sync_priority_queue()
    {
      if(!this->closed.load())
      {
        this->close();
      }
    }

    bool empty() const;
    void close();

    bool is_closed() const { return closed.load(); }

    std::size_t size() const;

    void push(const ValueType& elem);
    bool try_push(const ValueType& elem);

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    void push(ValueType&& elem);
    bool try_push(ValueType&& elem);
#endif

    ValueType pull();
    optional<ValueType> pull_until(chrono::steady_clock::time_point);
    optional<ValueType> pull_for(chrono::steady_clock::duration);
    optional<ValueType> pull_no_wait();

    optional<ValueType> try_pull(); //Time point wait on mutex?
    optional<ValueType> try_pull_no_wait();


  protected:
    atomic<bool> closed;
    mutable mutex q_mutex_;
    condition_variable is_empty_;
    std::priority_queue<ValueType> pq_;

  private:
    sync_priority_queue(const sync_priority_queue&);
    sync_priority_queue& operator= (const sync_priority_queue&);
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    sync_priority_queue(sync_priority_queue&&);
    sync_priority_queue& operator= (sync_priority_queue&&);
#endif
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
  void sync_priority_queue<ValueType>::close()
  {
    closed.store(true);
    is_empty_.notify_all();
  }

  template<typename ValueType>
  ValueType sync_priority_queue<ValueType>::pull()
  {
    unique_lock<mutex> lk(q_mutex_);
    while(pq_.empty())
    {
      if(closed.load()) throw std::exception();
      is_empty_.wait(lk);
    }
    ValueType first = pq_.top();
    pq_.pop();
    return first;
  }

  template<typename ValueType>
  optional<ValueType>
  sync_priority_queue<ValueType>::pull_until(chrono::steady_clock::time_point tp)
  {
    unique_lock<mutex> lk(q_mutex_);
    while(pq_.empty())
    {
      if(closed.load()) throw std::exception();
      if(is_empty_.wait_until(lk, tp) == cv_status::timeout )
      {
        return optional<ValueType>();
      }
    }
    optional<ValueType> fst( pq_.top() );
    pq_.pop();
    return fst;
  }

  template<typename ValueType>
  optional<ValueType>
  sync_priority_queue<ValueType>::pull_for(chrono::steady_clock::duration dura)
  {
    return pull_until(chrono::steady_clock::now() + dura);
  }

  template<typename ValueType>
  optional<ValueType> sync_priority_queue<ValueType>::pull_no_wait()
  {
    unique_lock<mutex> lk(q_mutex_);
    if(pq_.empty())
    {
      return optional<ValueType>();
    }
    else
    {
      optional<ValueType> fst( pq_.top() );
      pq_.pop();
      return fst;
    }
  }

  template<typename ValueType>
  void sync_priority_queue<ValueType>::push(const ValueType & elem)
  {
    lock_guard<mutex> lk(q_mutex_);
    pq_.push(elem);
    is_empty_.notify_one();
  }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
  template<typename ValueType>
  void sync_priority_queue<ValueType>::push(ValueType&& elem)
  {
    lock_guard<mutex> lk(q_mutex_);
    pq_.emplace(elem);
    is_empty_.notify_one();
  }

#endif

  template<typename ValueType>
  optional<ValueType> sync_priority_queue<ValueType>::try_pull()
  {
    unique_lock<mutex> lk(q_mutex_, try_to_lock);
    if(lk.owns_lock())
    {
      while(pq_.empty())
      {
        if(closed.load()) throw std::exception();
        is_empty_.wait(lk);
      }
      optional<ValueType> fst( pq_.top() );
      pq_.pop();
      return fst;
    }
    return optional<ValueType>();
  }

  template<typename ValueType>
  optional<ValueType> sync_priority_queue<ValueType>::try_pull_no_wait()
  {
    unique_lock<mutex> lk(q_mutex_, try_to_lock);
    if(lk.owns_lock() && !pq_.empty())
    {
      optional<ValueType> fst( pq_.top() );
      pq_.pop();
      return fst;
    }
    return optional<ValueType>();
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

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
  template<typename ValueType>
  bool sync_priority_queue<ValueType>::try_push(ValueType&& elem)
  {
    if(q_mutex_.try_lock())
    {
      pq_.emplace(elem);
      q_mutex_.unlock();
      is_empty_.notify_one();
      return true;
    }
    return false;
  }
#endif

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
