#include "MatrixMultiplier.h"
#include <stdexcept>
#include <thread>
#include <vector>

namespace TissNum {

Matrix MatrixMultiplier::matmul(const Matrix& a, const Matrix& b) {
    size_t a_dims = a.get_shape().size();
    size_t b_dims = b.get_shape().size();

    if (a_dims == 2 && b_dims == 2) {
        if (a.get_shape()[1] != b.get_shape()[0]) throw std::invalid_argument("Dim mismatch (2D).");
        Matrix res({a.get_shape()[0], b.get_shape()[1]});
        unsigned int nt = std::thread::hardware_concurrency();
        std::vector<std::thread> ts;
        size_t rpt = a.get_shape()[0] / nt;
        for (unsigned int i = 0; i < nt; ++i) {
            size_t s = i * rpt, e = (i == nt - 1) ? a.get_shape()[0] : s + rpt;
            ts.emplace_back([&, s, e]() {
                for (size_t r = s; r < e; ++r) for (size_t c = 0; c < b.get_shape()[1]; ++c)
                    for (size_t k = 0; k < a.get_shape()[1]; ++k) res({r, c}) += a({r, k}) * b({k, c});
            });
        }
        for (auto& t : ts) t.join();
        return res;
    }

    if (a_dims == 3 && b_dims == 2) {
        if (a.get_shape()[2] != b.get_shape()[0]) throw std::invalid_argument("Dim mismatch (3Dx2D).");
        Matrix res({a.get_shape()[0], a.get_shape()[1], b.get_shape()[1]});
        for (size_t i = 0; i < a.get_shape()[0]; ++i) for (size_t j = 0; j < a.get_shape()[1]; ++j)
            for (size_t k = 0; k < b.get_shape()[1]; ++k) for (size_t l = 0; l < a.get_shape()[2]; ++l)
                res({i, j, k}) += a({i, j, l}) * b({l, k});
        return res;
    }

    throw std::invalid_argument("Unsupported matmul dimensions.");
}

Matrix MatrixMultiplier::batch_matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() != 3 || b.get_shape().size() != 3) throw std::invalid_argument("3D only.");
    if (a.get_shape()[0] != b.get_shape()[0]) throw std::invalid_argument("Batch size mismatch.");
    if (a.get_shape()[2] != b.get_shape()[1]) throw std::invalid_argument("Inner dim mismatch.");

    Matrix res({a.get_shape()[0], a.get_shape()[1], b.get_shape()[2]});
    for (size_t i = 0; i < a.get_shape()[0]; ++i) for (size_t j = 0; j < a.get_shape()[1]; ++j)
        for (size_t k = 0; k < b.get_shape()[2]; ++k) for (size_t l = 0; l < a.get_shape()[2]; ++l)
            res({i, j, k}) += a({i, j, l}) * b({i, l, k});
    return res;
}

}
