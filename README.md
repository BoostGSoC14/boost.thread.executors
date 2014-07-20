#GSoC Scheduled Executors

This is the feature branch for implementing Scheduled Executors for GSoC 2014.

##Design

Please read the original GSoC proposal [Here](http://cs.mcgill.ca/~iforbe).

Since Boost.Thread chose not to follow the [N3785](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3785.pdf) proposal there have been a few changes to the original design.

Most of the effort for this project has gone into creating a thread-safe priority queue and a thread-safe 'timed' queue which is specialization of the thread-safe priority queue.

All additional code can be found in:

 * `include/boost/thread/sync_priority_queue.hpp`
 * `include/boost/thread/sync_timed_queue.hpp`
 * `include/boost/thread/executors/scheduled_executor.hpp`
 * `include/boost/thread/executors/scheduled_thread_pool.hpp`
 * `include/boost/thread/executors/scheduling_adaptor`

### `sync_priority_queue<T>`

The `sync_priority_queue` class is simply an unbounded `std::priority_queue` guarded by a `boost::mutex` with a `boost::condition_var` to allow threads to wait when the queue becomes empty.

This class provided to important methods, `void push(const T&)` and `T pull()`. 

The former allows a thread to safely push an item on the queue while the later allows a thread to safely take the first/top element for the queue. 

There are numerous other methods that allow for nonblocking access to the queue but may no necessarily succeed if the queue is locked by another thread or the queue is empty. These are not used by any classes in the implementation of scheduled_executor but may be useful to others. 

On problem with `T pull()` is that if the queue is empty and 

### `sync_timed_queue<T> : private sync_priority_queue<scheduled_type<T>>`

This is a specialization of `sync_priority_queue<T>`. It is exactly the same as `sync_priority_queue<T>` except for the fact that `T` is `scheduled_type` and the `T pull()` method is slightly modified. It uses `using` declarations to expose the methods from the super/parent class which are the same. These include `void push(const T&)`. 

`scheduled_type<T>` is a simple struct that contains a `T` and `time_point`. I also implements `bool operator<(const scheduled_type& other) const` in such a way that when placed in a `priority_queue` the `scheduled_type` with the `time_point` closest to time 0, i.e. the least `time_point` is at the front of the queue. 

The `T pull()` method is modified so that a `scheduled_type` will never be removed from the underlying queue while its `time_point` > `now()`. If a thread tries to pull the first element of the from the queue and the first item's `time_point` is greater than `now()` then the thread will wait on a condition for `top().time_point - now()` units of time. If another element is pushed onto the queue the waiting thread will be notified to wake in case the new element is inserted at the front. 

### `scheduled_executor`

This is an **abstract class** which contains all the queuing logic for implementing an executor with scheduling logic. It contains a work queue of type `sync_timed_queue<std::function>` to hold closures and exposes all the necessary functions to push work on to the queue and have it dequeued at a certain point in the future. 

The constructor is `protected` so that that it cannot be created as a concert class. In order to use it users must inherit from it and implement there own dequeuing logic. This generally means the number of worker threads and there logic. 

**Note:** This class is not meant to be used polymorphically, i.e. through a pointer or referece. There are no virtual functions! This is simply to abstract queuing logic into a common base. That being said since subclasses should not override any of the parent methods (except the ctor) it may be safe to use it polymorphically for methods like `submit`.

### `scheduled_thread_pool : public scheduled_executor`

This is a concrete class the extends `scheduled_executor`. It implements a thread_pool. It contains a `std::vector<boost::thread>` as has worker threads which simply dequeue and execute closures on the work queue. 

### `scheduling_adaptor<Executor> : public scheduled_executor`

This class wraps another executor of type `Executor` and adds scheduling logic on top of it. This class contains a reference to the wrapped executor and has one worker thread which simply forwards elements to the wrapped executor.

## Observations and Justifications.

I've found that in general an executor can be described by the formula. 

`queue_type + N * worker_threads = executor_type`

For example:

 * `std::queue + N * (worker logic that executes the closure) = thread_pool`
 * `std::priority_queue + N * (worker logic that executes the closure) = scheduled_thread_pool`
 * `std::queue + 1 * (worker that forwards to another executor and wait for completion) = serial_executor`
 * `std::priority_queue + 1 * (worker that simply forwards work) = scheduling_adaptor`
 
Given this pattern I found that it was best to abstract the queuing logic into an abstract class for scheduled_executor so that all concrete classes implementing it simply need to provide their own worker logic and the number of workers they would like since the queuing logic is always the same between scheduled_executors.

## Caveats 

This release currently requires `C++11` in order to use `std::function<void()>` as the closure type. This is only until all of the bugs in `executors::work` have been ironed out. 

## Tests

Tests can be found in the following places:

 * `test/test_scheduled_tp.cpp`
 * `test/test_scheduling_adaptor.cpp`
 * `test/sync/mutal_exclusion/sync_pq/*`
 
Please link against at least boost 1.55.0

`g++ -std=c++11 test_scheduled_tp.cpp -l:libboost_thread.so.1.55.0 -l:libboost_system.so.1.55.0 -l:libboost_chrono.so.1.55.0`

You may need to define `-DBOOST_THREAD_VERSION=4` for some tests.

## TODO 

* Write more tests
* Move to `executors::work` as closure type.
>>>>>>> Inital commit for README.
