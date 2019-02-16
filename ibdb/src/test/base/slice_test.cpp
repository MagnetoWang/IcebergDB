#include "base/slice.h"
#include <iostream>

int main() {
    std::cout<< "base slice test" << std::endl;
    std::string string_test("i am wangzixian");
    ibdb::base::Slice slice_test(string_test);
    std::cout<< slice_test.data() << std::endl;
}