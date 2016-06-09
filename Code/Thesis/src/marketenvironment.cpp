#include <thesis/marketenvironment.h>
#include <armadillo>  /* arma::mat */
#include <string>     /* std::string */ 
#include <fstream>    /* std::ifstream */
#include <sstream>    /* std::istringstream */

MarketEnvironment::MarketEnvironment (std::string inputFilePath, 
									  double riskFreeRate_,
									  size_t numDaysObserved_,
									  size_t startDate_, 
									  size_t endDate_)
	: riskFreeRate(riskFreeRate_), 
	  numDaysObserved(numDaysObserved_)
{
	// Initialize filestream from inputFilePath
	std::ifstream ifs(inputFilePath);
	std::string line;

	// pierpaolo - gio 09 giu 2016 09:57:32 CEST
	// TODO: Test file opening

	// Read number of days and number of risky assets from the first line
	if (getline(ifs, line))
	{
		std::istringstream linestream(line);
		linestream >> numDays >> numRiskyAssets;
	}

	// Read risky assets symbols from the second line 
	if (getline(ifs, line))
	{
		std::istringstream linestream(line);
		std::string symbol;
		for (size_t i = 0; i < numRiskyAssets; ++i) 
		{			 
			linestream >> symbol; 
			assetSymbols.push_back(symbol);
		}
	}

	// Read risky assets log-returns in an armadillo matrix. 
	// For faster slicing, the matrix is of size numRiskyAssets X numDays
	logReturns.set_size(numRiskyAssets, numDays); 
	double oneLogReturn = 0.0;
	for(size_t i = 0; i < numDays && getline(ifs, line); ++i)
	{
		std::istringstream linestream(line);
		for(size_t j = 0; j < numRiskyAssets; ++j)
		{
			linestream >> oneLogReturn;
			logReturns(j, i) = oneLogReturn;
		}
	}
	
	// Set dimensions of state and action spaces 
	dimState = (numDaysObserved + 1) * numRiskyAssets + 1;
	dimAction = numRiskyAssets + 1;

	// Set evaluation dates 
	startDate = (startDate_ > numDaysObserved) ? startDate_ : numDaysObserved;
	currentDate = startDate;
	endDate = (endDate_ < numDays) ? endDate_ : numDays;
}
		
arma::vec MarketEnvironment::getState() const
{
	// pierpaolo - gio 09 giu 2016 10:28:19 CEST
	// TODO: To avoid copy, pass state vector as input by reference? 
	
	arma::vec pastLogReturns(dimState);  
	pastLogReturns(0) = riskFreeRate;			
	pastLogReturns.rows(1, dimState-1) = arma::vectorise(
			logReturns.cols(currentDate - numDaysObserved, currentDate)); 	
	return pastLogReturns;
}
	
void MarketEnvironment::performAction(arma::vec const &action)
{
	// The system evolution is independent from the agent's allocation
	currentDate++;
}

void MarketEnvironment::SetEvaluationInterval(size_t startDate_, 
											  size_t endDate_)
{
	(*this).setStartDate(startDate_);
	(*this).setEndDate(endDate_);
	(*this).reset();
}

void MarketEnvironment::reset() 
{
	currentDate = (startDate > numDaysObserved) ? startDate : numDaysObserved;
}