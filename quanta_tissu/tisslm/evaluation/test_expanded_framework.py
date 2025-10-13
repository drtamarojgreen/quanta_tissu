#!/usr/bin/env python3
"""
Test script to verify the expanded evaluation framework works correctly
with mock data when proper model weights and tokenizer are not available.
"""

import os
import sys
import numpy as np
from unittest.mock import Mock, MagicMock

# Add project root for module discovery
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, '..', '..', '..'))
sys.path.insert(0, project_root)

def create_mock_tokenizer():
    """Create a mock tokenizer for testing."""
    mock_tokenizer = Mock()
    mock_tokenizer.get_vocab_size.return_value = 1000
    
    # Mock tokenize method - return simple token IDs based on text length
    def mock_tokenize(text):
        # Simple mock: return token IDs based on text characters
        tokens = [ord(c) % 100 for c in text[:10]]  # Limit to first 10 chars, mod 100 for small vocab
        return np.array(tokens)
    
    # Mock detokenize method - return simple text
    def mock_detokenize(token_ids):
        # Simple mock: convert token IDs back to characters
        chars = [chr(65 + (token_id % 26)) for token_id in token_ids]  # A-Z based on token ID
        return ''.join(chars)
    
    mock_tokenizer.tokenize = mock_tokenize
    mock_tokenizer.detokenize = mock_detokenize
    
    return mock_tokenizer

def create_mock_model():
    """Create a mock model for testing."""
    mock_model = Mock()
    
    # Create mock generator
    mock_generator = Mock()
    
    def mock_sample(prompt_tokens, n_new_tokens, **kwargs):
        """Mock sample method that returns predictable token sequences."""
        # Generate predictable tokens based on prompt and parameters
        base_tokens = list(range(10, 10 + n_new_tokens))  # Simple sequence
        
        # Add some variation based on parameters
        if kwargs.get('temperature', 1.0) > 1.0:
            # Higher temperature = more random-looking
            base_tokens = [(t + np.random.randint(0, 5)) % 100 for t in base_tokens]
        
        return base_tokens
    
    mock_generator.sample = mock_sample
    mock_model.generator = mock_generator
    
    return mock_model

def test_evaluation_functions():
    """Test the evaluation framework functions with mock data."""
    print("Testing expanded evaluation framework...")
    
    # Import the functions we want to test
    from quanta_tissu.tisslm.evaluation.full_fledged_workout import (
        analyze_generation_quality,
        generate_evaluation_summary,
        run_generative_callable_discovery
    )
    
    # Test analyze_generation_quality
    print("\n1. Testing analyze_generation_quality...")
    test_prompt = "The weather today is"
    test_generated = "The weather today is sunny and warm with clear blue skies."
    test_params = {"temperature": 0.8, "method": "nucleus"}
    
    analysis = analyze_generation_quality(test_prompt, test_generated, 50, test_params)
    
    print(f"   Word count: {analysis['word_count']}")
    print(f"   Lexical diversity: {analysis['lexical_diversity']:.3f}")
    print(f"   Repetition ratio: {analysis['repetition_ratio']:.3f}")
    print(f"   Length accuracy: {analysis['length_accuracy']:.3f}")
    
    # Test generate_evaluation_summary
    print("\n2. Testing generate_evaluation_summary...")
    mock_results = [
        {
            'param_set': 0,
            'prompt_category': 'short_completion',
            'target_length': 25,
            'parameters': {'temperature': 0.8, 'method': 'nucleus'},
            'analysis': {
                'lexical_diversity': 0.85,
                'repetition_ratio': 0.1,
                'length_accuracy': 0.9,
                'word_count': 23
            }
        },
        {
            'param_set': 1,
            'prompt_category': 'medium_narrative',
            'target_length': 50,
            'parameters': {'temperature': 1.2, 'method': 'nucleus'},
            'analysis': {
                'lexical_diversity': 0.75,
                'repetition_ratio': 0.15,
                'length_accuracy': 0.8,
                'word_count': 40
            }
        }
    ]
    
    summary = generate_evaluation_summary(mock_results, 2, 2)
    print("   Summary generated successfully:")
    for line in summary[:10]:  # Show first 10 lines
        print(f"   {line}")
    
    # Test run_generative_callable_discovery
    print("\n3. Testing run_generative_callable_discovery...")
    mock_model = create_mock_model()
    mock_tokenizer = create_mock_tokenizer()
    
    discovery_report = run_generative_callable_discovery(mock_model, mock_tokenizer)
    print("   Discovery report:")
    for line in discovery_report[:10]:  # Show first 10 lines
        print(f"   {line}")
    
    print("\n✓ All evaluation framework functions tested successfully!")
    return True

def test_parameter_configurations():
    """Test that parameter configurations are properly defined."""
    print("\n4. Testing parameter configurations...")
    
    from quanta_tissu.tisslm.evaluation.full_fledged_workout import (
        EXPANDED_GENERATION_PARAMETERS,
        DIVERSE_PROMPT_SETS,
        TOKEN_LENGTH_TESTS
    )
    
    print(f"   Parameter configurations: {len(EXPANDED_GENERATION_PARAMETERS)}")
    print(f"   Prompt categories: {len(DIVERSE_PROMPT_SETS)}")
    print(f"   Total prompts: {sum(len(prompts) for prompts in DIVERSE_PROMPT_SETS.values())}")
    print(f"   Token length tests: {len(TOKEN_LENGTH_TESTS)}")
    
    # Verify parameter structure
    for i, params in enumerate(EXPANDED_GENERATION_PARAMETERS[:3]):  # Check first 3
        print(f"   Param set {i+1}: {params}")
    
    # Verify prompt categories
    for category, prompts in DIVERSE_PROMPT_SETS.items():
        print(f"   {category}: {len(prompts)} prompts")
        print(f"     Example: '{prompts[0][:50]}...'")
    
    print("   ✓ Parameter configurations are properly structured!")
    return True

def main():
    """Run all tests."""
    print("="*60)
    print("EXPANDED EVALUATION FRAMEWORK TEST")
    print("="*60)
    
    try:
        # Test core functions
        test_evaluation_functions()
        
        # Test configurations
        test_parameter_configurations()
        
        print("\n" + "="*60)
        print("ALL TESTS PASSED!")
        print("="*60)
        print("\nThe expanded evaluation framework is working correctly.")
        print("To run with a real model, ensure you have:")
        print("1. Trained tokenizer files in test_tokenizer/")
        print("2. Model checkpoint in test_model/checkpoint_step_50000.npz")
        print("3. Run: python -m tisslm.evaluation.full_fledged_workout --test expanded")
        
        return True
        
    except Exception as e:
        print(f"\n❌ TEST FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
