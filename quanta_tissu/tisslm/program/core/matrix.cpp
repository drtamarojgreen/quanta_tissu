#include "matrix.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <functional>
#include <vector>
#include <thread>

namespace TissNum {

Matrix::Matrix(const std::vector<size_t>& shape) : shape_(shape) {
    size_t total_size = 1;
    for (size_t dim : shape) total_size *= dim;
    data_.resize(total_size, 0.0f);
}

float& Matrix::operator()(const std::vector<size_t>& indices) {
    if (indices.size() != shape_.size()) throw std::out_of_range("Index dimension mismatch.");
    size_t index = 0;
    for (size_t i = 0; i < shape_.size(); ++i) {
        if (indices[i] >= shape_[i]) throw std::out_of_range("Index out of range.");
        size_t stride = 1;
        for (size_t j = i + 1; j < shape_.size(); ++j) stride *= shape_[j];
        index += indices[i] * stride;
    }
    return data_[index];
}

const float& Matrix::operator()(const std::vector<size_t>& indices) const {
    if (indices.size() != shape_.size()) throw std::out_of_range("Index dimension mismatch.");
    size_t index = 0;
    for (size_t i = 0; i < shape_.size(); ++i) {
        if (indices[i] >= shape_[i]) throw std::out_of_range("Index out of range.");
        size_t stride = 1;
        for (size_t j = i + 1; j < shape_.size(); ++j) stride *= shape_[j];
        index += indices[i] * stride;
    }
    return data_[index];
}

Matrix Matrix::random(const std::vector<size_t>& shape, float mean, float stddev) {
    Matrix m(shape);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(mean, stddev);
    for (size_t i = 0; i < m.data_.size(); ++i) m.data_[i] = dist(gen);
    return m;
}

Matrix Matrix::zeros(const std::vector<size_t>& shape) { return Matrix(shape); }

Matrix Matrix::ones(const std::vector<size_t>& shape) {
    Matrix m(shape);
    std::fill(m.data_.begin(), m.data_.end(), 1.0f);
    return m;
}

Matrix Matrix::transpose() const {
    if (shape_.size() != 2) throw std::invalid_argument("Default transpose only for 2D.");
    return transpose(0, 1);
}

Matrix Matrix::reshape(const std::vector<size_t>& new_shape) const {
    size_t total_size = 1;
    for (size_t dim : new_shape) total_size *= dim;
    if (total_size != data_.size()) throw std::invalid_argument("Size mismatch in reshape.");
    Matrix result(new_shape);
    result.data_ = data_;
    return result;
}

Matrix Matrix::repeat(int n, int axis) const {
    if (axis < 0 || axis >= (int)shape_.size()) throw std::invalid_argument("Invalid axis.");
    std::vector<size_t> new_shape = shape_;
    new_shape[axis] *= n;
    Matrix result(new_shape);
    size_t outer = 1;
    for (int i = 0; i < axis; ++i) outer *= shape_[i];
    size_t axis_size = shape_[axis];
    size_t inner = 1;
    for (size_t i = axis + 1; i < shape_.size(); ++i) inner *= shape_[i];
    for (size_t i = 0; i < outer; ++i) {
        for (int j = 0; j < n; ++j) {
            float* d = result.get_data() + (i * n + j) * axis_size * inner;
            const float* s = get_data() + i * axis_size * inner;
            std::copy(s, s + axis_size * inner, d);
        }
    }
    return result;
}

Matrix Matrix::transpose(int dim1, int dim2) const {
    if (dim1 >= (int)shape_.size() || dim2 >= (int)shape_.size()) throw std::out_of_range("Invalid dims.");
    std::vector<size_t> new_shape = shape_;
    std::swap(new_shape[dim1], new_shape[dim2]);
    Matrix result(new_shape);
    std::vector<size_t> old_strides(shape_.size());
    if (!shape_.empty()) {
        old_strides.back() = 1;
        for (int i = (int)shape_.size() - 2; i >= 0; --i) old_strides[i] = old_strides[i + 1] * shape_[i + 1];
    }
    std::vector<size_t> new_strides(new_shape.size());
    if (!new_shape.empty()) {
        new_strides.back() = 1;
        for (int i = (int)new_shape.size() - 2; i >= 0; --i) new_strides[i] = new_strides[i + 1] * new_shape[i + 1];
    }
    for (size_t i = 0; i < data_.size(); ++i) {
        std::vector<size_t> idx(shape_.size());
        size_t rem = i;
        for (size_t j = 0; j < shape_.size(); ++j) { idx[j] = rem / old_strides[j]; rem %= old_strides[j]; }
        std::vector<size_t> nidx = idx; std::swap(nidx[dim1], nidx[dim2]);
        size_t ni = 0;
        for (size_t j = 0; j < new_shape.size(); ++j) ni += nidx[j] * new_strides[j];
        result.data_[ni] = data_[i];
    }
    return result;
}

Matrix Matrix::operator+(const Matrix& other) const { return broadcast_op(other, std::plus<float>()); }
Matrix Matrix::operator-(const Matrix& other) const { return broadcast_op(other, std::minus<float>()); }
Matrix Matrix::operator*(const Matrix& other) const { return broadcast_op(other, std::multiplies<float>()); }
Matrix Matrix::operator/(const Matrix& other) const { return broadcast_op(other, std::divides<float>()); }

Matrix Matrix::matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() == 2 && b.get_shape().size() == 2) {
        if (a.cols() != b.rows()) throw std::invalid_argument("Dim mismatch.");
        Matrix res({a.rows(), b.cols()});
        unsigned int nt = std::thread::hardware_concurrency();
        std::vector<std::thread> ts;
        size_t rpt = a.rows() / nt;
        for (unsigned int i = 0; i < nt; ++i) {
            size_t s = i * rpt, e = (i == nt - 1) ? a.rows() : s + rpt;
            ts.emplace_back([&, s, e]() {
                for (size_t r = s; r < e; ++r) for (size_t c = 0; c < b.cols(); ++c)
                    for (size_t k = 0; k < a.cols(); ++k) res({r, c}) += a({r, k}) * b({k, c});
            });
        }
        for (auto& t : ts) t.join();
        return res;
    }
    if (a.get_shape().size() == 3 && b.get_shape().size() == 2) {
        if (a.get_shape()[2] != b.get_shape()[0]) throw std::invalid_argument("Dim mismatch.");
        size_t bz = a.get_shape()[0], sl = a.get_shape()[1], d = a.get_shape()[2], nd = b.get_shape()[1];
        Matrix res({bz, sl, nd});
        for (size_t i = 0; i < bz; ++i) for (size_t j = 0; j < sl; ++j) for (size_t k = 0; k < nd; ++k) {
            float sum = 0.0f; for (size_t l = 0; l < d; ++l) sum += a({i, j, l}) * b({l, k});
            res({i, j, k}) = sum;
        }
        return res;
    }
    throw std::invalid_argument("Matmul not implemented.");
}

