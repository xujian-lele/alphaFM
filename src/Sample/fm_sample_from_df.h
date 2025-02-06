#ifndef FM_SAMPLE_FROM_DF_H_
#define FM_SAMPLE_FROM_DF_H_

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <functional>
#include "../Utils/utils.h"

using namespace std;


class fm_sample
{
public:
    int y;
    vector<pair<string, double> > x;
    fm_sample(const string& line);
    // std::vector<std::string> split(const std::string& s, char delimiter);
    std::vector<std::vector<std::string>> cartesian_product(const std::vector<std::vector<std::string>>& vectors);
    bool is_valid_value(const std::string& value);
    static std::vector<std::string> column_names;
    static int column_names_size;
    static std::vector<std::string> combine_schema;
    static int combine_schema_size;
    // 接收字符串参数的静态初始化函数
    static void init_column_names(const std::string& input) {
        std::istringstream iss(input);
        std::string item;
        // 按逗号分割字符串
        while (std::getline(iss, item, ',')) {
            fm_sample::column_names.push_back(item);
        }
        fm_sample::column_names_size = fm_sample::column_names.size();
    }

    static void init_combine_schema(const std::string& input) {
        std::istringstream iss(input);
        std::string item;
        // 按逗号分割字符串
        while (std::getline(iss, item, ',')) {
            fm_sample::combine_schema.push_back(item);
        }
        fm_sample::combine_schema_size = fm_sample::combine_schema.size();
    }

private:
    static const string spliter;
    static const string innerSpliter;
};

const string fm_sample::spliter = "\002";
const string fm_sample::innerSpliter = "\001";

// 生成笛卡尔积函数
std::vector<std::vector<std::string>> fm_sample::cartesian_product(const std::vector<std::vector<std::string>>& vectors) {
    std::vector<std::vector<std::string>> result = {{}};
    for (const auto& vec : vectors) {
        std::vector<std::vector<std::string>> temp;
        for (const auto& res : result) {
            for (const auto& val : vec) {
                std::vector<std::string> new_res = res;
                new_res.push_back(val);
                temp.push_back(new_res);
            }
        }
        result = std::move(temp);
    }
    return result;
}

// 筛选符合条件的 value
bool fm_sample::is_valid_value(const std::string& value) {
    return!value.empty() && value != "none" && value.find(':') == std::string::npos;
}

fm_sample::fm_sample(const string& line)
{
    this->x.clear();
    std::vector<std::string> row;
    row.reserve(column_names_size+1);
    utils::split(line, '\002', row);
    // 为了方便根据列名获取列索引
    std::unordered_map<std::string, int> column_index;
    for (int i = 0; i < column_names.size(); ++i) {
        column_index[column_names[i]] = i;
    }

    int label = atoi(row.at(0).c_str());
    this->y = label > 0 ? 1 : -1;

    // 遍历 combine_schema 中的每一个元素
    for (const auto& schema : combine_schema) {
        if (!schema.empty() && schema[0] == '#') {
                continue;
        }

        if (schema.find('#') == std::string::npos) {
            // 单字段情况，如 'a', 'b', 'c'
            int col_idx = column_index[schema];
            std::vector<std::string> values;
            values.reserve(1);
            utils::split(row[col_idx], '\001', values);
            for (const auto& value : values) {
                if (is_valid_value(value)) {
                    this->x.push_back(make_pair(schema + '=' + value, 1));
                }
            }
        } else {
            // 组合字段情况，如 'a#b', 'a#c' 等
            std::vector<std::string> cols;
            cols.reserve(3);
            utils::split(schema, '#', cols);
            std::vector<std::vector<std::string>> col_values;
            col_values.reserve(cols.size());
            for (const auto& col : cols) {
                int col_idx = column_index[col];
                std::vector<std::string> values;
                values.reserve(3);
                utils::split(row[col_idx], '\001', values);
                std::vector<std::string> named_values;
                named_values.reserve(values.size());
                for (const auto& value : values) {
                    if (is_valid_value(value)) {
                        named_values.push_back(col + "=" + value);
                    }
                }
                col_values.push_back(named_values);
            }
            // 生成笛卡尔积
            std::vector<std::vector<std::string>> cartesian = cartesian_product(col_values);
            std::vector<std::string> combined_values;
            combined_values.reserve(cartesian.size());
            for (const auto& item : cartesian) {
                std::string combined = std::accumulate(std::next(item.begin()), item.end(), item[0],
                                                        [](const std::string& a, const std::string& b) {
                                                            return a + '#' + b;
                                                        });
                combined_values.push_back(combined);
            }
            for (const auto& value : combined_values) {
                this->x.push_back(make_pair(value, 1));
            }
        }
    }
}


#endif /*FM_SAMPLE_FROM_DF_H_*/
