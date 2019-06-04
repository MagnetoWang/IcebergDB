/**
 * @file folly_test.cpp
 * @author MagnetoWang
 * @brief 
 * @version 0.1
 * @date 2019-02-09
 * 
 * @copyright Copyright (c) 2019
 * 
 */
// #include<iostream>
// #include<folly/FBString.h>

// int main() {
//     std::cout<<"this is folly string test"<<std::endl;
//     folly::fbstring test("hello i am magneto wang");
//     std::cout<<test<<std::endl;
// }

#include <folly/FBVector.h>
int main() {
    folly::fbvector<int> numbers({0, 1, 2, 3});
    numbers.reserve(10);
    for (int i = 4; i < 10; i++) {
        numbers.push_back(i * 2);
    }
    assert(numbers[6] == 12);
    return 0;
}