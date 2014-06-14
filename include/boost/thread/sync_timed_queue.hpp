#ifndef BOOST_THREAD_SYNC_TIMED_QUEUE_HPP
#define BOOST_THREAD_SYNC_TIMED_QUEUE_HPP

#include <boost/chrono/time_point.hpp>
#include <boost/thread/sync_priority_queue.hpp>
#include <boost/thread/executors/work.hpp>

#include <boost/config/abi_prefix.hpp>

namespace boost
{

  template<typename T, typename TimePoint = chrono::steady_clock::time_point>
  struct scheduled_type
  {
    T data;
    TimePoint time;

    scheduled_type(T pdata, TimePoint tp) : data(pdata), time(tp) {}

    bool operator <(const scheduled_type<T,TimePoint> other) const
    {
      return this->data > other.data;
    }
  }; //end struct

  template<typename T>
  class sync_timed_queue : private sync_priority_queue<scheduled_type<T> >
  {
  public:
    typedef chrono::steady_clock clock;
    typedef sync_priority_queue<scheduled_type<T> > parent;

    sync_timed_queue() : parent() {};
	~sync_timed_queue() {} //Call parent?
    BOOST_THREAD_NO_COPYABLE(sync_timed_queue)

    using parent::size;
    using parent::empty;

    T pull();
	bool try_pull(T& elem);

    void push(const T& elem, clock::time_point tp);
    void push(const T& elem, clock::duration dura);
    bool try_push(const T& elem, clock::time_point tp);
    bool try_push(const T& elem, clock::duration dura);

  }; //end class

  template<typename T>
  void sync_timed_queue<T>::push(const T& elem, clock::time_point tp)
  {
    parent::push(scheduled_type<T>(elem,tp));
  }

  template<typename T>
  void sync_timed_queue<T>::push(const T& elem, clock::duration dura)
  {
    const chrono::steady_clock::time_point tp = clock::now() + dura;
    parent::push(scheduled_type<T>(elem,tp));
  }

  template<typename T>
  bool sync_timed_queue<T>::try_push(const T& elem, clock::time_point tp)
  {
    return parent::try_push(scheduled_type<T>(elem,tp));
  }

  template<typename T>
  bool sync_timed_queue<T>::try_push(const T& elem, clock::duration dura)
  {
    const clock::time_point tp = clock::now() + dura;
    return parent::try_push(scheduled_type<T>(elem,tp));
  }

  template<typename T>
  T sync_timed_queue<T>::pull()
  {
    unique_lock<mutex> lk(this->q_mutex_);
    while(this->pq_.empty() || (!this->pq_.empty() && this->pq_.top().time > clock::now()) )
    {
        if(this->pq_.empty())
        {
            this->is_empty_.wait(lk);
        }
        else
        {
            this->is_empty_.wait_until(lk,this->pq_.top().time);
        }
    }
    const T temp = this->pq_.top().data;
    this->pq_.pop();
    return temp;
  }


  template<typename T>
  bool sync_timed_queue<T>::try_pull(T& dest)
  {
    unique_lock<mutex> lk(this->q_mutex_);
    if(lk.owns_lock())
    {
        while(this->pq_.empty() || (!this->pq_.empty() && this->pq_.top().time > clock::now()) )
        {
            if(this->pq_.empty())
            {
                this->is_empty_.wait(lk);
            }
            else
            {
                this->is_empty._wait_until(lk,this->pq_.top().time);
            }
        }
        dest = this->pq_.top().data;
        this->pq_.pop();
        return true;
    }
    return false;
  }

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
