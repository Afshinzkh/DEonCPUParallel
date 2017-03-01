#include "../Headers/Vasicek.h"
#include "../Headers/DE.h"
#include "../Headers/Helper.h"

using namespace Calibration;

// In the main we go through each time-serie and get the optimized model
// parameters as well as the final yield curve for each time-serie
int main(int argc, char* argv[])
{

  // Cheking the Arguments
  if( argc != 5){
    std::cout << "Error: Wrong number of Arguments" << std::endl;
    return -1;
  }

  std::string method = argv[2];
  std::string param = argv[3];
  std::string number = argv[4];
  std::cout << "Method to use: "<< method << std::endl;

  /****************************************************************************/
  /******************** STEP 1 : Initialize variables *************************/
  /****************************************************************************/
  const int maturityCount = 9;
  const int seriesCount = 12;

  // Define time to maturity
  std::array<double,maturityCount> tau = {0.25, 1, 3, 5, 7, 10, 15, 20, 30};

  // Define an array for Model Parameters e.g. alpha, beta, sigma
  std::array<double , seriesCount> alphaArray;
  std::array<double , seriesCount> betaArray;
  std::array<double , seriesCount> sigmaArray;
  std::array<double , seriesCount> errorArray;
  std::array<double , seriesCount> iterArray;
  std::array<double , seriesCount> timeArray;

  // Define the array that stores the final mdlData
  std::array< std::array< double, maturityCount>, seriesCount> mdlData;


  /****************************************************************************/
	/******************** STEP 2 : Read the Data ********************************/
	/****************************************************************************/
  // read the data from file e.g. for 12 month
  // it is a matrix named mrktData and has the size of
  // sereisCount * maturityCount with seriesCount as the number of month
  // and maturityCount as the number of maturities. e.g mrktData [12 * 9]
  // we also have an array tau [maturityCount] as the Time to maturity

    std::array<std::array<double,maturityCount>, seriesCount> mrktData;
    std::array<double,maturityCount> crrntMonthMrktData;

  // get the first argument of main as file name
  readData(argv[1], mrktData);

  double r0 = 0.0018;
  /****************************************************************************/
	/*************************** STEP 2 : Run DE ********************************/
	/****************************************************************************/
  // Here we call the DE functions to run over the data
  // main DE function which is called runDE implments the DE algorithm and
  // calculates the Error for each time-serie we have

    // define the Differential Evolution object
    std::string dataFileName = "../Data/" + method + param + number + ".csv";
    std::ofstream dataFile (dataFileName);
    dataFile << "Error;\n";

  int maxIter = 70; //Best Choice
  double F;// = 0.6;
  double CR;
  if(param == "F")  CR = 0.85;
  else              F = 0.6;

  int NP = 75;  //Best Choice
  // int j = 50;
  // for(int i = 5; i<=100; i+=5)
  // {
  //   if (i>100)
  //   {
  //     NP = 100 + j;
  //     j+=50;
  //   }
  //   else  NP = i;
  //   if (NP>500) break;

 double imax;
 if(param == "F")  imax = 1.25;
 else              imax = 1.05;
 for(double i = 0.05 ;i<imax; i+=0.05)
  {
    if(param == "F")  F = i;
    else              CR = i;
  // CR = i;
  DE d(method,NP,F,CR,maxIter);


  double newR = r0;

  // Get the total time
  auto start = std::chrono::steady_clock::now();

  // Call the Differential Evolution Function
  // for each time-serie
  for(int i = 0; i < seriesCount; i++)
  {
    if(param == "F")  std::cout << "values for F = "<< F << std::endl;
    else              std::cout << "values for CR = "<< CR << std::endl;
    // std::cout << "values for CR = "<< CR << std::endl;
    crrntMonthMrktData = mrktData[seriesCount-1-i];
    std::cout << "=============================" << std::endl;
    std::cout << "Running DE for series number:" << i+1 << std::endl;
    d.setMrktArray(crrntMonthMrktData);
    newR = d.runDE(newR);
    alphaArray[i] = d.getAlpha();
    betaArray[i] = d.getBeta();
    sigmaArray[i] = d.getSigma();
    errorArray[i] = d.getError();
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> durationCount = end - start;
  double totalTime = durationCount.count();

  /****************************************************************************/
	/*************************** STEP 4 : Print Out *****************************/
	/****************************************************************************/

  double avg = 0.0;
  for(int i = 0; i < seriesCount; i++)
  {
    avg+=errorArray[i];
  }

  std::cout << "average error is:" << avg/12.0 << '\n';
  if(param == "F")  dataFile << F << ";";
  else              dataFile << CR << ";";
  // dataFile << CR << ";"
  dataFile << avg/12 << ";";
  dataFile << "\n ";

}

  dataFile.close();

  return 0;
}
