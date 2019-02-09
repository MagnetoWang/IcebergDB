/**
 * @file gflags_test.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <gflags/gflags.h>
#include <iostream>

DECLARE_string(test_flags);

int main() {
    std::cout<<"this is gflags test"<<std::endl;
    std::cout<<FLAGS_test_flags<<std::endl;
    std::cout<<"gflags test is over"<<std::endl;
    return 0;
}