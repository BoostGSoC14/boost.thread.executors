#ifndef BOOST_THREAD_SYNC_TIMED_QUEUE_HPP
#define BOOST_THREAD_SYNC_TIMED_QUEUE_HPP

#include <boost/static_assert.hpp>
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
      return this->time > other.time;
    }
  }; //end struct

  template<typename T, typename Clock = chrono::steady_clock>
  class sync_timed_queue : private sync_priority_queue<scheduled_type<T, typename Clock::time_point> >
  {
  public:
    BOOST_STATIC_ASSERT(Clock::is_steady);
    typedef typename Clock::duration duration;
    typedef typename Clock::time_point time_point;

    typedef scheduled_type<T,typename Clock::time_point> stype;
    typedef sync_priority_queue<scheduled_type<T,time_point> > super;

    sync_timed_queue() : super() {};
    ~sync_timed_queue() {} //Call super?
    BOOST_THREAD_NO_COPYABLE(sync_timed_queue)

    using super::size;
    using super::empty;
    using super::close;
    using super::is_closed;

    T pull();
    optional<T> try_pull();

    void push(const T& elem, const time_point& tp);
    void push(const T& elem, const duration& dura);
    bool try_push(const T& elem, const time_point& tp);
    bool try_push(const T& elem, const duration& dura);

  }; //end class

  template<typename T, typename Clock>
  void sync_timed_queue<T,Clock>::push(const T& elem, const time_point& tp)
  {
    super::push(stype(elem,tp));
  }

  template<typename T, typename Clock>
  void sync_timed_queue<T,Clock>::push(const T& elem, const duration& dura)
  {
    const time_point tp = Clock::now() + dura;
    super::push(stype(elem,tp));
  }

  template<typename T, typename Clock>
  bool sync_timed_queue<T,Clock>::try_push(const T& elem, const time_point& tp)
  {
    return super::try_push(stype(elem,tp));
  }

  template<typename T, typename Clock>
  bool sync_timed_queue<T,Clock>::try_push(const T& elem, const duration& dura)
  {
    const time_point tp = Clock::now() + dura;
    return super::try_push(stype(elem,tp));
  }

  template<typename T, typename Clock>
  T sync_timed_queue<T,Clock>::pull()
  {
    unique_lock<mutex> lk(this->q_mutex_);
    while(1)
    {
      if(this->pq_.empty())
      {
        if(this->closed.load()) throw std::exception();
        this->is_empty_.wait(lk);
      }
      else if(this->pq_.top().time > Clock::now())
      {
        this->is_empty_.wait_until(lk,this->pq_.top().time);
      }
      else
      {
        break;
      }
    }
    const T temp = this->pq_.top().data;
    this->pq_.pop();
    return temp;
  }

  template<typename T, typename Clock>
  optional<T> sync_timed_queue<T,Clock>::try_pull()
  {
    unique_lock<mutex> lk(this->q_mutex_);
    if(lk.owns_lock())
    {
      while(1)
      {
        if(this->pq_.empty())
        {
          if(this->closed.load()) throw std::exception();
          this->is_empty_.wait(lk);
        }
        else if(this->pq_.top().time > Clock::now())
        {
          this->is_empty._wait_until(lk,this->pq_.top().time);
        }
        else
        {
          break;
        }
      }
      const optional<T> ret = optional<T>( this->pq_.top().data );
      this->pq_.pop();
      return ret;
    }
    return optional<T>();
  }

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
