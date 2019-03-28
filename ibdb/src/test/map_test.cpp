//
// Created by magnetowang on 2018/12/21.
//
#include <stdio.h>
#include <iostream>
#include <map>

int main() {
    std::map<int, std::string> sort_key;
    sort_key.insert(std::make_pair(1000, "baidu"));
    sort_key.insert(std::make_pair(500, "ali"));
    sort_key.insert(std::make_pair(700, "tengxu"));
    // std::cout<< std::to_string(sort_key) << std::endl;
    for (const auto& iter : sort_key) {
        std::cout<<iter.first<<std::endl;
        // std::cout<<iter.second<<std::endl;
    }
}