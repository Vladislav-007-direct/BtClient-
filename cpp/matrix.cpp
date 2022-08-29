#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "matrix.h"
//#include "util_ocl.hpp"

#ifdef HAVE_OPENCL
bool Matrix::oclEnabled = false;
cl::CommandQueue Matrix::queue;
cl::Context Matrix::context;
cl::Program Matrix::add, Matrix::sub, Matrix::mult;
#endif

// default constructor constructs an ordinary number
Matrix::Matrix()
    : rows(1),
      cols(1),
      _isZero(1),
      prettified(0) {
    this->M.resize(1);
    this->M.reserve(4);
}

// construct a matrix of size rows x cols
Matrix::Matrix(size_t rows, size_t cols)
    : rows(rows),
      cols(cols),
      _isZero(1),
      prettified(0) {
    this->M.resize(rows * cols);
}

Matrix::Matrix(const std::vector<double>& data) {
    this->FromData(data);
}

Matrix::Matrix(const std::vector<std::vector<double>>& data) {
    this->FromData(data);
}

Matrix::Matrix(const std::string fname) {
    if (!this->FromFile(fname)) {
        this->rows = this->cols = 1;
        this->M.resize(1);
        this->M.reserve(4);
        this->_isZero = 0;
        this->prettified = false;
    }
}

Matrix::Matrix(const Matrix& m) {
    rows = m.rows;
    cols = m.cols;
    M.assign(m.M.begin(), m.M.end());
}

// transpose a matrix
Matrix Matrix::T() {

    if (this->IsNum()) return *this;

    Matrix tmp(cols, rows);

    for (size_t a = 0; a < cols; ++a)
        for (size_t b = 0; b < rows; ++b)
            tmp.M[a * rows + b] = this->M[b * cols + a];

    return tmp;
}

Matrix &Matrix::Inverted()
{

}

// return identity matrix
Matrix Matrix::Identity() {

    if (this->IsNum()) {
        Matrix k(1, 1);
        k.Ones();
        return k;
    }

    if (rows != cols)
        throw SizeException("Matrix must be square to have an identity matrix");

    Matrix k(rows, rows);

    for (size_t a = 0; a < rows; a++)
        for (size_t b = 0; b < cols; b++)
            k.M[a * cols + b] = (a == b);

    k._isZero = false;

    return k;
}

// exponential of a matrix (element-wise)
Matrix exp(const Matrix& A) {

    size_t zeros = UNDEFINED;
    Matrix E(A.Rows(), A.Cols());

    for (size_t a = 0; a < A.Rows(); a++)
        for (size_t b = 0; b < A.Cols(); b++) {
            E.M[a * (A.Cols()) + b] = exp(A.M[a * (A.Cols()) + b]);
            CheckZero(&zeros, E.M[a * (A.Cols()) + b]);
        }

    if (zeros == ZERO) E._isZero = true;

    return E;
}

// raise a matrix to power of 2
Matrix sqr(const Matrix& A) {

    if (A.rows != A.cols)
        throw SizeException("Matrix must be square to be raised to power of 2");
    else
        return A * A;
}

Matrix& Matrix::Prettify() {
    this->prettified = 1;
    return (*this);
}

Matrix& Matrix::ShowMaxPrecision() {
    this->prettified = 0;
    return (*this);
}

// generate a random number
double Matrix::_Random(long min, long max) {

#if __cplusplus > 199711L
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);

    return dis(gen);
#else
    return double(min) + double(rand()) / double(RAND_MAX / (max - min));
#endif
}

const std::vector<double> &Matrix::getM() const
{
    return M;
}

// fill a matrix with a constant number
Matrix& Matrix::FillWith(double num) {
    if (num == 0) {
        this->Zeros();
        return *this;
    } else if (num == 1) {
        this->Ones();
        return *this;
    }

    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            M[a * cols + b] = num;

    this->_isZero = false;

    return *this;
}

// fill a matrix with random numbers
Matrix& Matrix::Random(const size_t min, const size_t max) {
    size_t zeros = UNDEFINED;
    this->_isZero = false;
    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b) {
            M[a * cols + b] = double(_Random(min, max));
            CheckZero(&zeros, M[a * cols + b]);
        }

    if (zeros == ZERO) this->_isZero = true;

    return *this;
}

