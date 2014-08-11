#ifndef BOOST_THREAD_SYNC_TIMED_QUEUE_HPP
#define BOOST_THREAD_SYNC_TIMED_QUEUE_HPP

#include <boost/chrono/time_point.hpp>
#include <boost/thread/sync_priority_queue.hpp>

#include <boost/config/abi_prefix.hpp>

namespace boost
{

  template<typename T>
  struct scheduled_type
  {
    typedef chrono::steady_clock::time_point time_point;
    T data;
    time_point time;

    scheduled_type(T pdata, time_point tp) : data(pdata), time(tp) {}

    bool operator <(const scheduled_type<T> other) const
    {
      return this->time > other.time;
    }
  }; //end struct

  template<typename T>
  class sync_timed_queue : private sync_priority_queue<scheduled_type<T> >
  {
  public:
    typedef typename chrono::steady_clock clock; 
    typedef typename clock::duration duration;
    typedef typename clock::time_point time_point;

    typedef scheduled_type<T> stype;
    typedef sync_priority_queue<scheduled_type<T> > super;

    sync_timed_queue() : super() {};
    ~sync_timed_queue() {} //Call super?

    using super::size;
    using super::empty;
    using super::close;
    using super::is_closed;

    T pull();
    optional<T> try_pull();
    optional<T> pull_no_wait();

    void push(const T& elem, const time_point& tp);
    void push(const T& elem, const duration& dura);
    bool try_push(const T& elem, const time_point& tp);
    bool try_push(const T& elem, const duration& dura);
  private:
    sync_timed_queue(const sync_timed_queue&);
    sync_timed_queue& operator=(const sync_timed_queue&);
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
    sync_timed_queue(sync_timed_queue&&);
    sync_timed_queue& operator=(sync_timed_queue&&);
#endif
  }; //end class

  template<typename T>
  void sync_timed_queue<T>::push(const T& elem, const time_point& tp)
  {
    super::push(stype(elem,tp));
  }

  template<typename T>
  void sync_timed_queue<T>::push(const T& elem, const duration& dura)
  {
    const time_point tp = clock::now() + dura;
    super::push(stype(elem,tp));
  }

  template<typename T>
  bool sync_timed_queue<T>::try_push(const T& elem, const time_point& tp)
  {
    return super::try_push(stype(elem,tp));
  }

  template<typename T>
  bool sync_timed_queue<T>::try_push(const T& elem, const duration& dura)
  {
    const time_point tp = clock::now() + dura;
    return super::try_push(stype(elem,tp));
  }

  template<typename T>
  T sync_timed_queue<T>::pull()
  {
    unique_lock<mutex> lk(super::_qmutex);
    while(1)
    {
      if(super::_pq.empty())
      {
        if(super::_closed.load()) throw std::exception();
        super::_qempty.wait(lk);
      }
      else if(super::_pq.top().time > clock::now())
      {
        super::_qempty.wait_until(lk,super::_pq.top().time);
      }
      else
      {
        break;
      }
    }
    const T temp = super::_pq.top().data;
    super::_pq.pop();
    return temp;
  }

  template<typename T>
  optional<T> sync_timed_queue<T>::try_pull()
  {
    unique_lock<mutex> lk(super::_qmutex);
    if(lk.owns_lock())
    {
      while(1)
      {
        if(super::_pq.empty())
        {
          if(super::_closed.load()) throw std::exception();
          super::_qempty.wait(lk);
        }
        else if(super::_pq.top().time > clock::now())
        {
          super::_qempty.wait_until(lk,super::_pq.top().time);
        }
        else
        {
          break;
        }
      }
      const optional<T> ret = optional<T>( super::_pq.top().data );
      super::_pq.pop();
      return ret;
    }
    return optional<T>();
  }

  template<typename T>
  optional<T> sync_timed_queue<T>::pull_no_wait()
  {
    lock_guard<mutex> lk(super::_qmutex);
    if(super::_pq.empty())
    {
      return optional<T>();
    }
    else if(super::_pq.top().time > clock::now())
    {
      return optional<T>();
    }
    else
    {
      const optional<T> ret = optional<T>( super::_pq.top().data );
      super::_pq.pop();
      return ret;
    }
  }

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
