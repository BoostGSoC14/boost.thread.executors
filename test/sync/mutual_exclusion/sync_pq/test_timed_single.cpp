#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/function.hpp>
#include <boost/thread/detail/sync_timed_queue.hpp>

#include <boost/core/lightweight_test.hpp>

using namespace boost::chrono;

typedef boost::detail::sync_timed_queue<int> sync_tq;

void test_all()
{
  sync_tq pq;
  BOOST_TEST(pq.empty());
  BOOST_TEST(!pq.is_closed());
  BOOST_TEST_EQ(pq.size(), 0);

  for(int i = 1; i <= 5; i++){
    pq.push(i, milliseconds(i*100));
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 6; i <= 10; i++){
    pq.push(i,steady_clock::now() + milliseconds(i*100));
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 1; i <= 10; i++){
    int val = pq.pull();
    BOOST_TEST_EQ(val, i);
  }

  boost::optional<int> val = pq.pull_no_wait();
  BOOST_TEST(!val);

  BOOST_TEST(pq.empty());
  pq.close();
  BOOST_TEST(pq.is_closed());
}

void test_all_with_try()
{
  sync_tq pq;
  BOOST_TEST(pq.empty());
  BOOST_TEST(!pq.is_closed());
  BOOST_TEST_EQ(pq.size(), 0);

  for(int i = 1; i <= 5; i++){
    bool succ = pq.try_push(i, milliseconds(i*100));
    BOOST_TEST(succ);
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 6; i <= 10; i++){
    bool succ = pq.try_push(i,steady_clock::now() + milliseconds(i*100));
    BOOST_TEST(succ);
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 1; i <= 10; i++){
    boost::optional<int> val = pq.try_pull();
    BOOST_TEST(val);
    BOOST_TEST_EQ(*val, i);
  }

  boost::optional<int> val = pq.pull_no_wait();
  BOOST_TEST(!val);

  BOOST_TEST(pq.empty());
  pq.close();
  BOOST_TEST(pq.is_closed());
}

void func(steady_clock::time_point pushed, steady_clock::duration dur)
{
    BOOST_TEST(pushed + dur <= steady_clock::now());
}

/**
 * This test ensures that when items come of the front of the queue
 * that at least $dur has elapsed.
 */
void test_deque_times()
{
    boost::detail::sync_timed_queue<boost::function<void()> > tq;
    for(int i = 0; i < 10; i++)
    {
        steady_clock::duration d = milliseconds(i*100);
        boost::function<void()> fn = boost::bind(func, steady_clock::now(), d);
        tq.push(fn, d);
    }
    while(!tq.empty())
    {
        boost::function<void()> fn = tq.pull();
        fn();
    }
}

int main()
{
  test_all();
  test_all_with_try();
  test_deque_times();
  return boost::report_errors();
}
