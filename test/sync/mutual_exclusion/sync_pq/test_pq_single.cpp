#include <iostream>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/sync_priority_queue.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace boost::chrono;

void test_pull_for()
{
  boost::sync_priority_queue<int> pq;
  steady_clock::time_point start = steady_clock::now();
  boost::optional<int> val = pq.pull_for(milliseconds(500));
  steady_clock::duration diff = steady_clock::now() - start;
  BOOST_TEST(!val);
  BOOST_TEST(diff < milliseconds(510) && diff > milliseconds(500));
}

void test_pull_until()
{
  boost::sync_priority_queue<int> pq;
  steady_clock::time_point start = steady_clock::now();
  boost::optional<int> val = pq.pull_until(start + milliseconds(500));
  steady_clock::duration diff = steady_clock::now() - start;
  BOOST_TEST(!val);
  BOOST_TEST(diff < milliseconds(510) && diff > milliseconds(500));
}

void test_pull_no_wait()
{
  boost::sync_priority_queue<int> pq;
  steady_clock::time_point start = steady_clock::now();
  boost::optional<int> val = pq.pull_no_wait();
  steady_clock::duration diff = steady_clock::now() - start;
  BOOST_TEST(!val);
  BOOST_TEST(diff < milliseconds(5));
}

void test_pull_for_when_not_empty()
{
  boost::sync_priority_queue<int> pq;
  pq.push(1);
  steady_clock::time_point start = steady_clock::now();
  boost::optional<int> val = pq.pull_for(milliseconds(500));
  steady_clock::duration diff = steady_clock::now() - start;
  BOOST_TEST(val);
  BOOST_TEST(diff < milliseconds(5));
}

void test_pull_until_when_not_empty()
{
  boost::sync_priority_queue<int> pq;
  pq.push(1);
  steady_clock::time_point start = steady_clock::now();
  boost::optional<int> val = pq.pull_until(start + milliseconds(500));
  steady_clock::duration diff = steady_clock::now() - start;
  BOOST_TEST(val);
  BOOST_TEST(diff < milliseconds(5));
}

int main()
{
  boost::sync_priority_queue<int> pq;
  BOOST_TEST(pq.empty());
  BOOST_TEST(!pq.is_closed());
  BOOST_TEST_EQ(pq.size(), 0);

  for(int i = 1; i <= 5; i++){
    pq.push(i);
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 6; i <= 10; i++){
    bool succ = pq.try_push(i);
    BOOST_TEST(succ);
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 10; i > 5; i--){
    int val = pq.pull();
    BOOST_TEST_EQ(val, i);
  }

  for(int i = 5; i > 0; i--){
    boost::optional<int> val = pq.try_pull();
    BOOST_TEST(val);
    BOOST_TEST_EQ(*val, i);
  }

  BOOST_TEST(pq.empty());
  pq.close();
  BOOST_TEST(pq.is_closed());

  test_pull_for();
  test_pull_until();
  test_pull_no_wait();

  test_pull_for_when_not_empty();
  test_pull_until_when_not_empty();

  return boost::report_errors();
}