Matrix& Matrix::Zeros() {
    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            M[a * cols + b] = double(0.0);

    this->_isZero = true;
    return *this;
}

Matrix& Matrix::Ones() {
    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            this->M[a * cols + b] = double(1.0);

    this->_isZero = false;
    return *this;
}

Matrix& Matrix::FromData(const std::vector<std::vector<double>>& data) {
    this->_isZero = false;
    this->rows = data.size();
    this->cols = data[0].size();
    this->M.resize((this->rows) * (this->cols));

    size_t zeros = UNDEFINED;

    if ((data.size() == 1) && (data[0].size() == 1) && (!data[0][0]))
        this->_isZero = true;

    for (size_t a = 0; a < this->rows; ++a)
        for (size_t b = 0; b < this->cols; ++b) {
            this->M[a * (this->cols) + b] = data[a][b];
            CheckZero(&zeros, this->M[a * (this->cols) + b]);
        }

    this->prettified = false;

    if (zeros == ZERO) this->_isZero = true;

    return *this;
}

Matrix& Matrix::FromData(const std::vector<double>& data) {
    this->rows = 1;
    this->cols = data.size();
    this->_isZero = false;
    this->M.assign(data.begin(), data.end());

    if ((data.size() == 1) && (!data[0])) this->_isZero = true;

    this->prettified = false;

    return *this;
}

// fill a matrix with data from a file
Matrix& Matrix::FromFile(const std::string fname) {
    std::ifstream f;
    f.open(fname.c_str());

    if (!f.is_open()) throw FileException();
    std::string line;
    cols = rows = 0;
    prettified = false, _isZero = false;
    M.clear();

    size_t columns, zeros = UNDEFINED;
    this->_isZero = false;
    while (std::getline(f, line)) {
        columns = 0;
        line = rtrim(line);
        if (!line.length()) break;

        double value;
        std::istringstream ss(line);

        while (ss >> value) {
            M.push_back(value);
            CheckZero(&zeros, value);
            ++columns;
        }

        ++rows, cols = columns;
    }

    if (zeros == ZERO) this->_isZero = true;

    f.close();
    return *this;
}

void Matrix::Reshape(size_t rows, size_t cols) {
    if (!rows || !cols) throw SizeException("Invalid shape");

    if (this->rows == rows && this->cols == cols) return; // nothing to do here

    std::vector<double>::iterator it;

    if (rows == 1 && cols == 1) {
        this->M.resize(1);
        goto done;
    } else if (rows == 2 && cols == 2) {
        this->M.resize(4);
        goto done;
    } else if (rows == 3 && cols == 3) {
        this->M.resize(9);
        goto done;
    }

    // The simplest case: just resize the array
    // (either losing data either inserting zeros)
    if (this->rows != rows) this->M.resize(rows * (this->cols));

    this->rows = rows;

    it = this->M.begin();

    // Now a bit trickier: it's time to deal with
    // data that is located elsewhere in the array
    if (this->cols > cols) {
        for (size_t r = 0; r < this->rows; ++r) {
            this->M.erase(it + cols, it + this->cols);
            it += cols;
        }
    } else if (this->cols < cols) {
        for (size_t r = 0; r < this->rows; ++r) {
            this->M.insert(it + this->cols, cols - this->cols, 0);
            it += cols;
        }
    }

    this->cols = cols;

done:
    this->rows = rows, this->cols = cols;
    return;
}

