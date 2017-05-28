#pragma once
#include <vector>
#include <string>
#include <utility>
#include "constants.h"

typedef bool (*test_fn)();

class TestObj{
public:
    using test_ty = std::pair<test_fn,std::string>;
    std::vector<test_ty> tests;
    test_fn add_test(test_fn test, std::string func_name){
        tests.emplace_back(test,func_name);
        return test;
    }
    void run_all();
    void fancy_print(std::string func_str);
    void collect_tests(){
#define _add_test(testname) add_test(testname,#testname)
    }
};

extern TestObj all_tests;
