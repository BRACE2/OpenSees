/* 
 * Objectives:
 * - little to no overhead above C-style arrays
 * - value semantics; objects do not decay to pointers;
 *
 * This code is influenced by the following sources
 *  list initialization:
 *  - https://stackoverflow.com/questions/42068882/list-initialization-for-a-matrix-class
 *
 *  style/practices
 *  - https://quuxplusone.github.io/blog/2021/04/03/static-constexpr-whittling-knife/
 * 
 *  Operator overloading / semantics
 *  - https://stackoverflow.com/questions/9851188/does-it-make-sense-to-use-move-semantics-for-operator-and-or-operator/9851423#9851423
 *
 *  compile-time template restrictions/concepts:
 *  - https://codereview.stackexchange.com/questions/259038/compile-time-matrix-class
 *    (C++ 20)
 *  - https://github.com/calebzulawski/cotila/
 *    (C++ 17)
 */

// Claudio Perez
#ifndef VectorND_H
#define VectorND_H
#include <math.h>
#include <assert.h>
#include <array>
#include <stdexcept>
#include <functional>
#include <iostream> // overloading <<
                    //
#include <Vector.h>
#include <Matrix.h>
#include "blasdecl.h"

#if __cplusplus < 202000L
#define consteval
#define requires(X)
#endif

namespace OpenSees {

typedef int index_t;
// forward declaration
template<int n, int m, typename T> struct MatrixND;


template <index_t N, typename T=double> 
requires(N > 0)
struct VectorND {
  T values[N];

  operator Vector() { return Vector(values, N);}

  template<int n, int m, typename> friend struct MatrixND;

  constexpr T&
  operator[](index_t index) {return values[index];}
  constexpr const T&
  operator[](index_t index) const {return values[index];}

  constexpr int
  size() const {
    return N;
  }

  void
  zero() {
    for (index_t i = 0; i < N; ++i)
      values[i] = 0.0;
  }