#ifdef HAVE_OPENCL
bool Matrix::initOpenCL(std::string add_src, std::string sub_src,
                        std::string mult_src) {
    Matrix::oclEnabled = false;
    cl_int err = CL_SUCCESS;
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);

    if (platforms.empty()) return 0;

    for (auto plat : platforms) {
        std::vector<cl::Device> accelerators, GPUS;
        plat.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &accelerators);
        plat.getDevices(CL_DEVICE_TYPE_GPU, &GPUS);
        devices.insert(devices.end(), accelerators.begin(), accelerators.end());
        devices.insert(devices.end(), GPUS.begin(), GPUS.end());
    }

    if (devices.empty()) return 0;

    Matrix::context = cl::Context(devices, NULL, NULL, NULL, &err);
    if (check_cl_err(err)) return 0;

    Matrix::queue = cl::CommandQueue(Matrix::context, 0, &err);
    if (check_cl_err(err)) return 0;

    std::ifstream add_file, sub_file, mult_file;

    add_file.open(add_src);
    sub_file.open(sub_src);
    mult_file.open(mult_src);

    if ((!add_file.good()) || (!mult_file.good()) || (!sub_file.good())) {
        std::cerr << "[!] Error opening source files!" << std::endl;
        return 0;
    }

    std::string add_source(std::istreambuf_iterator<char>(add_file),
                           (std::istreambuf_iterator<char>()));
    std::string sub_source(std::istreambuf_iterator<char>(sub_file),
                           (std::istreambuf_iterator<char>()));
    std::string mult_source(std::istreambuf_iterator<char>(mult_file),
                            (std::istreambuf_iterator<char>()));

    add_file.close(), sub_file.close(), mult_file.close();

    cl::Program::Sources _add = cl::Program::Sources(
        1, std::make_pair(add_source.c_str(), add_source.length() + 1));
    cl::Program::Sources _sub = cl::Program::Sources(
        1, std::make_pair(sub_source.c_str(), sub_source.length() + 1));
    cl::Program::Sources _mult = cl::Program::Sources(
        1, std::make_pair(mult_source.c_str(), mult_source.length() + 1));

    Matrix::add = cl::Program(Matrix::context, _add);
    Matrix::sub = cl::Program(Matrix::context, _sub);
    Matrix::mult = cl::Program(Matrix::context, _mult);

    err = Matrix::add.build();

    if (check_cl_err(err)) {
        std::cerr << "[!] Error compiling kernel for addition!" << std::endl;
        return 0;
    }

    err = Matrix::sub.build();

    if (check_cl_err(err)) {
        std::cerr << "[!] Error compiling kernel for substraction!"
                  << std::endl;
        return 0;
    }

    err = Matrix::mult.build();

    if (check_cl_err(err)) {
        std::cerr << "[!] Error compiling kernel for multiplication!"
                  << std::endl;
        return 0;
    }

    Matrix::oclEnabled = true;

    for (auto dev : Matrix::context.getInfo<CL_CONTEXT_DEVICES>())
        std::cout << "Accelerator: " << dev.getInfo<CL_DEVICE_VENDOR>() << ' '
                  << dev.getInfo<CL_DEVICE_NAME>() << std::endl;

    std::cout << '\n' << std::endl;

    return 1;
}
#endif

Matrix& Matrix::clear() {
    this->M.clear();
    this->M.push_back((double) 0);
    this->rows = this->cols = 1;

    this->_isZero = true;

    return *this;
}

// append number to a matrix (element-wise)
Matrix Matrix::operator+(const double& right) const {

    if (right == 0.0) return *this;

    Matrix ret = *this;

    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            ret.M[a * cols + b] += right;

    return ret;
}

// add matrix to another matrix
Matrix operator+(const Matrix& left, const Matrix& right) {
    if (left.rows != right.rows || left.cols != right.cols)
        throw SizeException("Size mismatch while adding matrices!");

    Matrix ret = left;

#ifdef HAVE_OPENCL
    if (!Matrix::oclEnabled || (left.rows < 100) && (left.cols < 100)) {
#endif

        for (size_t a = 0; a < left.rows; ++a)
            for (size_t b = 0; b < left.cols; ++b)
                ret.M[a * left.cols + b] += right.M[a * left.cols + b];

#ifdef HAVE_OPENCL
    } else {
        double* data = new double[left.M.size()]();
        cl_int write_err = CL_SUCCESS, err = CL_SUCCESS;

        cl::Buffer _left = cl::Buffer::Buffer(
            Matrix::context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            left.M.size() * sizeof(double), (void*) left.M.data(), &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to allocate buffer, computation failed!"
                      << std::endl;
            return ret;
        }
        write_err = CL_SUCCESS;

        cl::Buffer _right = cl::Buffer::Buffer(
            Matrix::context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            left.M.size() * sizeof(double), (void*) right.M.data(), &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to allocate buffer, computation failed!"
                      << std::endl;
            return ret;
        }
        write_err = CL_SUCCESS;

        cl::Buffer _out = cl::Buffer::Buffer(
            Matrix::context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            left.M.size() * sizeof(double), (void*) data, &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to allocate buffer, computation failed!"
                      << std::endl;
            return ret;
        }
        write_err = CL_SUCCESS;

        cl::Kernel kernel(Matrix::add, "Matrix_add", &err);

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to init kernel, computation failed!"
                      << std::endl;
            return ret;
        }
        err = CL_SUCCESS;

        kernel.setArg(0, _left), kernel.setArg(1, _right),
            kernel.setArg(2, _out), kernel.setArg(3, left.rows),
            kernel.setArg(4, left.cols);

        cl::Event add_done;

#ifdef SLOW
        err = Matrix::queue.enqueueTask(kernel, NULL, &add_done);
#else
        err = Matrix::queue.enqueueNDRangeKernel(
            kernel, cl::NullRange, cl::NDRange(left.rows, left.cols),
            cl::NullRange, NULL, &add_done);
#endif

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to run task, computation failed!"
                      << std::endl;
            return ret;
        }
        err = CL_SUCCESS;

        add_done.wait();

        err = Matrix::queue.enqueueReadBuffer(
            _out, CL_TRUE, 0, left.M.size() * sizeof(double), data);

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to read result, computation failed!"
                      << std::endl;
            return ret;
        }
        err = CL_SUCCESS;

        Matrix::queue.finish();

        // ret.M.assign(data, data + left.M.size());
        ret.M = std::vector<double>(data, data + left.M.size());

        delete[] data;
    }
