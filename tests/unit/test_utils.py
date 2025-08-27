import numpy as np
import traceback

def assert_equal(a, b, msg=""):
    """Asserts that two values are equal."""
    if a != b:
        raise AssertionError(f"Assertion Failed: {a} != {b}. {msg}")

def assert_true(x, msg=""):
    """Asserts that a value is True."""
    if not x:
        raise AssertionError(f"Assertion Failed: {x} is not True. {msg}")

def assert_allclose(a, b, msg="", atol=1e-8):
    """Asserts that two numpy arrays are close."""
    if not np.allclose(a, b, atol=atol):
        raise AssertionError(f"Assertion Failed: Arrays are not close.\nArray A:\n{a}\nArray B:\n{b}\n{msg}")

def assert_raises(expected_exception, func, *args, **kwargs):
    """Asserts that a function call raises a specific exception."""
    try:
        func(*args, **kwargs)
    except Exception as e:
        if not isinstance(e, expected_exception):
            stack_trace = traceback.format_exc()
            raise AssertionError(
                f"Assertion Failed: Expected exception {expected_exception}, but got {type(e)}.\n{stack_trace}"
            )
    else:
        raise AssertionError(f"Assertion Failed: Expected exception {expected_exception}, but no exception was raised.")
