#ifndef FM_SAMPLE_FROM_TXT_H_
#define FM_SAMPLE_FROM_TXT_H_

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


class fm_sample_from_txt
{
public:
    int y;
    vector<pair<string, double> > x;
    fm_sample_from_txt(const string& line);
    std::vector<std::vector<std::string>> cartesian_product(const std::vector<std::vector<std::string>>& vectors);
    bool is_valid_value(const std::string& value);
    bool is_sample_valid = true;
    static std::vector<std::string> column_names;
    static int column_names_size;
    static std::vector<std::string> combine_schema;
    static int combine_schema_size;
    static std::unordered_map<std::string, int> column_index;

    // 接收字符串参数的静态初始化函数
    static void init_column_names(const std::string& input) {
        std::istringstream iss(input);
        std::string item;
        // 按逗号分割字符串
        while (std::getline(iss, item, ',')) {
            fm_sample_from_txt::column_names.push_back(item);
        }
        fm_sample_from_txt::column_names_size = fm_sample_from_txt::column_names.size();
    }

    static void init_combine_schema(const std::string& input) {
        std::istringstream iss(input);
        std::string item;
        // 按逗号分割字符串
        while (std::getline(iss, item, ',')) {
            fm_sample_from_txt::combine_schema.push_back(item);
        }
        fm_sample_from_txt::combine_schema_size = fm_sample_from_txt::combine_schema.size();
    }
    
    static void init_column_index() {
        // 为了方便根据列名获取列索引
        for (int i = 0; i < fm_sample_from_txt::column_names_size; ++i) {
            fm_sample_from_txt::column_index[fm_sample_from_txt::column_names[i]] = i;
        }
    }

private:
    static const char spliter;
    static const char innerSpliter;
};

const char fm_sample_from_txt::spliter = '\002';
const char fm_sample_from_txt::innerSpliter = '\001';

// 生成笛卡尔积函数
std::vector<std::vector<std::string>> fm_sample_from_txt::cartesian_product(const std::vector<std::vector<std::string>>& vectors) {
    std::vector<std::vector<std::string>> result = {{}};
    for (const auto& vec : vectors) {
        size_t temp_size = result.size() * vec.size();
        std::vector<std::vector<std::string>> temp;
        temp.reserve(temp_size);
        for (const auto& res : result) {
            for (const auto& val : vec) {
                temp.emplace_back(res);
                temp.back().push_back(val);
            }
        }
        result = std::move(temp);
    }
    return result;
}

// 筛选符合条件的 value
bool fm_sample_from_txt::is_valid_value(const std::string& value) {
    return!value.empty() && value != "none" && value.find(':') == std::string::npos;
}

fm_sample_from_txt::fm_sample_from_txt(const string& line)
{
    try{
        this->x.clear();
        std::vector<std::string> row;
        row.reserve(column_names_size+1);
        utils::split(line, spliter, row);
        if (row.size() != column_names_size) {
            throw length_error("length is not same.column_name:" + to_string(column_names_size) + ",sample:" + to_string(row.size()));
        }

        // // 为了方便根据列名获取列索引
        // std::unordered_map<std::string, int> column_index;
        // for (int i = 0; i < column_names.size(); ++i) {
        //     column_index[column_names[i]] = i;
        // }

        // 列中第一个元素为label!
        int label = atoi(row.at(0).c_str());
        this->y = label > 0 ? 1 : -1;

        // 遍历 combine_schema 中的每一个元素
        for (const auto& schema : combine_schema) {
            if (!schema.empty() && schema[0] == '#') {
                    continue;
            }

            if (schema.find('#') == std::string::npos) {
                // 单字段情况，如 'a', 'b', 'c'
                // int col_idx = column_index[schema];
                auto it = column_index.find(schema);
                if (it == column_index.end()) {
                    throw out_of_range("column_name not contains " + schema + ". Please check column_name and combine_schema!program exist!");
                    continue;
                }
                int col_idx = it->second;

                std::vector<std::string> values;
                values.reserve(3);
                utils::split(row[col_idx], innerSpliter, values);
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
                    auto it = column_index.find(col);
                    if (it == column_index.end()) {
                        throw out_of_range("column_name not contains " + col + ". Please check column_name and combine_schema!program exist!");
                        continue;
                    }
                    int col_idx = it->second;

                    std::vector<std::string> values;
                    values.reserve(3);
                    utils::split(row[col_idx], '\001', values);

                    std::vector<std::string> named_values;
                    named_values.reserve(values.size());

                    for (const auto& value : values) {
                        if (is_valid_value(value)) {
                            named_values.emplace_back(col);
                            named_values.back() += "=";
                            named_values.back() += value;
                        }
                    }
                    col_values.emplace_back(std::move(named_values));
                }
                // 生成笛卡尔积
                std::vector<std::vector<std::string>> cartesian = cartesian_product(col_values);
                std::vector<std::string> combined_values;
                combined_values.reserve(cartesian.size());
                this->x.reserve(this->x.size() + cartesian.size());

                for (const auto& item : cartesian) {
                    if (item.empty()) continue;

                    std::string combined = item[0];
                    // 手动拼接字符串，避免临时对象创建
                    for (size_t i = 1; i < item.size(); ++i) {
                        combined += '#';
                        combined += item[i];
                    }
                    // 直接添加到结果容器
                    this->x.emplace_back(std::move(combined), 1);
                }
            }
        }
    } 
    catch(const length_error& e)
    {
        cerr << "length_error:" << e.what() << endl;
        this->is_sample_valid = false;
    }
    catch(const out_of_range& e)
    {
        cerr << "out_of_range:" << e.what() << endl;
        this->is_sample_valid = false;
        exit(1);
    }
}


#endif /*FM_SAMPLE_FROM_TXT_H_*/
