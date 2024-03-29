#include "LoadData.h"


double T = 10;
double R = 10;

double Dsize = 1000;
double Tsize = 1300;

//STGrid Parameters

double spatialTr = 20.0; //for diameter
double temporalTr = 20.0; //for diameter

double threshold;

//########################################

double theta = 1;

int size;
STPoint* data;
fstream uchwyt;

int numOfEventTypes; // not used

vector<vector <STPoint>> dataset;
vector<vector <STPoint>> sortedDataset;

vector<vector <MCEntry>> MCindex;
vector<vector <MCEntry>> sortedMCindex;

vector<Sequence> SequencesSet;
vector<MCSequence> MCSequencesSet;

vector<vector<vector<Cell>>> Grid;

int GseqID = 0;
int GMCseqID = 0;
int GclusterID = 0;


bool comparison(const STPoint &i1, const STPoint &i2)
{
	return i1.temporal < i2.temporal;
}

bool comparisonID(const STPoint &i1, const STPoint &i2)
{
	return i1.eventID < i2.eventID;
}

bool isEqual(const STPoint &i1, const STPoint &i2)
{
	return (i1.eventID == i2.eventID);
}

bool comparison_MC(const MCEntry &i1, const MCEntry &i2)
{
	return i1.temporalCenter < i2.temporalCenter;
}

bool comparisonID_MC(const MCEntry &i1, const MCEntry &i2)
{
	return i1.clusterID < i2.clusterID;
}

bool isEqual_MC(const MCEntry &i1, const MCEntry &i2)
{
	return (i1.clusterID == i2.clusterID);
}

void SortDataset()
{
	sortedDataset = dataset;
	for(int i = 0; i < dataset.size(); i++)
	{
		sort(sortedDataset[i].begin(), sortedDataset[i].end(), comparison);
	}
}

void SortIndex()
{
	sortedMCindex = MCindex;
	for(int i = 0; i < MCindex.size(); i++)
	{
		sort(sortedMCindex[i].begin(), sortedMCindex[i].end(), comparison_MC);
	}
}

