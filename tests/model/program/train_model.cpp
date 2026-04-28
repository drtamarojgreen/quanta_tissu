#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include "quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "quanta_tissu/tisslm/program/training/trainer.h"
#include "quanta_tissu/tisslm/program/training/dataset.h"
#include "quanta_tissu/tisslm/program/training/optimizer.h"
#include "quanta_tissu/tisslm/program/training/loss_function.h"
#include "tests/model/analyzer/error_handler.hpp"
#include "config/TestConfig.h"
#include "tissdb/json/json.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <glob.h>
#include <cstring>
#include <filesystem>

// Configuration struct to hold dynamic values
struct DynamicConfig {
    int n_embd = TestConfig::EmbedDim;
    int n_layer = TestConfig::NumLayers;
    int n_head = TestConfig::NumHeads;
    int d_ff = TestConfig::FFNDim;
    float dropout_rate = TestConfig::DropoutRate;
    float layer_norm_eps = 1e-6;
    int max_seq_len = TestConfig::MaxSeqLen;
    int lora_rank = TestConfig::LoraRank;

    int num_epochs = 3;
    int batch_size = 1;
    float learning_rate = 1e-4;
};

DynamicConfig load_dynamic_config(const std::string& path) {
    DynamicConfig config;
    
    // 1. Load from file if exists
    std::ifstream f(path);
    if (f) {
        std::stringstream buffer;
        buffer << f.rdbuf();
        try {
            auto json = TissDB::Json::JsonValue::parse(buffer.str());
            if (json.is_object()) {
                auto obj = json.as_object();
                if (obj.count("model") && obj.at("model").is_object()) {
                    auto m = obj.at("model").as_object();
                    if (m.count("n_embd")) config.n_embd = (int)m.at("n_embd").as_number();
                    if (m.count("n_layer")) config.n_layer = (int)m.at("n_layer").as_number();
                    if (m.count("n_head")) config.n_head = (int)m.at("n_head").as_number();
                    if (m.count("d_ff")) config.d_ff = (int)m.at("d_ff").as_number();
                    if (m.count("dropout_rate")) config.dropout_rate = (float)m.at("dropout_rate").as_number();
                    if (m.count("layer_norm_eps")) config.layer_norm_eps = (float)m.at("layer_norm_eps").as_number();
                }
                if (obj.count("training") && obj.at("training").is_object()) {
                    auto t = obj.at("training").as_object();
                    if (t.count("num_epochs")) config.num_epochs = (int)t.at("num_epochs").as_number();
                    if (t.count("batch_size")) config.batch_size = (int)t.at("batch_size").as_number();
                    if (t.count("learning_rate")) config.learning_rate = (float)t.at("learning_rate").as_number();
                }
            }
        } catch (...) {
            std::cerr << "Warning: Failed to parse test_config.json, using defaults." << std::endl;
        }
    }

    // 2. Override with Environment Variables (TISSLM_ prefix)
    char* env_val;
    if ((env_val = std::getenv("TISSLM_N_EMBD"))) config.n_embd = std::atoi(env_val);
    if ((env_val = std::getenv("TISSLM_N_LAYER"))) config.n_layer = std::atoi(env_val);
    if ((env_val = std::getenv("TISSLM_N_HEAD"))) config.n_head = std::atoi(env_val);
    if ((env_val = std::getenv("TISSLM_D_FF"))) config.d_ff = std::atoi(env_val);
    if ((env_val = std::getenv("TISSLM_DROPOUT_RATE"))) config.dropout_rate = std::atof(env_val);
    if ((env_val = std::getenv("TISSLM_NUM_EPOCHS"))) config.num_epochs = std::atoi(env_val);
    if ((env_val = std::getenv("TISSLM_BATCH_SIZE"))) config.batch_size = std::atoi(env_val);
    if ((env_val = std::getenv("TISSLM_LEARNING_RATE"))) config.learning_rate = std::atof(env_val);

    return config;
}

// Function to load corpus from a directory
std::string load_corpus(const std::string& path) {
    std::stringstream corpus_stream;
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    std::string pattern = path + "/*.txt";
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        std::cerr << "glob() failed with return_value " << return_value << std::endl;
        return "";
    }

    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        std::ifstream file(glob_result.gl_pathv[i]);
        if (file) {
            corpus_stream << file.rdbuf();
            file.close();
        }
    }
    globfree(&glob_result);
    return corpus_stream.str();
}

void run_training() {
    std::cout << "=== Running C++ Model Training ===" << std::endl;

    DynamicConfig config = load_dynamic_config("test_config.json");

    // --- 1. Load Corpus ---
    std::cout << "[1/5] Loading corpus..." << std::endl;
    std::string corpus = load_corpus("./corpus");
    if (corpus.empty()) {
        std::cerr << "Failed to load corpus. Exiting." << std::endl;
        return;
    }
    std::cout << "Corpus loaded." << std::endl;

    // --- 2. Train Tokenizer ---
    std::cout << "[2/5] Training tokenizer..." << std::endl;
    auto tokenizer = std::make_shared<TissLM::Tokenizer::Tokenizer>("");
    tokenizer->train(corpus, 4196);
    tokenizer->save("trained_tokenizer");
    std::cout << "Tokenizer trained and saved." << std::endl;

    // --- 3. Create Dataset ---
    std::cout << "[3/5] Creating dataset..." << std::endl;
    std::vector<int> token_ids = tokenizer->encode(corpus);
    TissLM::Training::TokenDataset dataset(token_ids, config.max_seq_len);
    std::cout << "Dataset created." << std::endl;

    // --- 4. Initialize Training Components ---
    std::cout << "[4/5] Initializing training components..." << std::endl;
    int vocab_size = tokenizer->get_vocab_size();
    auto model = std::make_shared<TissLM::Core::TransformerModel>(
        vocab_size,
        config.max_seq_len,
        config.n_embd,
        config.n_head,
        config.n_layer,
        config.d_ff,
        config.dropout_rate,
        config.lora_rank
    );
    auto optimizer = std::make_shared<TissLM::Training::Adam>(config.learning_rate);
    auto loss_function = std::make_shared<TissLM::Training::CrossEntropyLoss>();
    std::cout << "Training components initialized." << std::endl;

    // --- 5. Run Training ---
    std::cout << "[5/5] Starting training..." << std::endl;

    const std::string checkpoint_dir = "checkpoints";
    std::filesystem::create_directories(checkpoint_dir);
    TissLM::Training::Trainer trainer(model, optimizer, loss_function);
    trainer.train(dataset, config.num_epochs, config.batch_size, 100, checkpoint_dir);
    std::cout << "Training completed." << std::endl;
    std::cout.flush();
}

int main(int argc, char* argv[]) {
    uint32_t session_id = 0;
    if (argc > 1) {
        session_id = static_cast<uint32_t>(std::atoi(argv[1]));
    }

    if (!RMA_INIT(session_id)) {
        std::cerr << "Warning: Could not initialize error handler." << std::endl;
    }

    try {
        run_training();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C++ Training failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