  int
  addVector(T thisFact, Vector &other, T otherFact) {
    if (otherFact == 0.0 && thisFact == 1.0)
      return 0; 

    else if (thisFact == 1.0) {
      // want: this += other * otherFact
      double *dataPtr = values;
      double *otherDataPtr = other.theData;
      if (otherFact == 1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) 
          *dataPtr++ += *otherDataPtr++;
      } else if (otherFact == -1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) 
          *dataPtr++ -= *otherDataPtr++;
      } else 
        for (int i=0; i<N; i++) 
          *dataPtr++ += *otherDataPtr++ * otherFact;

    } else if (thisFact == 0.0) {
        // want: this = other * otherFact
        double *dataPtr = values;
        double *otherDataPtr = other.theData;
        if (otherFact == 1.0) { // no point doing a multiplication if otherFact == 1.0
          for (int i=0; i<N; i++) 
            *dataPtr++ = *otherDataPtr++;
        } else if (otherFact == -1.0) { // no point doing a multiplication if otherFact == 1.0
          for (int i=0; i<N; i++) 
            *dataPtr++ = -(*otherDataPtr++);
        } else 
          for (int i=0; i<N; i++) 
            *dataPtr++ = *otherDataPtr++ * otherFact;
    } else {
      // want: this = this * thisFact + other * otherFact
      double *dataPtr = values;
      double *otherDataPtr = other.theData;
      if (otherFact == 1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) {
          double value = *dataPtr * thisFact + *otherDataPtr++;
          *dataPtr++ = value;
        }
      } else if (otherFact == -1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) {
          double value = *dataPtr * thisFact - *otherDataPtr++;
          *dataPtr++ = value;
        }
      } else 
        for (int i=0; i<N; i++) {
          double value = *dataPtr * thisFact + *otherDataPtr++ * otherFact;
          *dataPtr++ = value;
      }
    }

    // successfull
    return 0;
  }

  int
  addVector(T thisFact, VectorND<N> &other, T otherFact) {
    if (otherFact == 0.0 && thisFact == 1.0)
      return 0; 

    else if (thisFact == 1.0) {
      // want: this += other * otherFact
      double *dataPtr = values;
      double *otherDataPtr = other.values;
      if (otherFact == 1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) 
          *dataPtr++ += *otherDataPtr++;
      } else if (otherFact == -1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) 
          *dataPtr++ -= *otherDataPtr++;
      } else 
        for (int i=0; i<N; i++) 
          *dataPtr++ += *otherDataPtr++ * otherFact;

    } else if (thisFact == 0.0) {
        // want: this = other * otherFact
        double *dataPtr = values;
        double *otherDataPtr = other.values;
        if (otherFact == 1.0) { // no point doing a multiplication if otherFact == 1.0
          for (int i=0; i<N; i++) 
            *dataPtr++ = *otherDataPtr++;
        } else if (otherFact == -1.0) { // no point doing a multiplication if otherFact == 1.0
          for (int i=0; i<N; i++) 
            *dataPtr++ = -(*otherDataPtr++);
        } else 
          for (int i=0; i<N; i++) 
            *dataPtr++ = *otherDataPtr++ * otherFact;
    } else {
      // want: this = this * thisFact + other * otherFact
      double *dataPtr = values;
      double *otherDataPtr = other.values;
      if (otherFact == 1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) {
          double value = *dataPtr * thisFact + *otherDataPtr++;
          *dataPtr++ = value;
        }
      } else if (otherFact == -1.0) { // no point doing a multiplication if otherFact == 1.0
        for (int i=0; i<N; i++) {
          double value = *dataPtr * thisFact - *otherDataPtr++;
          *dataPtr++ = value;
        }
      } else 
        for (int i=0; i<N; i++) {
          double value = *dataPtr * thisFact + *otherDataPtr++ * otherFact;
          *dataPtr++ = value;
      }
    }

    // successfull
    return 0;
  }


  template <int NC>
  int
  addMatrixVector(double thisFact, const MatrixND<N, NC, double> &m, const Vector &v, double otherFact)
  {
    // check the sizes are compatable
    assert(NC == v.sz);

    // see if quick return
    if (thisFact == 1.0 && otherFact == 0.0)
      return 0;

    else {
      int incr = 1,
             i = N,
             n = NC;
       DGEMV("N", &i, &n,
             &otherFact,
             &m.values[0][0], &i,
             v.theData, &incr,
             &thisFact,
             values,   &incr);
      // successfull
      return 0;
    } 
  }

  template <int NR>
  int
  addMatrixTransposeVector(double thisFact, const MatrixND<NR, N, double> &m, const Vector &v, double otherFact)
  {
    // check the sizes are compatable
    assert(NR == v.sz);

    // see if quick return
    if (thisFact == 1.0 && otherFact == 0.0)
      return 0;

    else {
      int incr = 1,
             i = N,
             n = NR;
      DGEMV("T", &i, &n,
            &otherFact,
            &m.values[0][0], &i,
            v.theData, &incr,
            &thisFact,
            values,   &incr);
      // successfull
      return 0;
    } 
  }



  int
  addMatrixVector(double thisFact, const Matrix &m, const Vector &v, double otherFact)
  {
    // check the sizes are compatable
    assert(N == m.noRows());
    assert(m.noCols() == v.sz);

    // see if quick return
    if (thisFact == 1.0 && otherFact == 0.0)
      return 0;

#ifdef VECTOR_BLAS
    else if (v.sz > 10) {
      int incr = 1,
             i = m.numRows,
             n = m.numCols;
      return
        DGEMV("N", &i, &n,
              &otherFact,
              m.data, &i,
              v.theData, &incr,
              &thisFact,
              values,   &incr);
    }
#endif

    else if (thisFact == 1.0) {

      // want: this += m * v * otherFact
      if (otherFact == 1.0) { // no point doing multiplication if otherFact = 1.0
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++;
          for (int j=0; j < N; j++)
            values[j] += *matrixDataPtr++ * otherData;
        }
      } 
      else if (otherFact == -1.0) { // no point doing multiplication if otherFact = -1.0
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++;
          for (int j=0; j < N; j++)
            values[j] -= *matrixDataPtr++ * otherData;
        }
      } 
      else { // have to do the multiplication
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++ * otherFact;
          for (int j=0; j < N; j++)
            values[j] += *matrixDataPtr++ * otherData;
        }
      }
    }

    else if (thisFact == 0.0) {
      
      // want: this = m * v * otherFact
      for (int i=0; i < N; i++)
        values[i] = 0.0;

      if (otherFact == 1.0) { // no point doing multiplication if otherFact = 1.0
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++;
          for (int j=0; j < N; j++)
            values[j] += *matrixDataPtr++ * otherData;
        }
      } 
      else if (otherFact == -1.0) { // no point doing multiplication if otherFact = -1.0
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++;
          for (int j=0; j < N; j++)
            values[j] -= *matrixDataPtr++ * otherData;
        }
      } else {
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++ * otherFact;
          for (int j=0; j < N; j++)
            values[j] += *matrixDataPtr++ * otherData;
        }
      }
    }

    else {

      // want: this = this * thisFact + m * v * otherFact
      for (int i=0; i<N; i++)
        values[i] *= thisFact;

      if (otherFact == 1.0) { // no point doing multiplication if otherFact = 1.0
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++;
          for (int j=0; j < N; j++)
            values[j] += *matrixDataPtr++ * otherData;
        }
      } else if (otherFact == -1.0) { // no point doing multiplication if otherFact = 1.0
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++;
          for (int j=0; j < N; j++)
            values[j] -= *matrixDataPtr++ * otherData;
        }
      } else {
        int otherSize = v.sz;
        double *matrixDataPtr = m.data;
        double *otherDataPtr = v.theData;
        for (int i=0; i<otherSize; i++) {
          double otherData = *otherDataPtr++ * otherFact;
          for (int j=0; j < N; j++)
            values[j] += *matrixDataPtr++ * otherData;
        }
      }
    }
    
    // successfull
    return 0;
  }

  template<typename VecT>
  constexpr T
  dot(const VecT &other) const {
    T sum = 0.0;
    for (index_t i = 0; i < N; ++i) {
      sum += values[i] * other[i];
    }
    return sum;
  }

  constexpr T
  norm() const {
    return sqrt(this->dot(*this));
  }

  template<typename VecT>
  VectorND<N> &operator=(const VecT &right) {
    for (int i=0; i< N; i++)
      values[i] = right[i];
    return *this;
  }

  VectorND<N> &operator/=(const double &right) {
    for (int i=0; i< N; i++)
      values[i] /= right;
    return *this;
  }

  VectorND<N> &operator*=(const double &right) {
    for (int i=0; i< N; i++)
      values[i] *= right;
    return *this;
  }

  VectorND<N> &operator+=(const VectorND<N> &right) {
    for (int i=0; i< N; i++)
      values[i] += right[i];
    return *this;
  }

  VectorND<N> &operator+=(const Vector &right) {
    for (int i=0; i< N; i++)
      values[i] += right[i];
    return *this;
  }

  VectorND<N> &operator-=(const VectorND<N> &right) {
    for (int i=0; i< N; i++)
      values[i] -= right[i];
    return *this;
  }

  VectorND<N> &operator-=(const Vector &right) {
    for (int i=0; i< N; i++)
      values[i] -= right[i];
    return *this;
  }


  friend std::ostream &
  operator<<(std::ostream &out, VectorND const &vec) {
    out << "{";
    for (int r=0; r<N; r++){
        out << vec[r] << ( r < N-1? ", ": "");
    }
    return out << "}\n";
  }
};
}
#endif
