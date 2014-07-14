#include <iostream>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/sync_timed_queue.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace boost::chrono;

void test_all()
{
  boost::sync_timed_queue<int> pq;
  BOOST_TEST(pq.empty());
  BOOST_TEST(!pq.is_closed());
  BOOST_TEST_EQ(pq.size(), 0);

  for(int i = 1; i <= 5; i++){
    pq.push(i,seconds(i));
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 6; i <= 10; i++){
    pq.push(i,steady_clock::now() + seconds(i));
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
  boost::sync_timed_queue<int> pq;
  BOOST_TEST(pq.empty());
  BOOST_TEST(!pq.is_closed());
  BOOST_TEST_EQ(pq.size(), 0);

  for(int i = 1; i <= 5; i++){
    bool succ = pq.try_push(i,seconds(i));
    BOOST_TEST(succ);
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 6; i <= 10; i++){
    bool succ = pq.try_push(i,steady_clock::now() + seconds(i));
    BOOST_TEST(succ);
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(), i);
  }

  for(int i = 1; i <= 10; i++){
    boost::optional<int> val = pq.try_pull();
    BOOST_TEST_EQ(*val, i);
  }

  boost::optional<int> val = pq.pull_no_wait();
  BOOST_TEST(!val);

  BOOST_TEST(pq.empty());
  pq.close();
  BOOST_TEST(pq.is_closed());
}


int main()
{
  test_all();
  test_all_with_try();
  return boost::report_errors();
}
