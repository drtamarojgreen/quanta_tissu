# TissLM Pipeline Implementation Guide

This document details the end-to-end process for setting up the environment and running the prototype Conversational Analytics Engine. The pipeline involves setting up dependencies, providing a data corpus, training a tokenizer, training the language model, and finally, running the natural language translator.

## Introduction

The goal of this pipeline is to demonstrate a prototype system that can translate a natural language question into a TissQL query. It uses the `QuantaTissu` language model, which is built from scratch in this repository.

**Note:** The `QuantaTissu` model is a small, educational model. To get meaningful results, it must be trained on a large, high-quality corpus of (natural language question, TissQL query) pairs. This guide demonstrates how to make the *pipeline functional*, but the quality of the output will depend entirely on the quality and size of the corpus provided.

## Prerequisites

- Python 3.7+
- `pip` for installing packages

## Step 1: Install Dependencies

The required Python packages are listed in `requirements.txt`. Install them using pip:

```bash
pip install -r requirements.txt
```

## Step 2: Place the Corpus

The model and tokenizer require a text corpus to train on. The training scripts are hard-coded to look for a file at the following path:

`corpus/resiliency_research.txt`

To use your existing corpus, please place your corpus file at this location in the project root. The `corpus/` directory may need to be created first if it does not exist.

## Step 3: Train the BPE Tokenizer

The Word-Piece-Encoding (BPE) tokenizer must be trained on the corpus. This will generate the necessary vocabulary and merge files in the `/models` directory.

Run the training script as a module from the project root:
```bash
python3 -m quanta_tissu.tisslm.train_bpe
```

## Step 4: Train the Language Model

Once the tokenizer is trained, you can train the `QuantaTissu` language model. The training script has been modified to run without a database connection.

Run the training script as a module from the project root:
```bash
python3 -m quanta_tissu.tisslm.train
```
This will generate the trained model weights at `/models/quanta_tissu.npz`.

## Step 5: Run the NLQ Translator

With the tokenizer and model trained, you can now run the Natural Language Query (NLQ) translator. This script will load the trained model and attempt to translate a few hard-coded example questions.

```bash
python3 quanta_tissu/analytics/translator.py
```

The script will print the translated TissQL queries to the console.
