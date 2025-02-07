#ifndef FM_SAMPLE_FROM_TXT_TEST_H_
#define FM_SAMPLE_FROM_TXT_TEST_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <functional>
#include "fm_sample_from_txt.h"

using namespace std;
int main() {
    std::vector<std::string> samples = {
        "1\0021\0012\00210\00220\00230\00240",
        "0\0023\0014\00211\00221\00231\00241",
    };
    for (const string& line: samples) {
        fm_sample_from_txt sample(line);
        cout << sample.y << endl;
        for (const auto& key: sample.x) {
            cout << key.first << " " << key.second << endl;
        }
    }
    return 0;
}
#endif /*FM_SAMPLE_FROM_TXT_TEST_H_*/