Matrix Matrix::sum(int axis) const {
    if (axis == -1) {
        Matrix res({1}); res.data_[0] = std::accumulate(data_.begin(), data_.end(), 0.0f);
        return res;
    }
    std::vector<size_t> ns = shape_; ns[axis] = 1; Matrix res(ns);
    std::vector<size_t> idx(shape_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
        size_t t = i; for (int d = (int)shape_.size() - 1; d >= 0; --d) { idx[d] = t % shape_[d]; t /= shape_[d]; }
        std::vector<size_t> nidx = idx; nidx[axis] = 0; res(nidx) += data_[i];
    }
    return res;
}

Matrix Matrix::mean(int axis) const {
    Matrix s = sum(axis);
    if (axis == -1) return s / (float)data_.size();
    return s / (float)shape_[axis];
}

Matrix Matrix::variance(int axis) const {
    if (data_.empty()) throw std::invalid_argument("Empty matrix.");
    if (axis == -1) {
        float m = 0.0f, M2 = 0.0f; size_t c = 0;
        for (float v : data_) { c++; float d = v - m; m += d / c; M2 += d * (v - m); }
        Matrix res({1}); res.data_[0] = M2 / c; return res;
    }
    Matrix m = mean(axis), diff = (*this) - m, sq = diff * diff, ssq = sq.sum(axis);
    std::vector<size_t> ns = shape_; ns[axis] = 1; Matrix res(ns);
    size_t el = shape_[axis];
    for (size_t i = 0; i < res.data_.size(); ++i) res.data_[i] = ssq.data_[i] / (float)el;
    return res;
}

Matrix Matrix::max(int axis) const {
    if (axis == -1) { Matrix res({1}); res.data_[0] = *std::max_element(data_.begin(), data_.end()); return res; }
    std::vector<size_t> ns = shape_; ns[axis] = 1; Matrix res(ns);
    std::fill(res.data_.begin(), res.data_.end(), -std::numeric_limits<float>::infinity());
    std::vector<size_t> idx(shape_.size());
    for (size_t i = 0; i < data_.size(); ++i) {
        size_t t = i; for (int d = (int)shape_.size() - 1; d >= 0; --d) { idx[d] = t % shape_[d]; t /= shape_[d]; }
        std::vector<size_t> nidx = idx; nidx[axis] = 0;
        if (data_[i] > res(nidx)) res(nidx) = data_[i];
    }
    return res;
}

