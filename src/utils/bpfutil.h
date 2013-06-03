#ifndef BPFUTIL_H
#define BPFUTIL_H

#include "types.h"

namespace essentia {
  namespace util {
    class BPF {
      protected:
        std::vector<Real> _xPoints;
        std::vector<Real> _yPoints;
        std::vector<Real> _slopes;

      public:
        BPF() {
        }
        BPF(std::vector<Real> xPoints, std::vector<Real> yPoints) {
          init(xPoints,yPoints);
        }
        void init(std::vector<Real> xPoints, std::vector<Real> yPoints) {
          _xPoints = xPoints;
          _yPoints = yPoints;
          if (_xPoints.size() != _yPoints.size()) {
            throw EssentiaException("BPF: xPoints and yPoints do not have the same size");
          }

          if (_xPoints.size() < 2) {
            throw EssentiaException("BPF: There are less than 2 points, which is the minimum required for the break-point function");
          }

          for (int i=1; i<int(_xPoints.size()); ++i) {
            if (_xPoints[i-1] >= _xPoints[i] ) {
              throw EssentiaException("BPF: xPoints are not sorted by increasing values");
            }
          }

          _slopes.resize(_xPoints.size()-1);

          for (int j=1; j<int(_xPoints.size()); ++j) {
            // this never gives a division by zero as we checked just before that x[i-1] < x[i]
            _slopes[j-1] = (_yPoints[j] - _yPoints[j-1]) / (_xPoints[j] - _xPoints[j-1]);
          }
        }

        inline float operator()(float x) {
          if (x < _xPoints[0]) {
            throw EssentiaException("BPF: Input x-value is before the first point");
          }

          if (x > _xPoints.back()) {
            throw EssentiaException("BPF: Input x-value is past the last point");
          }

          std::vector<Real>::size_type j = 0;
          while (x > _xPoints[j+1]) {
            j += 1;
          }

          return (x - _xPoints[j]) * _slopes[j] + _yPoints[j];
        }
    };
  }
}

#endif // BPFUTIL_H
