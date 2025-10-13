import numpy as np
from .parameter import Parameter

def im2col(x, kernel_height, kernel_width, stride=1, padding=1):
    """
    Transforms an image into columns for convolution.

    Args:
        x: Input image of shape (batch_size, channels, height, width).
        kernel_height: Height of the convolution kernel.
        kernel_width: Width of the convolution kernel.
        stride: Stride of the convolution.
        padding: Padding of the convolution.

    Returns:
        Columns of the image.
    """
    batch_size, channels, height, width = x.shape
    out_height = (height + 2 * padding - kernel_height) // stride + 1
    out_width = (width + 2 * padding - kernel_width) // stride + 1

    img = np.pad(x, [(0, 0), (0, 0), (padding, padding), (padding, padding)], 'constant')
    cols = np.zeros((batch_size, channels, kernel_height, kernel_width, out_height, out_width))

    for y in range(kernel_height):
        y_max = y + stride * out_height
        for x_ in range(kernel_width):
            x_max = x_ + stride * out_width
            cols[:, :, y, x_, :, :] = img[:, :, y:y_max:stride, x_:x_max:stride]

    cols = cols.transpose(0, 4, 5, 1, 2, 3).reshape(batch_size * out_height * out_width, -1)
    return cols

def col2im(cols, x_shape, kernel_height, kernel_width, stride=1, padding=1):
    """
    Transforms columns back to an image.

    Args:
        cols: Columns of the image.
        x_shape: Shape of the original image (batch_size, channels, height, width).
        kernel_height: Height of the convolution kernel.
        kernel_width: Width of the convolution kernel.
        stride: Stride of the convolution.
        padding: Padding of the convolution.

    Returns:
        The image.
    """
    batch_size, channels, height, width = x_shape
    out_height = (height + 2 * padding - kernel_height) // stride + 1
    out_width = (width + 2 * padding - kernel_width) // stride + 1
    cols = cols.reshape(batch_size, out_height, out_width, channels, kernel_height, kernel_width).transpose(0, 3, 4, 5, 1, 2)

    img = np.zeros((batch_size, channels, height + 2 * padding + stride - 1, width + 2 * padding + stride - 1))
    for y in range(kernel_height):
        y_max = y + stride * out_height
        for x_ in range(kernel_width):
            x_max = x_ + stride * out_width
            np.add.at(img, (slice(None), slice(None), slice(y, y_max, stride), slice(x_, x_max, stride)), cols[:, :, y, x_, :, :])

    return img[:, :, padding:height + padding, padding:width + padding]

class Conv2D:
    def __init__(self, in_channels, out_channels, kernel_size, stride=1, padding=0, name=""):
        self.in_channels = in_channels
        self.out_channels = out_channels
        self.kernel_size = kernel_size
        self.stride = stride
        self.padding = padding

        self.W = Parameter(np.random.randn(out_channels, in_channels, kernel_size, kernel_size) / np.sqrt(in_channels * kernel_size * kernel_size), name=f"{name}.W")
        self.b = Parameter(np.zeros(out_channels), name=f"{name}.b")

    def __call__(self, x):
        batch_size, _, height, width = x.shape
        out_height = (height + 2 * self.padding - self.kernel_size) // self.stride + 1
        out_width = (width + 2 * self.padding - self.kernel_size) // self.stride + 1

        col = im2col(x, self.kernel_size, self.kernel_size, self.stride, self.padding)
        col_W = self.W.value.reshape(self.out_channels, -1).T

        out = col @ col_W + self.b.value
        out = out.reshape(batch_size, out_height, out_width, -1).transpose(0, 3, 1, 2)

        self.cache = (x, col, col_W)
        return out

    def backward(self, d_out):
        x, col, col_W = self.cache
        batch_size, _, height, width = x.shape

        d_out = d_out.transpose(0, 2, 3, 1).reshape(-1, self.out_channels)

        self.b.grad += np.sum(d_out, axis=0)
        self.W.grad += (col.T @ d_out).T.reshape(self.W.value.shape)

        d_col = d_out @ col_W.T
        dx = col2im(d_col, x.shape, self.kernel_size, self.kernel_size, self.stride, self.padding)

        return dx

    def parameters(self):
        return [self.W, self.b]

class DepthwiseSeparableConv:
    def __init__(self, in_channels, out_channels, kernel_size, stride=1, padding=0, name=""):
        self.depthwise_conv = Conv2D(in_channels, in_channels, kernel_size, stride, padding, name=f"{name}.depthwise_conv")
        self.pointwise_conv = Conv2D(in_channels, out_channels, 1, 1, 0, name=f"{name}.pointwise_conv")

    def __call__(self, x):
        out = self.depthwise_conv(x)
        out = self.pointwise_conv(out)
        return out

    def backward(self, d_out):
        d_out = self.pointwise_conv.backward(d_out)
        d_out = self.depthwise_conv.backward(d_out)
        return d_out

    def parameters(self):
        return self.depthwise_conv.parameters() + self.pointwise_conv.parameters()