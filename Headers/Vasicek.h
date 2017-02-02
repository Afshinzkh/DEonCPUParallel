#pragma once

#include "../Headers/Method.h"
#include <vector>
#include <string>
#include <array>
#include <random>
#include <limits>
#include <algorithm>
#include <iostream>
#include <omp.h>

namespace Calibration {


  class Vasicek : public Method{
    public:
      virtual void run();
      virtual void nextRate();
      virtual double getYield(double const& tau);

      Vasicek(double const& rZero, std::array<double, 9> const& T ) : Method(rZero, T) {}

  };

}
