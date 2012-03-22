#ifndef TEST_PLUGIN_H
#define TEST_PLUGIN_H

#include <string>

class TestPlugin {
 public:
    TestPlugin();
    virtual ~TestPlugin();
    virtual std::string get() = 0;
    virtual void set(std::string & data) = 0;
};

#endif // TEST_PLUGIN_H
