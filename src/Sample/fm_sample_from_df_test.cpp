#ifndef DF_H_
#define DF_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <functional>
#include "fm_sample_from_df.h"

using namespace std;
int main() {
    // 存储 sample 数据
    std::vector<std::string> samples = {
        "1\0012\00210\00220\00230\00240",
        "3\0014\00211\00221\00231\00241",
    };
    for (const string& line: samples) {
        fm_sample sample(line);
        cout << sample.y << endl;
        for (const auto& key: sample.x) {
            cout << key.first << " " << key.second << endl;
        }
    }
    return 0;
}
#endif /*DF_H_*/