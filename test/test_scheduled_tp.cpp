#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/function.hpp>
#include <boost/thread/executors/scheduled_thread_pool.hpp>

#include <boost/core/lightweight_test.hpp>

using namespace boost::chrono;

typedef boost::scheduled_thread_pool scheduled_tp;

void fn(int x)
{
//  std::cout << x << std::endl;
}

void func(steady_clock::time_point pushed, steady_clock::duration dur)
{
    BOOST_TEST(pushed + dur < steady_clock::now());
}

void test_timing(const int n)
{
  //This function should take n seconds to execute.
  boost::scheduled_thread_pool se(4);

  for(int i = 1; i <= n; i++)
  {
    se.submit_after(boost::bind(fn,i), milliseconds(i*100));
  }
  //dtor is called here so all task will have to be executed before we return
}

void test_deque_timing()
{
    boost::scheduled_thread_pool se(4);
    for(int i = 0; i < 10; i++)
    {
        steady_clock::duration d = milliseconds(i*100);
        boost::function<void()> fn = boost::bind(func,steady_clock::now(),d);
        se.submit_after(fn,d);
    }
}

void test_deque_multi(const int n)
{
    scheduled_tp se(4);
    boost::thread_group tg;
    for(int i = 0; i < n; i++)
    {
        steady_clock::duration d = milliseconds(i*100);
        boost::function<void()> fn = boost::bind(func,steady_clock::now(),d);
        tg.create_thread(boost::bind(boost::mem_fn(&scheduled_tp::submit_after), &se, fn, d));
    }
    tg.join_all();
    //dtor is called here so execution will block untill all the closures
    //have been completed.
}

int main()
{
  steady_clock::time_point start = steady_clock::now();
  test_timing(5);
  steady_clock::duration diff = steady_clock::now() - start;
  BOOST_TEST(diff > milliseconds(500));
  test_deque_timing();
  test_deque_multi(4);
  test_deque_multi(8);
  test_deque_multi(16);
  return boost::report_errors();
}
