/*
 * player.cpp
 *
 *  Created on: Nov 9, 2017
 *      Author: claudio
 */

#include <iostream>

#include "ExperienceReader.h"
#include "RobocupTest.h"
#include "RobocupProblem.h"
#include "ExperienceLine.h"


int main() {
	ExperienceReader<float> reader("Robots_experience");

	std::vector<std::vector<ExperienceLine<float>>> partition = reader.partitionByZ(1.f);
	std::vector<ExperienceLine<float>> goalExp = partition.front();
	std::vector<ExperienceLine<float>> noGoalExp = partition.back();
	std::vector<std::vector<ExperienceLine<float>>> positiveSets = reader.createTrainTestSet(goalExp, 0.9f);
	std::vector<std::vector<ExperienceLine<float>>> negativeSets = reader.createTrainTestSet(noGoalExp, 0.9f);

	//reader.create_svmfile(positiveSets.at(0), negativeSets.at(0), "train");
	//reader.create_svmfile(positiveSets.at(1), negativeSets.at(1), "test");

	RLLib::Random<float>* problemRandom = new RLLib::Random<float>();
		problemRandom->reseed(unsigned(time(0)));
	std::vector<ExperienceLine<float>> partialTrain(negativeSets.at(0).begin(), negativeSets.at(0).begin()+ (int)negativeSets.at(0).size()*0.09f);
	RLLib::RLProblem<float>* robocupProblem = new RobocupProblem<float>(problemRandom, positiveSets.at(0),
			partialTrain);
	std::vector<ExperienceLine<float>> partialTest(negativeSets.at(1).begin(), negativeSets.at(1).begin()+ (int)negativeSets.at(1).size()*0.09f);
	RobocupTest<float>* train = new RobocupTest<float>(robocupProblem, positiveSets.at(1),
			partialTest);
	train->testGoalPredictionGQLambda();

	//RLLib::RLProblem<float>* robocupProblem = new RobocupProblem<float>(problemRandom, positiveSets.at(1), negativeSets.at(1));
	return 0;
}