#endif
    return ret;
}

Matrix& Matrix::operator+=(const Matrix& right) {
    if (rows != right.rows || cols != right.cols)
        throw SizeException("Size mismatch while adding matrices!");

#ifdef HAVE_OPENCL
    if (!Matrix::oclEnabled || ((rows < 100) || (cols < 100))) {
#endif

        for (size_t a = 0; a < rows; ++a)
            for (size_t b = 0; b < cols; ++b)
                this->M[a * cols + b] += right.M[a * cols + b];

#ifdef HAVE_OPENCL
    } else {
        *this = *this + right;
    }
#endif
    return *this;
}

// invert sign of elements of given matrix (element-wise)
Matrix Matrix::operator-() const {
    return (*this) * (-1);
}

// substract a number from a matrix
Matrix Matrix::operator-(const double& right) const {
    if (right == 0.0) return *this;

    Matrix ret = *this;

    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            ret.M[a * cols + b] -= right;

    return ret;
}

// substract a matrix from another matrix
Matrix operator-(const Matrix& left, const Matrix& right) {
    if (left.rows != right.rows || left.cols != right.cols)
        throw SizeException("Size mismatch while substracting matrices!");

    Matrix ret(left.rows, left.cols);

#ifdef HAVE_OPENCL
    if (!Matrix::oclEnabled || ((left.rows < 100) || (left.cols < 100))) {
#endif

        for (size_t a = 0; a < left.rows; ++a)
            for (size_t b = 0; b < left.cols; ++b)
                ret.M[a * left.cols + b] -= right.M[a * left.cols + b];

#ifdef HAVE_OPENCL
    } else {
        double* data = new double[left.M.size()]();
        cl_int write_err = CL_SUCCESS, err = CL_SUCCESS;

        cl::Buffer _left = cl::Buffer::Buffer(
            Matrix::context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            left.M.size() * sizeof(double), (void*) left.M.data(), &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to allocate buffer, computation failed!"
                      << std::endl;
            return ret;
        }
        write_err = CL_SUCCESS;

        cl::Buffer _right = cl::Buffer::Buffer(
            Matrix::context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            left.M.size() * sizeof(double), (void*) right.M.data(), &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to allocate buffer, computation failed!"
                      << std::endl;
            return ret;
        }
        write_err = CL_SUCCESS;

        cl::Buffer _out = cl::Buffer::Buffer(
            Matrix::context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            left.M.size() * sizeof(double), (void*) data, &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to allocate buffer, computation failed!"
                      << std::endl;
            return ret;
        }
        write_err = CL_SUCCESS;

        cl::Kernel kernel(Matrix::sub, "Matrix_sub", &err);

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to init kernel, computation failed!"
                      << std::endl;
            return ret;
        }
        err = CL_SUCCESS;

        kernel.setArg(0, _left), kernel.setArg(1, _right),
            kernel.setArg(2, _out), kernel.setArg(3, left.rows),
            kernel.setArg(4, left.cols);

        cl::Event sub_done;

#ifdef SLOW
        err = Matrix::queue.enqueueTask(kernel, NULL, &sub_done);
#else
        err = Matrix::queue.enqueueNDRangeKernel(
            kernel, cl::NullRange, cl::NDRange(left.rows, left.cols),
            cl::NullRange, NULL, &sub_done);
#endif

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to run task, computation failed!"
                      << std::endl;
            return ret;
        }
        err = CL_SUCCESS;

        sub_done.wait();

        err = Matrix::queue.enqueueReadBuffer(
            _out, CL_TRUE, 0, left.M.size() * sizeof(double), data);

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to read result, computation failed!"
                      << std::endl;
            return ret;
        }
        err = CL_SUCCESS;

        Matrix::queue.finish();

        // ret.M.assign(data, data + left.M.size());
        ret.M = std::vector<double>(data, data + left.M.size());

        delete[] data;
    }
