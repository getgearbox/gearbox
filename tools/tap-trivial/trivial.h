#ifndef TAP_TRIVIAL_H
#define TAP_TRIVIAL_H

#define WANT_TEST_EXTRAS
#include <tap++.h>
#include <sstream>
#include <boost/regex.hpp>

using namespace TAP;

namespace TAP {
    namespace Trivial {
        int test_fail(const char * filename, int line) {
            std::ostringstream oss;
            oss << "test failed at " << filename << " line " << line << '.';
            diag( oss.str() );
            return 0;
        }
    }
}

#define TESTFAIL TAP::Trivial::test_fail(__FILE__,__LINE__)

#define NOK(expected)     not_ok((expected), (#expected " is false")) && TESTFAIL
#define OK(expected)      ok(expected, (#expected " is true")) || TESTFAIL
#define IS(got, expected) smart_is(got, expected, (#got " => " #expected)) || TESTFAIL

#define LIKE(got, rx)     like(got, rx, (#got " =~ " #rx)) || TESTFAIL

#define THROWS(code, error) {                                           \
        try {                                                           \
            code;                                                       \
            ok(0, #code " throws error: " #error) || TESTFAIL;          \
        }                                                               \
        catch ( const std::exception & e ) {                            \
            smart_is(                                                   \
                std::string(e.what()),                                  \
                std::string(error),                                     \
                (#code " throws error: " #error)                        \
            ) || TESTFAIL;                                              \
        }                                                               \
    }

#define THROWS_LIKE(code, rx) {                                         \
        try {                                                           \
            code;                                                       \
            ok(0, #code " throws error like: " #rx)                     \
                || TESTFAIL;                                            \
        }                                                               \
        catch ( const std::exception & e ) {                            \
            like(                                                       \
                std::string(e.what()),                                  \
                std::string(rx),                                        \
                (#code " throws error like: " #rx)                      \
            ) || TESTFAIL;                                              \
        }                                                               \
    }

#define NOTHROW(code) {                                                 \
        try {                                                           \
            code;                                                       \
            ok(1, #code " does not throw an exception")                 \
                || TESTFAIL;                                            \
        }                                                               \
        catch ( const std::exception & e ) {                            \
            smart_is(                                                   \
                std::string(e.what()),                                  \
                std::string(""),                                        \
                (#code " does not throw an exception")                  \
            ) || TESTFAIL;                                              \
        }                                                               \
    }

namespace TAP {
    // template macros to determine if type is char pointer
    template <typename T> 
    struct is_char_pointer : public boost::false_type {};
    
    template <> 
    struct is_char_pointer<char *> : public boost::true_type {};
    
    // template prototype for smart_is_impl
    template<typename T, typename U, bool b>
    bool
    smart_is_impl(T left, U right, const std::string& message, const boost::integral_constant<bool, b>&) throw ();
    
    // basic impl (T is not a char *) .. just dispatch to TAP::is
    template<typename T, typename U>
    bool
    smart_is_impl(T left, U right, const std::string& message, boost::integral_constant<bool, false> const&) throw (){
        return is(left,right,message);
    }
    
    // special imple (T is a char *) .. so cast to std::string and then dispatch to TAP::is
    template<typename T, typename U>
    bool
    smart_is_impl(T *  left, U * right, const std::string& message, boost::integral_constant<bool, true> const&) throw () {
        return is(std::string(left ? left : "<NULL>"), std::string(right ? right : "<NULL>"),message);
    }
    
    // our smart_is template so we dont have to deal with char * specially in the testing code
    template<typename T, typename U> 
    bool
    smart_is(T left, U right, const std::string& message) throw () {
        return smart_is_impl(left, right, message, is_char_pointer<T>());
    }

    
	template<typename T, typename U> 
    bool
    like(const T& left, const U& right, const std::string& message = "") throw () {
		try {
            boost::regex expression(right);
            bool ret = ok( boost::regex_search(left, expression, boost::regex_constants::match_perl), message );
            if (!ret) {
                note("Failed test '", message, "'");
                note("       Got: ", left);
                note("  Expected: ", right);
            }
            return ret;
		}
		catch(const std::exception& e) {
			fail(message);
			note("Failed test '", message, "'");
			note("Cought exception '", e.what(), "'");
			note("       Got: ", left);
			note("  Expected: ", right);
			return false;
		}
		catch(...) {
			fail(message);
			note("Failed test '", message, "'");
			note("Cought unknown exception");
			note("       Got: ", left);
			note("  Expected: ", right);
			return false;
		}
	}
}

#endif // TAP_TRIVIAL_H
