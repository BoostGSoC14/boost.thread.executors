#ifndef BOOST_THREAD_SYNC_TIMED_QUEUE_HPP
#define BOOST_THREAD_SYNC_TIMED_QUEUE_HPP

#include <boost/chrono/time_point.hpp>
#include <boost/thread/sync_priority_queue.hpp>
#include <boost/thread/executors/work.hpp>

#include <boost/config/abi_prefix.hpp>

namespace boost
{

  template<typename T>
  struct scheduled_type
  {
    T data;
    chrono::steady_clock::time_point tp_;

    scheduled_type(T pdata, T ptp) : data(pdata), tp_(ptp) {}

    bool operator <(const scheduled_type<T> other) const
    {
      return this->data < other.data;
    }
  }; //end struct

  template<typename T>
  class sync_timed_queue : private sync_priority_queue<scheduled_type<T> >
  {
  public:
    typedef sync_priority_queue<scheduled_type<T> > parent;

    sync_timed_queue() : parent() {};
	~sync_timed_queue() {} //Call parent?
    BOOST_THREAD_NO_COPYABLE(sync_timed_queue)

    using parent::size;
    using parent::empty;

    T pull();
	bool try_pull(T& elem);

    void push(const T& elem, chrono::steady_clock::time_point tp);
    void push(const T& elem, chrono::steady_clock::duration dura);
    bool try_push(const T& elem, chrono::steady_clock::time_point tp);
    bool try_push(const T& elem, chrono::steady_clock::duration dura);

  }; //end class

  template<typename T>
  void sync_timed_queue<T>::push(const T& elem, chrono::steady_clock::time_point tp)
  {
    parent::push(scheduled_type<T>(elem,tp));
  }

  template<typename T>
  void sync_timed_queue<T>::push(const T& elem, chrono::steady_clock::duration dura)
  {
    const chrono::steady_clock::time_point tp = chrono::steady_clock::now() + dura;
    parent::push(scheduled_type<T>(elem,tp));
  }

} //end namespace

#include <boost/config/abi_suffix.hpp>

#endif