#endif
    return ret;
}

Matrix& Matrix::operator-=(const Matrix& right) {
    if (rows != right.rows || cols != right.cols)
        throw SizeException("Size mismatch while substracting matrices!");

#ifdef HAVE_OPENCL
    if (!Matrix::oclEnabled || ((right.rows < 100) || (right.cols < 100))) {
#endif

        for (size_t a = 0; a < rows; ++a)
            for (size_t b = 0; b < cols; ++b)
                this->M[a * cols + b] -= right.M[a * cols + b];

#ifdef HAVE_OPENCL
    } else {
        *this = *this - right;
    }
#endif

    return *this;
}

// divide a number by a matrix (element-wise)

Matrix operator/(const double d, const Matrix& mat) {
    Matrix ret(mat.rows, mat.cols);

    for (size_t a = 0; a < mat.rows; ++a)
        for (size_t b = 0; b < mat.cols; ++b)
            ret.M[a * (mat.cols) + b] = d / mat.M[a * (mat.cols) + b];

    return ret;
}

// divide a matrix by a number (element-wise)

Matrix Matrix::operator/(const double& right) const {
    Matrix res(rows, cols);

    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            res.M[a * cols + b] = (this->M[a * cols + b]) / right;

    return res;
}

// divide a matrix by a number (element-wise)
Matrix& Matrix::operator/=(const double& right) {

    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            (this->M[a * cols + b]) /= right;

    return *this;
}

// calculate Hadamard product: this (*) right -> element-wise multiplication
Matrix Matrix::Hadamard(const Matrix& right) const {
    if (cols != right.cols || rows != right.rows)
        throw SizeException("Size mismatch while caclulating Hadamard product");

    Matrix res(rows, cols);

    for (size_t a = 0; a < rows; ++a)
        for (size_t b = 0; b < cols; ++b)
            res.M[a * cols + b] =
                (this->M[a * cols + b]) * right.M[a * cols + b];

    return res;
}

