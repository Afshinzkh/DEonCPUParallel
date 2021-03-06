#include "../Headers/Method.h"
#include "../Headers/Vasicek.h"
#include "../Headers/CIR.h"
#include "../Headers/DE.h"

namespace Calibration
{
  #define boundaryMIN 0.000001;

  double DE::runDE(double newR)
  {
 /****************************************************************************/
 /******************** STEP 1 : Initialize *********************************/
 /****************************************************************************/
  	// Select the DE Parameters as follows, NP  : Population Size >= 4
    //                                      F   : Scale Factor
  	//                                      CR  : Crossover Ratio [0,1]


    // const int NP = 100;
    // double F = 0.9;
    // double CR = 0.6;

    // Creat a population matrix P with the size of [NP * mpCount]
    // where mpCount is the count of Model Parameters;
    // for vasicek it is 3 for alpha, beta and sigma
    // for risklab it is 9 : alphaS, betaS, sigmaS, alphaY, betaY, sigmaY,
    // sigmaZ, bY, bZ
    int mpCount = 3;
    std::vector < std::vector <double> > P(NP,std::vector<double> (mpCount,0));

    // Define the Random Generator here
    // to get random values for model Parameters e.g. alpha, beta, sigma
    // Upper and Lower bounds for them are also defined here
    std::array<double, 3> upperBound = {1, 0.1 , 0.01};
    std::array<double, 3> lowerBound = {0.0, 0.00001, 0.00001};

    // Define the Random Device
    std::random_device rd;
	 	std::mt19937 gen(rd());
	 	std::uniform_real_distribution<long double> alphaRands(lowerBound[0],upperBound[0]);
    std::uniform_real_distribution<long double> betaRands(lowerBound[1],upperBound[1]);
    std::uniform_real_distribution<long double> sigmaRands(lowerBound[2],upperBound[2]);

    // start the calculation time here
    auto start = std::chrono::steady_clock::now();

    // Begin the OpenMP
    omp_set_num_threads(4);
    #pragma omp parallel for
      for(int i = 0; i < NP; i++)
      {
        P[i][0] = alphaRands(gen);
        P[i][1] = betaRands(gen);
        P[i][2] = sigmaRands(gen);
      }

    // Define Tolerance for Error
    double tol = 0.00000001;
    avgError = 1.0;
    double lastAvgError = 2.0;
    // int maxIter = 80;
    int iter = 0;
    loopCount = 0;


    std::array<double,9> tau = {0.25, 1, 3, 5, 7, 10, 15, 20, 30};

    // Generate the best possible r0
    int rLog = std::floor(std::log10(crrntMonthMrktData[0]));
    double reducer = std::pow(10,rLog);
    newR = crrntMonthMrktData[0] - reducer;

    std::cout << "R0 is: " << newR<< '\n';
/****************************************************************************/
/******************** STEP 2 : DE LOOP **************************************/
/****************************************************************************/
    double pError [NP];

    while ( iter < maxIter)
    {
        // Calculate the Vasicek/cir Error for each of these populations
        double sum = 0.0;

        #pragma omp parallel
        {
          auto v = new Vasicek(newR, tau);
          if (methodName == "cir")
              auto v = new CIR(newR, tau);

          #pragma omp for
            for(int i = 0; i < NP; i++)
            {

                v->setParameters(P[i][0], P[i][1], P[i][2]);
                v->setMrktArray(crrntMonthMrktData);
                v->nextRate();
                v->run();
                pError[i] = v->getError();
                sum += pError[i];
            }
          }

        // compute the average Error
        avgError = sum/NP;
        loopCount++;

        std::cout << "Average Error for Calculation loop :" << loopCount;
        std::cout << "\t is : " << avgError << std::endl;
        if(std::abs(avgError -lastAvgError) < tol)    break;

        lastAvgError = avgError;

  /****************************************************************************/
  /******************** STEP 3 : Mutation *************************************/
  /****************************************************************************/
        // Do the Mutation Stage as follows
        // define a mutated population as mutP
        std::vector < std::vector <double> > mutP(NP,std::vector<double> (mpCount,0));
        // Define the Random Generator here
        // to get random values for model Parameters e.g. alpha, beta, sigma
    	 	std::uniform_int_distribution<> indexRands(0,NP-1);
        int id1, id2, id3;
        #pragma omp parallel for private(id1, id2, id3)
          for (int i = 0; i < NP; i++)
          {
              // create 3 random indexes for each i the new mutated population set
              // with restriction  0<=id1,id2,id3<NP
              // and all the indexes should be different from each other and also from i
              id1 = indexRands(gen);
              id2 = indexRands(gen);
              id3 = indexRands(gen);
              while (id1 == i)  id1 = indexRands(gen);
              while (id2 == i || id2 == id1) id2 = indexRands(gen);
              while (id3 == i || id3 == id1 || id3 == id2) id3 = indexRands(gen);
              // Check the boundaries
              mutP[i][0] = P[id1][0] + F * (P[id2][0] - P[id3][0]);
              if(mutP[i][0] > upperBound[0])
                  mutP[i][0] = upperBound[0] - boundaryMIN;
              if(mutP[i][0] < lowerBound[0])
                  mutP[i][0] = lowerBound[0] + boundaryMIN;

              mutP[i][1] = P[id1][1] + F * (P[id2][1] - P[id3][1]);
              if(mutP[i][1] > upperBound[1])
                  mutP[i][1] = upperBound[1] - boundaryMIN;
              if(mutP[i][1] < lowerBound[1])
                  mutP[i][1] = lowerBound[1] + boundaryMIN;

              mutP[i][2] = P[id1][2] + F * (P[id2][2] - P[id3][2]);
              if(mutP[i][2] > upperBound[2])
                  mutP[i][2] = upperBound[2] - boundaryMIN;
              if(mutP[i][2] < lowerBound[2])
                  mutP[i][2] = lowerBound[2] + boundaryMIN;
          }
  /****************************************************************************/
  /******************** STEP 4 : Crossover ************************************/
  /****************************************************************************/
        // Crossover the mutated with the origital Poupation
        // define a Crossover Population as crP
        std::vector < std::vector <double> > crP(NP,std::vector<double> (mpCount,0));
        // Define the random Distribution U(0,1)
        std::uniform_real_distribution<> crRands(0,1);
        // Define random intex for model parameter count
        std::uniform_int_distribution<> mpRand(0,mpCount-1);
        // choose each model Parameter  as follows
        int i,j;
        #pragma omp parallel for private(i,j)
          for (i = 0; i < NP; i++)
          {
              for (j = 0; j < mpCount; j++)
              {
                  if(crRands(gen) < CR || mpRand(gen) == j)
                       crP[i][j] = mutP[i][j];
                  else
                       crP[i][j] = P[i][j];
              }
          }
        // Now that we have the Crossover Population we can calculate it's Errors
        double crError [NP];
        #pragma omp parallel
        {
          auto v = new Vasicek(newR, tau);
          if (methodName == "cir")
              auto v = new CIR(newR, tau);

          #pragma omp for
            for(int i = 0; i < NP; i++)
            {
                  v->setParameters(crP[i][0], crP[i][1], crP[i][2]);
                  v->setMrktArray(crrntMonthMrktData);
                  v->nextRate();
                  v->run();

                  crError[i] = v->getError();
            }
        }

        // Now you can compare the Error and if the error of one crossover population
        // is less than the error of one original population you copy that
        #pragma omp parallel for
          for(int i = 0; i < NP; i++)
          {
               if (crError[i] < pError [i])
              {
                 P[i][0] = crP[i][0];
                 P[i][1] = crP[i][1];
                 P[i][2] = crP[i][2];
              }
          }

        // So with this new P you can again check the error and go ahead and repeat
        iter++;

    }// end of while loop

    double finError = pError[0];
    int smallestIndex = 0;
    // #pragma omp parallel for private(smallestIndex, finError)
      for(int i = 1; i < NP; i++)
      {
        if(pError[i]<finError)
        {
          finError = pError[i];
          smallestIndex = i;
        }
      }
    std::cout << "Smallest Error is: " << finError <<'\n';
    std::cout << "Smallest index: " << smallestIndex <<'\n';
    alpha = P[smallestIndex][0];
    beta = P[smallestIndex][1];
    sigma = P[smallestIndex][2];

  /****************************************************************************/
  /***************** STEP 4 : Final Calculation *******************************/
  /****************************************************************************/
    //get the final values with final parameters
    auto v = new Vasicek(newR, tau);
    if (methodName == "cir")
        auto v = new CIR(newR, tau);
    v->setParameters(alpha, beta, sigma);
    v->nextRate();
    newR = v->getNewR();

    #pragma omp parallel for
      for (size_t i = 0; i < 9; i++) {
        crrntMonthMdlData[i] = v->getYield(tau[i]);
      }

    // end the Calculation time here
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> durationCount = end - start;
    calTime = durationCount.count();
  /****************************************************************************/
	/*************************** STEP 4 : Print Out *****************************/
	/****************************************************************************/

      std::cout << "\nfinal alpha:" <<  alpha <<std::endl;
      std::cout << "final beta:" << beta <<std::endl;
      std::cout << "final sigma:" << sigma <<std::endl;
      std::cout << "Average Error for loop :" << loopCount;
      std::cout << "\t is : " << avgError << std::endl;
      std::cout << "Calculation Time :" << calTime << std::endl;

      for (size_t j = 0; j < 9; j++) {
        std::cout << "y for maturity: "  << tau[j] << "\t is: \t" << crrntMonthMdlData[j] << std::endl;
      }

      return newR;
  }// DE::runDE

/****************************************************************************/
/******************** Setters and Getters are here **************************/
/****************************************************************************/

  DE::DE(std::string m, const int np, double f, double cr, int maxiter)
  {
    methodName = m;
    NP = np;
    F = f;
    CR = cr;
    maxIter = maxiter;
  }

  const double& DE::getAlpha() const { return alpha; }
  const double& DE::getBeta() const { return beta; }
  const double& DE::getSigma() const { return sigma; }
  const double& DE::getError() const { return avgError; }
  const int& DE::getIter() const { return loopCount; }
  const double& DE::getTime() const { return calTime; }

  const std::array<double, 9>& DE::getMdlArray() const
  {
    return crrntMonthMdlData;
  }

  void DE::setMrktArray(std::array<double, 9> const& mrktData)
  {
    crrntMonthMrktData = mrktData;
  }

}// namespace Calibration