vector<STPoint> ForwardSweep(vector<STPoint> tailEventSet, vector<STPoint> instancesSet)
{
	sort(tailEventSet.begin(), tailEventSet.end(), comparison);
	//sort(instancesSet.begin(), instancesSet.end(), comparison);

	vector<STPoint> joinResult;

	while(tailEventSet.empty() != true && instancesSet.empty() != true)
	{
		int pindex = 0, qindex = 0;

		STPoint p = tailEventSet[pindex];
		STPoint q = instancesSet[qindex];

		if(p.temporal < q.temporal)
		{
			tailEventSet.erase(tailEventSet.begin());
			while((p.temporal + T) > q.temporal)
			{
				if((p.spatialX - R) < q.spatialX && (p.spatialX + R) > q.spatialX && (p.spatialY - R) < q.spatialY && (p.spatialY + R) > q.spatialY)
				{
					joinResult.push_back(q);
				}
				if(qindex < instancesSet.size()){
					qindex++;
					q = instancesSet[qindex];
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			instancesSet.erase(instancesSet.begin());
		}
	}

	return joinResult;
}

void Miner()
{
	for(int i = 0; i < sortedDataset.size(); i++) //create 1-length sequences
	{
		Sequence seq;

		//seq.seqID++;
		seq.sequence.push_back(sortedDataset[i][0].eventType);
		seq.tailEventSet = sortedDataset[i];
		//SequencesSet.push_back(seq);

		cout << "Processed: " << i << endl;
		ExpandSequence(seq);
	}
}

bool checkPresence(string eventType, vector<string> sequence)
{
	for(int i = 0; i < sequence.size(); i++)
	{
		if(eventType == sequence[i]){
			return true;
		}
	}

	return false;
}

void ExpandSequence(Sequence seq)
{
	for(int i = 0; i < sortedDataset.size(); i++)
	{
		if(checkPresence(sortedDataset[i][0].eventType, seq.sequence) != true)
		{
		vector<STPoint> joinSet = ForwardSweep(seq.tailEventSet, sortedDataset[i]);
		double DR = CalculateDR(seq.tailEventSet, joinSet, sortedDataset[i]);

		//cout << "DR " << DR << endl;

		if(DR > theta)
		{

			for(int k = 0; k < seq.sequence.size(); k++)
			{
//				cout << seq.sequence[k];
			}
//			cout << joinSet[0].eventType << endl;
			//cout << seq.tailEventSet[0].eventType << " " << seq.tailEventSet.size() << endl;
			//cout << joinSet[0].eventType << " " << joinSet.size() << endl;
//			cout << "  DR " << DR << endl;
			//cout << "Pring";

			GseqID++;
			Sequence seqNew;

			seqNew.seqID = GseqID;

			//seq.seqID = GseqID;
			//seq.sequence.push_back(sortedDataset[i][0].eventType);

			seqNew.sequence = seq.sequence;
			seqNew.sequence.push_back(sortedDataset[i][0].eventType);

			//cout << "1: " << joinSet.size() << endl;

			sort(joinSet.begin(), joinSet.end(), comparisonID);
			joinSet.erase(unique(joinSet.begin(), joinSet.end(), isEqual), joinSet.end());

			//cout << "2: " << joinSet.size() << endl;

			seqNew.tailEventSet = joinSet;
			seqNew.seqIndex = min(DR, seq.seqIndex);
			SequencesSet.push_back(seqNew);

			//cout << seqNew.seqID << ' ' << seqNew.sequence.size() << endl;

			ExpandSequence(seqNew);
		}
		}
	}
}


double CalculateDR(vector<STPoint> tailSet, vector<STPoint> joinSet, vector<STPoint> eventTypeSet)
{
	//cout << "Tail event set " << tailSet.size() << endl;
	//cout << "Join Set " << joinSet.size() << endl;
	//cout << "Event Set " << eventTypeSet.size() << endl;

	double avgDens = ((double)joinSet.size()/(double)(2*R*2*R*T))/(double)tailSet.size();

	//cout << "Avg dens: " << avgDens << endl;

	double Dens = ((double)eventTypeSet.size()/(double)(Dsize*Dsize*Tsize));

	//cout << "Dens: " << Dens << endl;

	//cout << "DR: " << (avgDens/Dens) << endl;

	return (avgDens/Dens);
}

//##################################################################

//################################################################

//Alternatywny spos�b mikrogrupowania

int FindMinimalCluster(vector<MCEntry> clusters, STPoint instance)
{
	double minDist = sqrt(pow(clusters[0].spatialXCenter - instance.spatialX, 2.0) + pow(clusters[0].spatialYCenter - instance.spatialY, 2.0)
			+ pow(clusters[0].temporalCenter - instance.temporal, 2.0));
	int minIndex = 0;

	for(int i = 1; i < clusters.size(); i++)
	{
		double dist = sqrt(pow(clusters[i].spatialXCenter - instance.spatialX, 2.0) + pow(clusters[i].spatialYCenter - instance.spatialY, 2.0)
				+ pow(clusters[i].temporalCenter - instance.temporal, 2.0));
		if(dist < minDist)
		{
			minDist = dist;
			minIndex = i;
		}
	}

	return minIndex;
}

MCEntry AddToCluster(MCEntry c, STPoint instance)
{
	c.size++;
	c.containedInstances.push_back(instance);

	double sumX = 0, sumY = 0, sumT = 0;

	for(int i = 0; i < c.containedInstances.size(); i++)
	{
		sumX += c.containedInstances[i].spatialX;
		sumY += c.containedInstances[i].spatialY;
		sumT += c.containedInstances[i].temporal;
	}

	c.spatialXCenter = sumX/c.containedInstances.size();
	c.spatialYCenter = sumY/c.containedInstances.size();
	c.temporalCenter = sumT/c.containedInstances.size();

	return c;
}

bool CheckCluster(MCEntry c)
{
	//compute spatial and temporal diameters
	double spatialD = 0, temporalD = 0;
	double spatialSum = 0, temporalSum = 0;

	double actualThreshold = 0, sumThreshold = 0;

	for(int i = 0; i < c.containedInstances.size(); i++)
	{
		for(int j = 0; j < c.containedInstances.size(); j++)
		{
			//spatialSum += pow(c.containedInstances[i].spatialX-c.containedInstances[j].spatialX, 2.0) + pow(c.containedInstances[i].spatialY - c.containedInstances[j].spatialY, 2.0);
			//temporalSum += pow(c.containedInstances[i].temporal - c.containedInstances[j].temporal, 2.0);

			sumThreshold += pow(c.containedInstances[i].spatialX-c.containedInstances[j].spatialX, 2.0)
					+ pow(c.containedInstances[i].spatialY - c.containedInstances[j].spatialY, 2.0)
					+ pow(c.containedInstances[i].temporal - c.containedInstances[j].temporal, 2.0);
		}
	}

	//spatialD = sqrt(spatialSum/(c.containedInstances.size()*(c.containedInstances.size() - 1)));
	//temporalD = sqrt(temporalSum/(c.containedInstances.size()*(c.containedInstances.size() - 1)));

	actualThreshold = sqrt(sumThreshold/(c.containedInstances.size()*(c.containedInstances.size() - 1)));

	if(actualThreshold > threshold)
	{
		return true;
	}
	else
	{
		return false;
	}
}

vector<MCEntry> SplitCluster(vector<MCEntry> clusters, int index)
{
	int index1, index2;
	double maxDist = -1.0;

	MCEntry c = clusters[index];

	for(int i = 0; i < c.containedInstances.size(); i++)
	{
		for(int j = 0; j < c.containedInstances.size(); j++)
		{
			double dist = sqrt(pow(c.containedInstances[i].spatialX - c.containedInstances[j].spatialX, 2.0) + pow(c.containedInstances[i].spatialY - c.containedInstances[j].spatialY, 2.0)
					+ pow(c.containedInstances[i].temporal - c.containedInstances[j].temporal, 2.0));

			if(dist > maxDist)
			{
				index1 = i;
				index2 = j;
				maxDist = dist;
			}
		}
	}
	//cout << "CID " << c.clusterID << endl;
	//cout << "I1 " << index1 << endl;
	//cout << "I2 " << index2 << endl;

	vector<STPoint> instances;
	STPoint c2 = clusters[index].containedInstances[index2];
	STPoint c1 = clusters[index].containedInstances[index1];
	instances.push_back(c2);

	//cout << "BF1 " << clusters[index].containedInstances.size() << endl;

	clusters[index].containedInstances.erase(clusters[index].containedInstances.begin() + index2);
	clusters[index].size--;



	for(int i = 0; i < clusters[index].containedInstances.size(); i++)
	{
		//if(i != index1)
		{
			STPoint cx = clusters[index].containedInstances[i];

			double dist1 = sqrt(pow(c1.spatialX - cx.spatialX, 2.0) + pow(c1.spatialY - cx.spatialY, 2.0)
					+ pow(c1.temporal - cx.temporal, 2.0));
			double dist2 = sqrt(pow(c2.spatialX - cx.spatialX, 2.0) + pow(c2.spatialY - cx.spatialY, 2.0)
					+ pow(c2.temporal - cx.temporal, 2.0));

			if(dist2 < dist1)
			{
				clusters[index].containedInstances.erase(clusters[index].containedInstances.begin() + i);
				instances.push_back(cx);
				clusters[index].size--;
			}
		}
	}

	//cout << "AF1 " << clusters[index].containedInstances.size() << endl;
	//cout << "AF2 " << instances.size() << endl;

	MCEntry entry1;
	entry1.clusterID = GclusterID;
	GclusterID++;
	entry1.eventType = instances[0].eventType;
	entry1.containedInstances = instances;
	entry1.size = instances.size();

	double sumX = 0, sumY = 0, sumT = 0;

	for(int i = 0; i < instances.size(); i++)
	{
		sumX += entry1.containedInstances[i].spatialX;
		sumY += entry1.containedInstances[i].spatialY;
		sumT += entry1.containedInstances[i].temporal;
	}

	entry1.spatialXCenter = sumX/instances.size();
	entry1.spatialYCenter = sumY/instances.size();
	entry1.temporalCenter = sumT/instances.size();

	clusters.push_back(entry1);

	sumX = 0, sumY = 0, sumT = 0;

	for(int i = 0; i < clusters[index].containedInstances.size(); i++)
	{
		sumX += clusters[index].containedInstances[i].spatialX;
		sumY += clusters[index].containedInstances[i].spatialY;
		sumT += clusters[index].containedInstances[i].temporal;
	}

	clusters[index].spatialXCenter = sumX/clusters[index].containedInstances.size();
	clusters[index].spatialYCenter = sumY/clusters[index].containedInstances.size();
	clusters[index].temporalCenter = sumT/clusters[index].containedInstances.size();

	return clusters;

}

void ClusterData(vector<STPoint> instances)
{
	vector<MCEntry> clusters;
	MCEntry entry;
	entry.clusterID = GclusterID;
	GclusterID++;
	entry.eventType = instances[0].eventType;
	entry.size = 1;
	entry.containedInstances.push_back(instances[0]);
	entry.spatialXCenter = instances[0].spatialX;
	entry.spatialYCenter = instances[0].spatialY;
	entry.temporalCenter = instances[0].temporal;
	clusters.push_back(entry);

	double avgFindMinimalClusterTime = 0.0;
	double avgAddToClusterTime = 0.0;
	double avgCheckClusterTime = 0.0;
	double avgSplitClusterTime = 0.0;

	for(int i = 1; i < instances.size(); i++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		int index = FindMinimalCluster(clusters, instances[i]);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>( t2 - t1 ).count();
		avgFindMinimalClusterTime += duration;

		t1 = high_resolution_clock::now();
		clusters[index] = AddToCluster(clusters[index], instances[i]);
		t2 = high_resolution_clock::now();
		duration = duration_cast<milliseconds>( t2 - t1 ).count();
		avgAddToClusterTime += duration;

		//cout << (clusters[index].containedInstances.size()) << endl;

		t1 = high_resolution_clock::now();
		bool split = CheckCluster(clusters[index]);
		t2 = high_resolution_clock::now();
		duration = duration_cast<milliseconds>( t2 - t1 ).count();
		avgCheckClusterTime += duration;


		if(split == true)
		{
			t1 = high_resolution_clock::now();
			clusters = SplitCluster(clusters, index);
			t2 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>( t2 - t1 ).count();
			avgSplitClusterTime += duration;
		}
	}

	avgFindMinimalClusterTime /= instances.size();
	avgAddToClusterTime /= instances.size();
	avgCheckClusterTime /= instances.size();
	avgSplitClusterTime /= instances.size();

	//cout << avgFindMinimalClusterTime << "\t" << avgAddToClusterTime << "\t" << avgCheckClusterTime << "\t" << avgSplitClusterTime << endl;
	MCindex.push_back(clusters);

}

void PerformMicroclustering()
{
	for(int i = 0; i < sortedDataset.size(); i++)
	{
		ClusterData(sortedDataset[i]);
	}
}

//################################################################

void SortMicrocluster()
{
	for(int i = 0; i < MCindex.size(); i++)
	{
		sort(MCindex[i].begin(), MCindex[i].end(), comparison_MC);
	}
}


//################################################################

void PrintMCIndex()
{
	for(int i = 0; i < MCindex.size(); i++)
	{
		for(int j = 0; j < MCindex[i].size(); j++)
		{
			cout << MCindex[i][j].clusterID << ' ' << MCindex[i][j].eventType << ' ' << MCindex[i][j].size << ' ' << MCindex[i][j].temporalCenter << '|';
		}
		cout << endl;
	}
}

//################################################################

void PrintSequencesMC()
{
	for(int i = 0; i < MCSequencesSet.size(); i++)
		{
			for(int j = 0; j < MCSequencesSet[i].sequence.size(); j++)
			{
				cout << MCSequencesSet[i].sequence[j] << ' ';
			}
			cout << MCSequencesSet[i].seqIndex << endl;
		}
}


//################################################################

vector<vector<MCEntry>> ForwardSweep(vector<MCEntry> tailEventSet, vector<MCEntry> indexClusters)
{
	sort(tailEventSet.begin(), tailEventSet.end(), comparison_MC);
	vector<vector<MCEntry>> joinResult;

	while(tailEventSet.empty() != true)
	{
		if(indexClusters.empty() == true)
		{
			while(tailEventSet.empty() != true)
			{
				tailEventSet.erase(tailEventSet.begin());
				vector<MCEntry> neighborhood;
				joinResult.push_back(neighborhood);
			}
		}
		else
		{

		int pindex = 0, qindex = 0;

		MCEntry p = tailEventSet[pindex];
		MCEntry q = indexClusters[qindex];

		if(p.temporalCenter < q.temporalCenter)
		{
			tailEventSet.erase(tailEventSet.begin());
			vector<MCEntry> neighborhood;

			//cout << p.spatialXCenter << " " << q.spatialXCenter << endl;

			while(p.temporalCenter + T > q.temporalCenter)
			{
				if(p.spatialXCenter - R < q.spatialXCenter && p.spatialXCenter + R > q.spatialXCenter && p.spatialYCenter - R < q.spatialYCenter && p.spatialYCenter + R > q.spatialYCenter)
				{
					neighborhood.push_back(q);
				}

				if(qindex < indexClusters.size() - 1)
				{
					qindex++;
					q = indexClusters[qindex];
				}
				else
				{
					break;
				}
			}
			joinResult.push_back(neighborhood);
		}
		else
		{
			indexClusters.erase(indexClusters.begin());
		}
	}
	}

	return joinResult;
}

bool checkPresenceMC(string eventType, vector<string> sequence)
{
	for(int i = 0; i < sequence.size(); i++)
	{
		if(eventType == sequence[i]){
			return true;
		}
	}

	return false;
}

void ExpandSequence(MCSequence seq)
{
	for(int i = 0; i < MCindex.size(); i++)
	{
		if(checkPresenceMC(MCindex[i][0].eventType, seq.sequence) != true)
		{
		//cout << "SeqID: " << seq.tailEventSet[0].eventType << endl;
		//cout << "tailEventSet size: " << seq.tailEventSet.size() << endl;
		//cout << "Event type " << MCindex[i].size() << endl;
		vector<vector<MCEntry>> joinSet = ForwardSweep(seq.tailEventSet, MCindex[i]);


		//cout << "joinSet size: " << joinSet.size() << endl;

		//if(joinSet.size() != 0){
		double DR = CalculateDR(seq.tailEventSet, joinSet, MCindex[i]);

		//cout << "DR " << DR << endl;
		//cout << seq.tailEventSet[0].eventType << endl;


		if(DR >= theta)
		{
			//cout << "DR " << DR << endl;
			GMCseqID++;
			seq.seqID = GMCseqID;
			seq.sequence.push_back(MCindex[i][0].eventType);

			//cout << "1: " << joinSet.size() << endl;

			vector<MCEntry> vec;
			for(int i = 0; i < joinSet.size(); i++)
			{
				if(joinSet[i].empty() != true){
					for(int j = 0; j < joinSet[i].size(); j++)
					{
						vec.push_back(joinSet[i][j]);
					}
				}
			}

			sort(vec.begin(), vec.end(), comparisonID_MC);
			vec.erase(unique(vec.begin(), vec.end(), isEqual_MC), vec.end());

			//cout << "2: " << vec.size() << endl;

			seq.tailEventSet = vec;
			seq.seqIndex = min(DR, seq.seqIndex);
			MCSequencesSet.push_back(seq);
			ExpandSequence(seq);

		}
		}
		//}
	}
}

double CalculateDR(vector<MCEntry> tailEventSet, vector<vector<MCEntry>> joinSet, vector<MCEntry> eventTypeSet)
{
	double avgDens;
	double Dens;

	double sum = 0.0;
	for(int i = 0; i < tailEventSet.size(); i++) //for each neighborhood
	{
		double param = 0.0;
		//cout << joinSet[i].size() << endl;
		for(int j = 0; j < joinSet[i].size(); j++) //compute number of total indexed instances
		{
			param += joinSet[i][j].containedInstances.size();
		}
		sum += (double)tailEventSet[i].containedInstances.size()*(param/(2*R*2*R*T)); //compute density
	}

	double weig = 0.0;
	for(int i = 0; i < tailEventSet.size(); i++)
	{
		weig += tailEventSet[i].containedInstances.size();
	}

	avgDens = sum/(weig);

	int sumInt = 0;
	for(int i = 0; i < eventTypeSet.size(); i++)
	{
		sumInt += eventTypeSet[i].containedInstances.size();
	}

	Dens = double(sumInt)/(Dsize*Dsize*Tsize);

	return (avgDens/Dens);
}

void MinerMC()
{
	for(int i = 0; i < MCindex.size(); i++)
	{
		MCSequence seq;

		GMCseqID++;
		seq.seqID = GMCseqID;
		seq.sequence.push_back(MCindex[i][0].eventType);
		seq.tailEventSet = MCindex[i];

		cout << "Process " << i << endl;
		ExpandSequence(seq);
	}
}

//##################################################################

void LoadDataset(string Path) //za�aduj zbi�r danych
{
	size = CountInstances(Path); // zlicz l. instancji w pliku
	data = new STPoint[size]; // rozszerz data

	uchwyt.open(Path);
	for(int i = 0; i < size; i++)
	{
		string line;
		getline(uchwyt, line);
		stringstream linestream(line);
		string dataPortion;

		if(line != ""){
			getline(linestream, dataPortion, ' ');
			data[i].eventID = stoi(dataPortion);
			getline(linestream, dataPortion, ' ');
			data[i].eventType = dataPortion;
			getline(linestream, dataPortion, ' ');
			data[i].spatialX = stod(dataPortion);
			getline(linestream, dataPortion, ' ');
			data[i].spatialY = stod(dataPortion);
			getline(linestream, dataPortion, ' ');
			data[i].temporal = stod(dataPortion);
		}

		//cout << i << endl;
	}
	uchwyt.close();
}

void TransformData()
{
	for(int i = 0; i < size; i++)
	{
		InsertInstance(data[i]);
	}
}

void InsertInstance(STPoint instance)
{
	for(int i = 0; i < dataset.size(); i++)
	{
		if(dataset[i][0].eventType == instance.eventType)
		{
			dataset[i].push_back(instance);
			return;
		}
	}

	vector<STPoint> vect;
	vect.push_back(instance);
	dataset.push_back(vect);
}

void PrintDataset()
{

	for(int i = 0; i < dataset.size(); i++)
	{
		for(int j = 0; j < dataset[i].size(); j++)
		{
			cout << dataset[i][j].eventID << ' ' << dataset[i][j].eventType << '\t';
			//cout << i;
		}

		cout << endl;
	}
}

void PrintSortedDataset()
{

	for(int i = 0; i < dataset.size(); i++)
	{
		//for(int j = 0; j < dataset[i].size(); j++)
		{
			cout << sortedDataset[i][0].eventType << '\t';
			//cout << i;
		}

		cout << endl;
	}
}

void PrintSequences()
{
	for(int i = 0; i < SequencesSet.size(); i++)
	{
		for(int j = 0; j < SequencesSet[i].sequence.size(); j++)
		{
			cout << SequencesSet[i].sequence[j] << ' ';
		}
		cout << SequencesSet[i].seqIndex << endl;
	}
}

void ClearSequencesSet()
{
	SequencesSet.clear();
}

void ClearDataset()
{
	dataset.clear();
}

void ClearSortedDataset()
{
	sortedDataset.clear();
}

void ClearStructures()
{
	dataset.clear();
	sortedDataset.clear();
	SequencesSet.clear();
	MCSequencesSet.clear();
	MCindex.clear();
	sortedMCindex.clear();
}

int CountInstances(string path) // zlicz liczb� instancji w pliku
{
	uchwyt.open(path);
	string line;

	int numInstances = 0;

	while(uchwyt.eof() != true)
	{
		getline(uchwyt, line);

		if(line != "")
		{
			numInstances++;
		}
	}

	uchwyt.close();
	return numInstances;
}