// multiply a matrix by another matrix
Matrix Matrix::operator*(const Matrix& right) const {
    if (cols != right.rows) {
        std::string msg =
            std::string("Size mismatch while multiplying matrices: ")
                .append(to_string(rows))
                .append("X")
                .append(to_string(cols));

        msg.append(" vs ")
            .append(to_string(right.rows))
            .append("X")
            .append(to_string(right.cols));

        throw SizeException(msg);
    }

    if (right.IsNum()) return this->operator*(right.M[0]);

    size_t a;

    Matrix res(rows, right.cols);

    if (right.IsCol()) {
        for (a = 0; a < cols; ++a)
            res.M[0] += M[a] * right.M[a];
        return res;
    } else if (this->IsSquare(2) && right.IsSquare(2)) {
        // loop unrolling for 2x2 matrices
        res.M[0] = M[0] * right.M[0] + M[1] * right.M[2],
        res.M[1] = M[0] * right.M[1] + M[1] * right.M[3],
        res.M[2] = M[2] * right.M[0] + M[3] * right.M[2],
        res.M[3] = M[2] * right.M[1] + M[3] * right.M[3];

        return res;
    }

#ifdef HAVE_OPENCL
    if (!Matrix::oclEnabled || (rows < 10) || (right.cols < 10)) {
#endif

        size_t b, c;
        for (a = 0; a < rows; ++a) {
            for (b = 0; b < right.cols; ++b) {
                double tmp;
                for (c = 0, tmp = 0.0; c < cols; ++c)
                    tmp += M[a * cols + c] * right.M[c * right.cols + b];
                res.M[a * right.cols + b] = tmp;
            }
        }

#ifdef HAVE_OPENCL
    } else {
        cl_int write_err = CL_SUCCESS;
        std::unique_ptr<double> data(new double[rows * right.cols]);

        cl::Buffer _left =
            cl::Buffer(Matrix::context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       this->M.size() * sizeof(double), (void*) this->M.data(),
                       &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to create buffer, computation failed!"
                      << std::endl;
            return res;
        }
        write_err = CL_SUCCESS;

        cl::Buffer _right =
            cl::Buffer(Matrix::context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       right.M.size() * sizeof(double), (void*) right.M.data(),
                       &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to create buffer, computation failed!"
                      << std::endl;
            return res;
        }
        write_err = CL_SUCCESS;

        cl::Buffer _out = cl::Buffer(
            Matrix::context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
            rows * right.cols * sizeof(double), data.get(), &write_err);

        if (check_cl_err(write_err)) {
            std::cerr << "[!] Failed to create buffer, computation failed!"
                      << std::endl;
            return res;
        }
        write_err = CL_SUCCESS;

        cl_int err = CL_SUCCESS;
        cl::Kernel kernel(Matrix::mult, "Matrix_mult", &err);

        if (check_cl_err(err)) {
            std::cerr << "\n[!] Failed to create kernel, computation failed!"
                      << std::endl;
            return res;
        }
        err = CL_SUCCESS;

        kernel.setArg(0, _left), kernel.setArg(1, _right),
            kernel.setArg(2, _out), kernel.setArg(3, right.rows),
            kernel.setArg(4, rows), kernel.setArg(5, right.cols),
            kernel.setArg(6, cols);

        std::vector<cl::Event> mult_done(1);

        cl::Event e;

#ifdef SLOW
        err = Matrix::queue.enqueueTask(kernel, NULL, &mult_done);
#else
        err = Matrix::queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                                                 cl::NDRange(rows, right.cols),
                                                 cl::NullRange, 0, &e);
#endif

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to run kernel, computation failed!"
                      << std::endl;
            return res;
        }
        err = CL_SUCCESS;

        e.wait();

        Matrix::queue.finish();

        // err = Matrix::queue.enqueueReadBuffer(_out, CL_TRUE, 0, rows *
        // right.cols * sizeof(double), data.get());
        cl::Event e2;
        void* ret = Matrix::queue.enqueueMapBuffer(
            _out, CL_TRUE, CL_MAP_READ, 0, rows * right.cols * sizeof(double),
            NULL, &e2, &err);

        if (check_cl_err(err)) {
            std::cerr << "[!] Failed to read data from GPU, computation failed!"
                      << std::endl;
            return res;
        }
        err = CL_SUCCESS;

        e2.wait();

        res.M = std::vector<double>((double*) ret,
                                    (double*) ret + rows * right.cols);
    }
#endif

    return res;
}

// multiply a matrix by a number
Matrix Matrix::operator*(const double& right) const {
    Matrix res(rows, cols);

    for (size_t a = 0; a < res.rows; ++a)
        for (size_t b = 0; b < res.cols; ++b)
            res.M[a * cols + b] = (this->M[a * cols + b]) * right;

    return res;
}

// assign a matrix to another matrix
Matrix& Matrix::operator=(const Matrix& mat) {
    if (this != &mat) {
        this->rows = mat.rows;
        this->cols = mat.cols;
        (this->M).assign(mat.M.begin(), mat.M.end());
        this->Reshape(this->rows, this->cols);
        this->prettified = false;
    }

    return *this;
}

// get a row of a matrix or its element
Matrix& Matrix::operator[](const size_t i) const {

    static Matrix ret;

    if (rows != 1) {
        if (i == rows) throw SizeException("Index out of range");

        ret.Reshape(1, cols);

        for (size_t a = 0; a < cols; ++a)
            ret.M[a] = this->M[i * cols + a];
    } else {
        if (i == cols) throw SizeException("Index out of range");

        ret.Reshape(1, 1);
        ret.M[0] = this->M[i];
    }

    return ret;
}
