//============================================================================
// Name        : aRticle.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <ctime>
#include <chrono>

#include <algorithm>    // std::sort
#include <vector>       // std::vector

#include "LoadData.h"
using namespace std;
using namespace std::chrono;


int main()
{
	//exp1 - microclustering

	fstream resultsMC; //for MC
	fstream resultsMCTime;
	int AverageNumberOfClusters = 0;
	int AverageDatasetSize = 0;

	resultsMC.open("Results//Exp1//Nf20//ExpMC.txt", fstream::out);
	resultsMCTime.open("Results//Exp1//Nf20//ExpMCTime.txt", fstream::out);

	int numberOfClusters = 0; //average number of microclusters
	double clusteringTime = 0.0; //average microclustering time
	double CompressionRate = 0.0;

	for(int Ni = 100; Ni <= 160; Ni += 10 ) //ps = 10, pn = 20
	{
		for(threshold = 40.0; threshold <= 100.0; threshold += 20.0)
		{
			for(int series = 1; series <= 10; series++)
			{
				cout << Ni << ' ' << threshold << ' ' << series << endl;
				string path = "Exp1//DataSe" + to_string(series) + "Ps5Pn10Ni" + to_string(Ni) + "Nf20.txt";

				LoadDataset(path); // wczytaj Dataset // za³aduj dataset
				TransformData(); //przekszta³æ dane
				SortDataset();
				//PrintSortedDataset();

				high_resolution_clock::time_point t1 = high_resolution_clock::now();
				PerformMicroclustering();
				high_resolution_clock::time_point t2 = high_resolution_clock::now();
				auto duration = duration_cast<milliseconds>( t2 - t1 ).count();

				//SortMicrocluster();

				for(int j = 0; j < MCindex.size(); j++){
					numberOfClusters += MCindex[j].size();
				}

				CompressionRate += (double)((double)size/(double)numberOfClusters);
				clusteringTime += duration;

				AverageNumberOfClusters += numberOfClusters;
				AverageDatasetSize += size;

				numberOfClusters = 0;
				size = 0;
				ClearStructures();
			}

			CompressionRate /= 10.0;
			AverageNumberOfClusters /= 10.0;
			AverageDatasetSize /= 10.0;
			clusteringTime /= 10.0;

			resultsMC <<  AverageNumberOfClusters << " (" << (float)((float)AverageDatasetSize/(float)AverageNumberOfClusters) << ")\t";
			resultsMCTime << clusteringTime << "\t";

			//cout <<  AverageDatasetSize << " " << AverageNumberOfClusters << " (" << (float)((float)AverageDatasetSize/(float)AverageNumberOfClusters) << ")\t" << endl;

			CompressionRate = 0.0;
			AverageNumberOfClusters = 0;
			AverageDatasetSize = 0;
			clusteringTime = 0.0;
		}

		resultsMC << endl;
		resultsMCTime << endl;
		cout << endl;
	}

	resultsMC.close();
	resultsMCTime.close();

	//Phase 2

	fstream resultsSTMinerPatternsNum;
	fstream resultsSTMinerExecTime;

	resultsSTMinerPatternsNum.open("Results//Exp1//Nf20//STMinerPatternsNum.txt", fstream::out);
	resultsSTMinerExecTime.open("Results//Exp1//Nf20//STMinerExecTime.txt", fstream::out);

	double execTime = 0.0;
	double patternsNum = 0;


	for(int Ni = 100; Ni <= 160; Ni += 10 )
	{
		for(int series = 1; series <= 10; series++)
		{
		cout << Ni << ' ' << ' ' << series << endl;
		string path = "Exp1//DataSe" + to_string(series) + "Ps5Pn10Ni" + to_string(Ni) + "Nf20.txt";

		LoadDataset(path); // wczytaj Dataset // za³aduj dataset
		TransformData();
		SortDataset();

		//PrintSortedDataset();

		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		Miner();
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>( t2 - t1 ).count();

		execTime += duration;
		patternsNum += SequencesSet.size();

		cout << duration << "\t" << SequencesSet.size() << "\t" << endl;

		ClearStructures();
		}

		resultsSTMinerExecTime << execTime/10.0 << endl;
		resultsSTMinerPatternsNum << patternsNum/10 << endl;

		execTime = 0.0;
		patternsNum = 0;

	}

	resultsSTMinerExecTime.close();
	resultsSTMinerPatternsNum.close();


	//Phase3

	fstream resultsMinerMCExecTime;
	fstream resultsMinerMCPatternsNum;

	resultsMinerMCExecTime.open("Results//Exp1//Nf20//MinerMCExecTime.txt", fstream::out);
	resultsMinerMCPatternsNum.open("Results//Exp1//Nf20//MinerMCPatternsNum.txt", fstream::out);

	 execTime = 0.0;
	 patternsNum = 0;

	for(int Ni = 100; Ni <= 160; Ni += 10 )
	{
		for(threshold = 40.0; threshold <= 100.0; threshold += 20.0)
		{
			for(int series = 1; series <= 10; series++)
			{
			cout << Ni << ' ' << series <<  ' ' << threshold << endl;
			string path = "Exp1//DataSe" + to_string(series) + "Ps5Pn10Ni" + to_string(Ni) + "Nf20.txt";

			LoadDataset(path);
			TransformData();
			SortDataset();

			//cout << "Phase 1 " << endl;
			PerformMicroclustering();
			SortMicrocluster();

			//cout << "Phase 2" << endl;
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			MinerMC();
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>( t2 - t1 ).count();

			execTime += duration;
			patternsNum += MCSequencesSet.size();

			ClearStructures();
			}

			resultsMinerMCExecTime << execTime/10.0 << "\t";
			resultsMinerMCPatternsNum << patternsNum/10 << "\t";

			execTime = 0.0;
			patternsNum = 0;

		}

		resultsMinerMCExecTime << endl;
		resultsMinerMCPatternsNum << endl;
	}

	resultsMinerMCExecTime.close();
	resultsMinerMCPatternsNum.close();

}
