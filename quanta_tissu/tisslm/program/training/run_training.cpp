#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <filesystem>
#include "quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include "loss_function.h"
#include "optimizer.h"
#include "dataset.h"
#include "trainer.h"

namespace fs = std::filesystem;

std::string load_corpus(const std::string& path) {
    std::string res;
    if (fs::is_directory(path)) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::ifstream f(entry.path());
                if (f.is_open()) { std::stringstream b; b << f.rdbuf(); res += b.str() + "\n"; }
            }
        }
    } else {
        std::ifstream f(path);
        if (f.is_open()) { std::stringstream b; b << f.rdbuf(); res = b.str(); }
    }
    return res;
}

int main(int argc, char* argv[]) {
    std::string path = "corpus/corpus.txt";
    if (argc > 1) path = argv[1];
    const std::string dir = "training_output";
    const int vs = 5000, sl = 128, bz = 32, ep = 5;
    const float lr = 1e-3f;

    std::string corpus = load_corpus(path);
    if (corpus.empty()) return 1;

    auto tokenizer = std::make_unique<TissLM::Tokenizer::Tokenizer>("");
    std::string t_pref = dir + "/tokenizer";
    if (fs::exists(t_pref + "_vocab.json")) tokenizer = std::make_unique<TissLM::Tokenizer::Tokenizer>(t_pref);
    else {
        tokenizer->train(corpus, vs, true);
        fs::create_directories(dir);
        tokenizer->save(t_pref);
    }

    std::vector<int> ids = tokenizer->encode(corpus);
    if (ids.size() > 10000) ids.resize(10000);
    TissLM::Training::TokenDataset ds(ids, sl);

    auto model = std::make_shared<TissLM::Core::TransformerModel>(vs, sl, 128, 4, 2, 512, 0.1f);
    auto loss = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    auto opt = std::make_shared<TissLM::Training::Adam>(lr);
    TissLM::Training::Trainer trainer(model, opt, loss);

    std::string final_p = dir + "/final_model.pt";
    if (fs::exists(final_p)) trainer.load_checkpoint(final_p);
    else {
        for (int i = 0; i < ep; ++i) {
            trainer.train(ds, 1, bz);
            trainer.save_checkpoint(dir + "/checkpoint_" + std::to_string(i + 1) + ".pt");
        }
        trainer.save_checkpoint(final_p);
    }

    std::string p = "The quick brown fox";
    std::vector<int> iids = tokenizer->encode(p);
    for (int i = 0; i < 20; ++i) {
        TissNum::Matrix in({1, iids.size()});
        for(size_t j=0; j<iids.size(); ++j) in({0, j}) = (float)iids[j];
        TissNum::Matrix l = model->forward(in, false);
        size_t lti = iids.size() - 1;
        float ml = -1e9; int bi = 0;
        for (int v = 0; v < vs; ++v) {
            float val = l({lti, (size_t)v});
            if (val > ml) { ml = val; bi = v; }
        }
        iids.push_back(bi);
        std::cout << tokenizer->decode({bi}) << std::flush;
    }
    std::cout << std::endl;
    return 0;
}
