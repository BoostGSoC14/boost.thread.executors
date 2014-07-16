#include <boost/chrono.hpp>
#include <boost/thread/sync_priority_queue.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace boost::chrono;

#include <future>
    
typedef boost::sync_priority_queue<int> sync_pq;

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

    std::future<int> futs[n];
    for(int i = 0; i < n; i++)
    {
        futs[i] = std::async(std::launch::async,&sync_pq::pull,&pq);
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
    for(int i  = 0; i < n; i++)
    {
        // std::async(std::launch::async,[&](){pq.push(i);});
        //Won't Compile with rvalue refs because overload resolution fails
        std::async(std::launch::async,&sync_pq::push,&pq,i);
    }
    BOOST_TEST(!pq.empty());
    BOOST_TEST_EQ(pq.size(),n);
}

int main()
{
    test_pull();
    return boost::report_errors();
}
