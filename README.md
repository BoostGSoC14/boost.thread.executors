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

#### `sync_priority_queue<T>`

The `sync_priority_queue` class is simply an unbounded `std::priority_queue` guarded by a `boost::mutex` with a `boost::condition_var` to allow threads to wait when the queue becomes empty.

This class provided to important methods, `void push(const T&)` and `T pull()`. 

The former allows a thread to safely push an item on the queue while the later allows a thread to safely take the first/top element for the queue. 

There are numerous other methods that allow for nonblocking access to the queue but may no necessarily succeed if the queue is locked by another thread or the queue is empty. These are not used by any classes in the implementation of scheduled_executor but may be useful to others. 

On problem with `T pull()` is that if the queue is empty and 

#### `sync_timed_queue<T> : private sync_priority_queue<scheduled_type<T>>`

This is a specialization of `sync_priority_queue<T>`. It is exactly the same as `sync_priority_queue<T>` except for the fact that `T` is `scheduled_type` and the `T pull()` method is slightly modified. It uses `using` declarations to expose the methods from the super/parent class which are the same. These include `void push(const T&)`. 

`scheduled_type<T>` is a simple struct that contains a `T` and `time_point`. I also implements `bool operator<(const scheduled_type& other) const` in such a way that when placed in a `priority_queue` the `scheduled_type` with the `time_point` closest to time 0, i.e. the least `time_point` is at the front of the queue. 

The `T pull()` method is modified so that a `scheduled_type` will never be removed from the underlying queue while its `time_point` > `now()`. If a thread tries to pull the first element of the from the queue and the first item's `time_point` is greater than `now()` then the thread will wait on a condition for `top().time_point - now()` units of time. If another element is pushed onto the queue the waiting thread will be notified to wake in case the new element is inserted at the front. 

#### `scheduled_executor`

This is an **abstract class** which contains all the queuing logic for implementing an executor with scheduling logic. It contains a work queue of type `sync_timed_queue<std::function>` to hold closures and exposes all the necessary functions to push work on to the queue and have it dequeued at a certain point in the future. 

The constructor is `protected` so that that it cannot be created as a concert class. In order to use it users must inherit from it and implement there own dequeuing logic. This generally means the number of worker threads and there logic. 

**Note:** This class is not meant to be used polymorphically, i.e. through a pointer or referece. There are no virtual functions! This is simply to abstract queuing logic into a common base. That being said since subclasses should not override any of the parent methods (except the ctor) it may be safe to use it polymorphically for methods like `submit`.

#### `scheduled_thread_pool : public scheduled_executor`

This is a concrete class the extends `scheduled_executor`. It implements a thread_pool. It contains a `std::vector<boost::thread>` as has worker threads which simply dequeue and execute closures on the work queue. 

#### `scheduling_adaptor<Executor> : public scheduled_executor`

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

This release still requires `C++11` ~~in order to use `std::function<void()>` as the closure type. This is only until all of the bugs in `executors::work` have been ironed out.~~ for moved semantics when creating `vector<thread>`.

## Tests & Examples

Tests can be found in the following places:

 * `test/test_scheduled_tp.cpp` [basic]
 * `test/test_scheduling_adaptor.cpp` [basic]
 * `test/sync/mutal_exclusion/sync_pq/*` 
 
Please link against at least boost 1.55.0

`g++ -std=c++11 test_scheduled_tp.cpp -l:libboost_thread.so.1.55.0 -l:libboost_system.so.1.55.0 -l:libboost_chrono.so.1.55.0 -pthread`

You may need to define `-DBOOST_THREAD_VERSION=4` for some tests if it complains about `future` not being found.

## Possible Changes & RFCs

 * `sync_timed_queue` and `scheduled_executor` have template parameters for a `Clock`. This defaults to `boost::chrono::steady_clock`. There is a static assert to ensure that `Clock` is steady. This is also satisfied by `high_resolution_clock` however the extra effort and complexity doesn't seems having this additional option as most people probably won't use it. The only counter argument would be that `cpu_time` clocks can also be used as `Clock`. This would give better guarantees on when closures will be executed as preemption of threads by the OS is no longer a factor like with `steady_clock` and `high_resolution_clock` which are wall clocks. 
 
 * There is an `atomic<bool> _closed` in the sync_queues which allows for immediate closing of the queue without waiting for the queue mutex. The problems with this is include:
 
    1. The memory_order is set to default so performance could be very poor. Especially on non-Intel architectures which will tend insert memory barrier instructions. The memory order could possibly be relaxed to improve performance. I require that after `_closed.store(true)` is executed all reads subsequent to `_closed` will read as `true`. I suppose this would mean `_closed.store(true)` shouldn't be re-ordered and no other instructions should be re-ordered from before this instruction to after. I'm not sure as to which memory_order this maps to.
    2. If the `atomic<bool>` isn't lock-free then we are essentially back to the original problem and might as well wait for the queue_mutex instead.
    
 * Currently enqueueing on a closed executor is allowed. It may be a good idea to disallow this, perhaps via an execption. 

 * Move `sync_timed_queue` into `detail`. This is a fairly specialized queue that few people will probably have little use for. By moving into `detail` I can include the bare minimum of functions which I require in `scheduled_executor`. It will also save me from testing functions which I, and probably no one else, will ever use.
 
 * Add a `priority_executor` class. This would be trivial to implement given my knowledge of the problem and the fact that `sync_priority_queue` is already complete.
 * Add an overload to `boost::aysnc` so that it takes an executor an argument.

## TODO 
* Write more tests rigorous tests.
* Integrate into Boost.Build
* Move to `executors::work` as closure type when its ready.
