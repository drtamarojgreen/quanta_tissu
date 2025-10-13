import numpy as np
import unittest

def assert_allclose(a, b, rtol=1e-7, atol=0, msg=""):
    np.testing.assert_allclose(a, b, rtol=rtol, atol=atol, err_msg=msg)

def assert_equal(a, b, msg=""):
    np.testing.assert_equal(a, b, err_msg=msg)

class _AssertRaisesContext(unittest.TestCase):
    def __init__(self, expected_exception):
        super().__init__()
        self._expected_exception = expected_exception

    def __enter__(self):
        self._cm = self.assertRaises(self._expected_exception)
        return self._cm.__enter__()

    def __exit__(self, *args):
        return self._cm.__exit__(*args)

def assert_raises(expected_exception, func, *args, **kwargs):
    context = _AssertRaisesContext(expected_exception)
    with context:
        func(*args, **kwargs)
