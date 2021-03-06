// Copyright (C) 2013 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 2013/09 Vicente J. Botet Escriba
//    Adapt to boost from CCIA C++11 implementation
//    Make use of Boost.Move

#ifndef BOOST_THREAD_DETAIL_NULLARY_FUNCTION_HPP
#define BOOST_THREAD_DETAIL_NULLARY_FUNCTION_HPP

#include <boost/config.hpp>
#include <boost/thread/detail/memory.hpp>
#include <boost/thread/detail/move.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/type_traits/decay.hpp>

namespace boost
{
  namespace detail
  {

    template <typename F>
    class nullary_function;
    template <>
    class nullary_function<void()>
    {
      struct impl_base
      {
        virtual void call()=0;
        virtual ~impl_base()
        {
        }
      };
      shared_ptr<impl_base> impl;
      template <typename F>
      struct impl_type: impl_base
      {
        F f;
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
        impl_type(F &f_)
          : f(f_)
        {}
#endif
        impl_type(BOOST_THREAD_RV_REF(F) f_)
          : f(boost::move(f_))
        {}

        void call()
        {
          f();
        }
      };
      struct impl_type_ptr: impl_base
      {
        void (*f)();
        impl_type_ptr(void (*f_)())
          : f(f_)
        {}
        void call()
        {
          f();
        }
      };
    public:
      BOOST_THREAD_MOVABLE(nullary_function)

      explicit nullary_function(void (*f)()):
      impl(new impl_type_ptr(f))
      {}

#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
      template<typename F>
      explicit nullary_function(F& f):
      impl(new impl_type<F>(f))
      {}
#endif
      template<typename F>
      nullary_function(BOOST_THREAD_RV_REF(F) f):
      impl(new impl_type<typename decay<F>::type>(thread_detail::decay_copy(boost::forward<F>(f))))
      {}

      nullary_function()
        : impl()
      {
      }
      nullary_function(nullary_function const& other) BOOST_NOEXCEPT :
      impl(other.impl)
      {
      }
      nullary_function(BOOST_THREAD_RV_REF(nullary_function) other) BOOST_NOEXCEPT :
      impl(BOOST_THREAD_RV(other).impl)
      {
        BOOST_THREAD_RV(other).impl.reset();
      }
      ~nullary_function()
      {
      }

      nullary_function& operator=(nullary_function const& other) BOOST_NOEXCEPT
      {
        impl=other.impl;
        return *this;
      }
      nullary_function& operator=(BOOST_THREAD_RV_REF(nullary_function) other) BOOST_NOEXCEPT
      {
        impl=BOOST_THREAD_RV(other).impl;
        BOOST_THREAD_RV(other).impl.reset();
        return *this;
      }


      void operator()()
      { impl->call();}

    };

    template <typename R>
    class nullary_function<R()>
    {
      struct impl_base
      {
        virtual R call()=0;
        virtual ~impl_base()
        {
        }
      };
      shared_ptr<impl_base> impl;
      template <typename F>
      struct impl_type: impl_base
      {
        F f;
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
        impl_type(F &f_)
          : f(f_)
        {}
#endif
        impl_type(BOOST_THREAD_RV_REF(F) f_)
          : f(boost::move(f_))
        {}

        R call()
        {
          return f();
        }
      };
      struct impl_type_ptr: impl_base
      {
        R (*f)();
        impl_type_ptr(R (*f_)())
          : f(f_)
        {}

        R call()
        {
          return f();
        }
      };
    public:
      BOOST_THREAD_MOVABLE(nullary_function)

      nullary_function(R (*f)()):
      impl(new impl_type_ptr(f))
      {}
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
      template<typename F>
      nullary_function(F& f):
      impl(new impl_type<F>(f))
      {}
#endif
      template<typename F>
      nullary_function(BOOST_THREAD_RV_REF(F) f):
      impl(new impl_type<typename decay<F>::type>(thread_detail::decay_copy(boost::forward<F>(f))))
      {}

      nullary_function(nullary_function const& other) BOOST_NOEXCEPT :
      impl(other.impl)
      {
      }
      nullary_function(BOOST_THREAD_RV_REF(nullary_function) other) BOOST_NOEXCEPT :
      impl(BOOST_THREAD_RV(other).impl)
      {
        BOOST_THREAD_RV(other).impl.reset();
      }
      nullary_function()
        : impl()
      {
      }
      ~nullary_function()
      {
      }

      nullary_function& operator=(nullary_function const& other) BOOST_NOEXCEPT
      {
        impl=other.impl;
        return *this;
      }
      nullary_function& operator=(BOOST_THREAD_RV_REF(nullary_function) other) BOOST_NOEXCEPT
      {
        impl=BOOST_THREAD_RV(other).impl;
        BOOST_THREAD_RV(other).impl.reset();
        return *this;
      }

      R operator()()
      { return impl->call();}

    };
  }
}

#endif // header
