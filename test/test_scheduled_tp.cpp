#include <iostream>
#include <functional>

#include <boost/chrono.hpp>
#include <boost/thread/executors/scheduled_thread_pool.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace boost::chrono;

void fn(int x)
{
  std::cout << x << std::endl;
}

void test_timing(const int n)
{
  //This function should take n seconds to execute.
  boost::scheduled_thread_pool<> se(4);

  for(int i = 1; i <= n; i++)
    se.submit_after(std::bind(fn,i), seconds(i));

}

int main()
{
  steady_clock::time_point start = steady_clock::now();
  test_timing(5);
  steady_clock::duration diff = steady_clock::now() - start;  
  BOOST_TEST(diff > seconds(5));
  return boost::report_errors();
}