Matrix Matrix::sqrt(const Matrix& m) {
    Matrix res(m.shape_); for (size_t i = 0; i < m.data_.size(); ++i) res.data_[i] = std::sqrt(m.data_[i]);
    return res;
}

Matrix Matrix::pow(const Matrix& m, float e) {
    Matrix res(m.shape_); for (size_t i = 0; i < m.data_.size(); ++i) res.data_[i] = std::pow(m.data_[i], e);
    return res;
}

Matrix Matrix::exp(const Matrix& m) {
    Matrix res(m.shape_); for (size_t i = 0; i < m.data_.size(); ++i) res.data_[i] = std::exp(m.data_[i]);
    return res;
}

Matrix Matrix::operator+(float s) const { Matrix r(shape_); for(size_t i=0; i<data_.size(); ++i) r.data_[i]=data_[i]+s; return r; }
Matrix Matrix::operator-(float s) const { Matrix r(shape_); for(size_t i=0; i<data_.size(); ++i) r.data_[i]=data_[i]-s; return r; }
Matrix Matrix::operator*(float s) const { Matrix r(shape_); for(size_t i=0; i<data_.size(); ++i) r.data_[i]=data_[i]*s; return r; }
Matrix Matrix::operator/(float s) const { if(s==0.0f) throw std::invalid_argument("Div 0"); Matrix r(shape_); for(size_t i=0; i<data_.size(); ++i) r.data_[i]=data_[i]/s; return r; }

Matrix operator/(float s, const Matrix& m) {
    Matrix r(m.shape_); for (size_t i = 0; i < m.data_.size(); ++i) { if(m.data_[i]==0.0f) throw std::invalid_argument("Div 0"); r.data_[i]=s/m.data_[i]; }
    return r;
}

Matrix Matrix::element_wise_product(const Matrix& o) const { return (*this) * o; }
Matrix Matrix::element_wise_division(const Matrix& o) const { return (*this) / o; }
Matrix Matrix::element_wise_sqrt() const {
    Matrix r(shape_); for (size_t i = 0; i < data_.size(); ++i) { if(data_[i]<0.0f) throw std::invalid_argument("Sqrt neg"); r.data_[i] = std::sqrt(data_[i]); }
    return r;
}

Matrix Matrix::concatenate(const Matrix& a, const Matrix& b, int axis) {
    if (a.shape_.size() != b.shape_.size()) throw std::invalid_argument("Dim count mismatch.");
    std::vector<size_t> ns = a.shape_; ns[axis] += b.shape_[axis]; Matrix res(ns);
    size_t o = 1; for (int i = 0; i < axis; ++i) o *= a.shape_[i];
    size_t as = a.shape_[axis], bs = b.shape_[axis], in = 1;
    for (size_t i = axis + 1; i < a.shape_.size(); ++i) in *= a.shape_[i];
    for (size_t i = 0; i < o; ++i) {
        float* d = res.get_data() + i * (as + bs) * in;
        const float* sa = a.get_data() + i * as * in; std::copy(sa, sa + as * in, d);
        const float* sb = b.get_data() + i * bs * in; std::copy(sb, sb + bs * in, d + as * in);
    }
    return res;
}

Matrix operator*(float s, const Matrix& m) { return m * s; }

Matrix Matrix::batch_matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() != 4 || b.get_shape().size() != 4) throw std::invalid_argument("4D only.");
    size_t bz = a.get_shape()[0], nh = a.get_shape()[1], slq = a.get_shape()[2], slk = b.get_shape()[3], hd = a.get_shape()[3];
    Matrix res({bz, nh, slq, slk});
    for (size_t i = 0; i < bz; ++i) for (size_t j = 0; j < nh; ++j) for (size_t l = 0; l < slq; ++l) for (size_t m = 0; m < slk; ++m) {
        float sum = 0.0f; for (size_t n = 0; n < hd; ++n) sum += a({i, j, l, n}) * b({i, j, n, m});
        res({i, j, l, m}) = sum;
    }
    return res;
}

Matrix Matrix::matmul(const Matrix& o) const {
    if (shape_.size() == 2 && o.shape_.size() == 2) {
        if (shape_[1] != o.shape_[0]) throw std::invalid_argument("Dim mismatch.");
        Matrix r({shape_[0], o.shape_[1]});
        for (size_t i = 0; i < shape_[0]; ++i) for (size_t j = 0; j < o.shape_[1]; ++j) {
            float sum = 0; for (size_t k = 0; k < shape_[1]; ++k) sum += (*this)({i, k}) * o({k, j});
            r({i, j}) = sum;
        }
        return r;
    }
    throw std::invalid_argument("Not implemented.");
}

}
