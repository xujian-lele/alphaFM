#ifndef FTRL_PREDICTOR_H_
#define FTRL_PREDICTOR_H_

#include "../Frame/pc_frame.h"
#include "predict_model.h"
#include "../Sample/fm_sample_from_txt.h"
#include "../Sample/fm_sample.h"


struct predictor_option
{
    predictor_option() : factor_num(8), threads_num(1), model_format("txt"), column_name(""), combine_schema(""), input_sample_format("libsvm"), predict_out_format("") {}
    string model_path, model_format, predict_path, model_number_type, column_name, combine_schema, input_sample_format,predict_out_format;
    int threads_num, factor_num;
    
    void parse_option(const vector<string>& args)
    {
        int argc = args.size();
        if(0 == argc)
            throw invalid_argument("invalid command\n");
        for(int i = 0; i < argc; ++i)
        {
            if(args[i].compare("-m") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_path = args[++i];
            }
            else if(args[i].compare("-mf") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_format = args[++i];
                if("bin" != model_format && "txt" != model_format)
                    throw invalid_argument("invalid command\n");
            }
            else if(args[i].compare("-dim") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                factor_num = stoi(args[++i]);
            }
            else if(args[i].compare("-core") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                threads_num = stoi(args[++i]);
            }
            else if(args[i].compare("-out") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                predict_path = args[++i];
            }
            else if(args[i].compare("-mnt") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                model_number_type = args[++i];
            }
            else if(args[i].compare("-cn") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                column_name = args[++i];
            }
            else if(args[i].compare("-cs") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                combine_schema = args[++i];
            }
            else if(args[i].compare("-isf") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                input_sample_format = args[++i];
            }
            else if(args[i].compare("-pof") == 0)
            {
                if(i == argc - 1)
                    throw invalid_argument("invalid command\n");
                predict_out_format = args[++i];
            }
            else
            {
                throw invalid_argument("invalid command\n");
                break;
            }
        }
    }
};


template<typename T>
class ftrl_predictor : public pc_task
{
public:
    ftrl_predictor(const predictor_option& opt);
    ~ftrl_predictor();
    virtual void run_task(vector<string>& dataBuffer);
    
private:
    predict_model<T>* pModel;
    ofstream fPredict;
    mutex outMtx;
    std::string input_sample_format;
    std::string predict_out_format;
};


template<typename T>
ftrl_predictor<T>::ftrl_predictor(const predictor_option& opt)
{
    pModel = new predict_model<T>(opt.factor_num);
    cout << "load model..." << endl;
    if(!pModel->load_model(opt.model_path, opt.model_format))
    {
        cerr << "load model error!" << endl;
        exit(1);
    }
    cout << "model loading finished" << endl;
    fPredict.open(opt.predict_path, ofstream::out);
    if(!fPredict)
    {
        cerr << "open output file error!" << endl;
        exit(1);
    }
    input_sample_format = opt.input_sample_format;
    predict_out_format = opt.predict_out_format;
}


template<typename T>
ftrl_predictor<T>::~ftrl_predictor()
{
    fPredict.close();
}


template<typename T>
void ftrl_predictor<T>::run_task(vector<string>& dataBuffer)
{
    vector<string> outputVec(dataBuffer.size());
    for(size_t i = 0; i < dataBuffer.size(); ++i)
    {
        if (input_sample_format == "txt") {
            fm_sample_from_txt sample(dataBuffer[i]);
            if (sample.is_sample_valid)
            {
                double score = pModel->get_score(sample.x, pModel->muBias->wi, pModel->muMap);
                if (predict_out_format == "only_label_and_score") {
                    outputVec[i] = to_string(sample.y) + " " + to_string(score);
                } else {
                    outputVec[i] = dataBuffer[i] + "\002" + to_string(score);
                }
            }

        } else {
            fm_sample sample(dataBuffer[i]);
            double score = pModel->get_score(sample.x, pModel->muBias->wi, pModel->muMap);
            outputVec[i] = dataBuffer[i] + "\002" + to_string(score);
            if (predict_out_format == "only_label_and_score") {
                outputVec[i] = to_string(sample.y) + " " + to_string(score);
            } else {
                outputVec[i] = dataBuffer[i] + " " + to_string(score);
            }
        }
    }
    outMtx.lock();
    for(size_t i = 0; i < outputVec.size(); ++i)
    {
        if (!outputVec[i].empty())
            fPredict << outputVec[i] << endl;
    }
    outMtx.unlock();
}


#endif /*FTRL_PREDICTOR_H_*/