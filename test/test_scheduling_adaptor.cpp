#include <boost/function.hpp>
#include <boost/thread/executors/executor.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/thread/executors/scheduling_adaptor.hpp>

#include <boost/core/lightweight_test.hpp>

using namespace boost::chrono;

typedef boost::executors::basic_thread_pool thread_pool; 

void fn(int x)
{
    std::cout << x << std::endl;
}

void test_timing(const int n)
{
    thread_pool tp(4);
    boost::scheduling_adpator<thread_pool> sa(tp);
    for(int i = 1; i <= n; i++)
    {
        sa.submit_after(boost::bind(fn,i),seconds(i));
        sa.submit_after(boost::bind(fn,i), milliseconds(i*100));
    }

}

int main()
{
  steady_clock::time_point start = steady_clock::now();
  test_timing(5);
  steady_clock::duration diff = steady_clock::now() - start;  
  BOOST_TEST(diff > seconds(5));
  return boost::report_errors();
}
