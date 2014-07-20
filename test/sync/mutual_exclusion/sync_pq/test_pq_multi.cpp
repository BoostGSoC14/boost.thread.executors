#define BOOST_THREAD_VERSION 4

#include <boost/thread/future.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/sync_priority_queue.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace boost::chrono;
 
typedef boost::sync_priority_queue<int> sync_pq;

struct call_push
{
  sync_pq& _pq;
  boost::barrier& _go;
  int _num;

  call_push(sync_pq &q, boost::barrier& go, int n) : _pq(q), _go(go), _num(n) {}

  typedef void result_type;
  void operator()()
  {
    _go.count_down_and_wait();
    _pq.push(_num);
  }
};

struct call_pull
{
  sync_pq& _pq;
  boost::barrier& _go;

  call_pull(sync_pq& q, boost::barrier& go) : _pq(q), _go(go) {}

  typedef int result_type;
  int operator()()
  {
    _go.count_down_and_wait();
    return _pq.pull();
  }
};

void test_pull()
{
    const int n = 4;
    sync_pq pq;
    BOOST_TEST(pq.empty());
    for(int i  = 0; i < n; i++)
    {
        pq.push(i);
    }
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(),n);

    boost::barrier b(n);
    boost::future<int> futs[n];
    for(int i = 0; i < n; i++)
    {
        futs[i] = boost::async(boost::launch::async, call_pull(pq,b));
    }
    for(int i = 0; i < n; i++)
    {
        futs[i].get();
    }
    BOOST_TEST(pq.empty());
}

void test_push()
{
    const int n = 4;
    sync_pq pq;
    BOOST_TEST(pq.empty());

    boost::barrier b(n);
    boost::future<void> futs[n];
    for(int i  = 0; i < n; i++)
    {
        futs[i] = boost::async(boost::launch::async, call_push(pq,b,i));
    }
    for(int i = 0; i < n; i++)
    {
        futs[i].wait();
    }
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(),n);
}

void test_both()
{
    const int n = 4;
    sync_pq pq;
    BOOST_TEST(pq.empty());

    boost::barrier b(2*n);
    boost::future<int> futs1[n];
    boost::future<void> futs2[n];
    for(int i  = 0; i < n; i++)
    {
        futs1[i] = boost::async(boost::launch::async, call_pull(pq,b));
        futs2[i] = boost::async(boost::launch::async, call_push(pq,b,i));
    }
    for(int i = 0; i < n; i++)
    {
        futs1[i].wait();
        futs2[i].wait();
    }
    BOOST_TEST(pq.empty());
    BOOST_TEST_EQ(pq.size(), 0);
}

int main()
{
    test_pull();
    test_push();
    test_both();
    return boost::report_errors();
}
