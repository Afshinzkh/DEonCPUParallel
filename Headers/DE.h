#pragma once

#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <random>
#include <chrono>
#include <omp.h>


namespace Calibration {

  class  DE{
    public:
      double runDE(double newR);
      DE(std::string m);
      const double& getAlpha() const;
      const double& getBeta() const;
      const double& getSigma() const;
      const double& getError() const;
      const int& getIter() const;
      const double& getTime() const;
      void setMrktArray(std::array<double, 9> const& mrktData);
      const std::array<double, 9>& getMdlArray() const;

    private:
      std::string methodName;
      double alpha, beta, sigma, avgError;
      int loopCount;
      double calTime;
      std::array<double, 9> crrntMonthMrktData;
      std::array<double, 9> crrntMonthMdlData;
  };

}
