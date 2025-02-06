#include <iostream>
#include <map>
#include <fstream>
#include "src/Frame/pc_frame.h"
#include "src/FTRL/ftrl_predictor.h"

using namespace std;

string predict_help()
{
    return string(
            "\nusage: cat sample | ./fm_predict [<options>]"
            "\n"
            "\n"
            "options:\n"
            "-m <model_path>: set the model path\n"
            "-mf <model_format>: set the model format, txt or bin\tdefault:txt\n"
            "-dim <factor_num>: dim of 2-way interactions\tdefault:8\n"
            "-core <threads_num>: set the number of threads\tdefault:1\n"
            "-out <predict_path>: set the predict path\n"
            "-mnt <model_number_type>: double or float\tdefault:double\n"
            "-ift <input_format>: libsvm or txt\tdefault:'libsvm'\n"
            "-cn <column_name>: string\tdefault:''\n"
            "-cs <combine_schema>: string\tdefault:''\n"
    );
}


template<typename T>
int predict(const predictor_option& opt)
{
    ftrl_predictor<T> predictor(opt);
    pc_frame frame;
    frame.init(predictor, opt.threads_num);
    frame.run();
    return 0;
}

std::vector<std::string> fm_sample::column_names;
std::vector<std::string> fm_sample::combine_schema;
int fm_sample::column_names_size;
int fm_sample::combine_schema_size;

int main(int argc, char* argv[])
{
    static_assert(sizeof(void *) == 8, "only 64-bit code generation is supported.");
    cin.sync_with_stdio(false);
    cout.sync_with_stdio(false);
    predictor_option opt;
    try
    {
        opt.parse_option(utils::argv_to_args(argc, argv));
        if (opt.input_sample_format == "txt") {
            fm_sample::init_column_names(opt.column_name);
            fm_sample::init_combine_schema(opt.combine_schema);
            cout << "column_name:";
            for (auto& value: fm_sample::column_names) {
                cout << value << " "; 
            }
            cout << "all_size:" << fm_sample::column_names_size << endl;
            cout << "combine_schema:";
            for (auto& value: fm_sample::combine_schema) {
            cout << value << " "; 
            }
            cout << "all_size:" << fm_sample::combine_schema_size << endl;
            cout << endl;
        }
    }
    catch(const invalid_argument& e)
    {
        cerr << e.what() << endl;
        cerr << predict_help() << endl;
        return 1;
    }
    if("float" == opt.model_number_type)
    {
        return predict<float>(opt);
    }
    return predict<double>(opt);
}

