/*
 * RobocupTest.h
 *
 *  Created on: Nov 10, 2017
 *      Author: claudio
 */

#ifndef ROBOCUPTEST_H_
#define ROBOCUPTEST_H_
#pragma once

#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include "BinaryToTextFile.h"
#include "RobocupRunner.h"
#include "ExperienceLine.h"

#include "Learning/ControlAlgorithm.h"
#include "Learning/RL.h"
#include "Learning/Vector.h"
#include "Learning/Trace.h"
#include "Learning/Projector.h"
#include "Learning/StateToStateAction.h"

template<typename T>
class RobocupTest {
private:
	const int MEM_SIZE = (int) ::pow(10, 6);
	const unsigned HASH_SEED = 1057680130;
	const int nbtilings = 16;
	const int active_cell = nbtilings*11+1;
	const float alpha_v = .3f ;
	const float alpha_w = .0005f / (active_cell);
	const float gamma_tp1 = 0.f;
	const float lambda_t = 0.f;

	const int runs = ::pow(10, 3);

	const std::string v = "RobotWeightsVect";
	const std::string w = "RobotWeightsVectW";

	RLLib::RLProblem<T>* robocupProblem;
	BinaryToTextFile<float>* converter;
	std::vector<ExperienceLine<T>> testPos;
	std::vector<ExperienceLine<T>> testNeg;
	std::vector<std::string> rmsetest(std::vector<std::string>(runs));

	typedef typename RLLib::RLRunner<T>::Event Event;

public:
	RobocupTest(RLLib::RLProblem<T>* robocupProblem_,  std::vector<ExperienceLine<T>>& testPos_,  std::vector<ExperienceLine<T>>& testNeg_) :
			robocupProblem(robocupProblem_), converter(new BinaryToTextFile<float>()),
			testPos(testPos_), testNeg(testNeg_) {

	}
	void testGoalPredictionGQLambda() {
		RLLib::Random<T>* random = new RLLib::Random<T>();
		random->reseed(HASH_SEED);
		RLLib::Hashing<T>* hashing = new RLLib::UNH<T>(random, MEM_SIZE);

		/*
		 * UNCOMMENT TO CHECK THE COLISION ON THE HASHED FEAUTURES VECTOR
		 *
		 RLLib::Random<float>* colision_random = new RLLib::Random<float>();
		 colision_random->reseed(HASH_SEED);
		 RLLib::Hashing<float>* hashCollision = new RLLib::ColisionDetection<float>(hashing, colision_random, MEM_SIZE, 0);
		 */

		RLLib::PVector<T>* resolutions = new RLLib::PVector<T>(
				robocupProblem->dimension());
		resolutions->set(10.f);
		resolutions->setEntry(3, 5.f);

		RLLib::Projector<T>* projector = new RLLib::TileCoderHashing<T>(hashing,
				robocupProblem->dimension(), resolutions, nbtilings, true);
		RLLib::StateToStateAction<T>* toStateAction =
				new RLLib::StateActionTilings<T>(projector,
						robocupProblem->getDiscreteActions());
		RLLib::Trace<T>* e = new RLLib::ATrace<T>(projector->dimension());

		RLLib::GQ<T>* gq = new RLLib::GQ<T>(alpha_v, alpha_w, gamma_tp1, lambda_t, e);

		/*
		 *UNCOMMENT TO INIZIALIZE THE MAIN WEIGHTS VECTOR ARBITRARILY
		 *
		 *
		 *
		 *
		 */
		RLLib::PVector<float>* weights =
				static_cast<RLLib::PVector<float>*>(gq->weights());
		RLLib::Random<float>* iniz = new RLLib::Random<float>();
		iniz->reseed(time(0));
		for (int i = 0; i < weights->dimension(); i++) {
			weights->setEntry(i, iniz->nextReal() - 0.5f);
		}
		gq->persist(v.c_str());

		/*gq->resurrect(v.c_str());*/

		RLLib::Policy<T>* behavior = new RLLib::SingleActionPolicy<T>(
				robocupProblem->getDiscreteActions());

		RLLib::Policy<T>* target = new RLLib::SingleActionPolicy<T>(
				robocupProblem->getDiscreteActions());

		RLLib::OffPolicyControlLearner<T>* control = new RLLib::GreedyGQ<T>(
				target, behavior, robocupProblem->getDiscreteActions(),
				toStateAction, gq);

		RLLib::RLAgent<T>* learnerAgent = new RLLib::LearnerAgent<T>(control);

		RLLib::RLRunner<T>* roboRunner = new RobocupRunner<T>(learnerAgent, robocupProblem, 1,
				std::pow(10, 3), 1, gq->predictions);

		float min = .3f;

		for (int i = 0; i < runs; i++) {
			if(i%20==0){
				float alpha_t = alpha_v*::pow(i+1, -0.15f);
				alpha_t /= (active_cell);
				gq->updateAlpha(alpha_t);
			}
			roboRunner->runEpisodes();
			float rmse = calcRmse(toStateAction, gq, behavior->sampleBestAction());
			if(rmse< min){
				min = rmse;
				convertWeightVectors(std::to_string(rmse));
			}
		}
		calcAccuracyPrecisionRecallTest(toStateAction, gq, behavior->sampleBestAction());

		gq->persist(v.c_str());
		convertWeightVectors();

		printPredictions(gq->predictions);
	}

	void calcAccuracyPrecisionRecallTest(RLLib::StateToStateAction<T>* toStateAction, RLLib::GQ<T>* gq, const RLLib::Action<T>* action){
		long res_a=0,res_b=0,res_c=0,res_d=0;
		long correct=0,incorrect=0;
		long totdoc = testNeg.size() + testPos.size();

		const RLLib::Vector<T>* x_t;
		const RLLib::Vector<T>* phi_t;
		const RLLib::Representations<T>* xas_t;

		typename std::vector<ExperienceLine<T>>::iterator iter;
		for(iter=testNeg.begin(); iter!=testNeg.end(); iter++){
			x_t = createStateVector(iter->x0);
			xas_t = toStateAction->stateActions(x_t);
			phi_t = xas_t->at(action);
			float pred = gq->predict(phi_t);
			if(pred<0){
				correct++;
				res_d++;
			}
			else{
				incorrect++;
				res_b++;
			}
		}

		for(iter=testPos.begin(); iter!=testPos.end(); iter++){
			x_t = createStateVector(iter->x0);
			xas_t = toStateAction->stateActions(x_t);
			phi_t = xas_t->at(action);
			float pred = gq->predict(phi_t);
			if(pred>0){
				correct++;
				res_a++;
			}
			else{
				incorrect++;
				res_c++;
			}
		}

		printf("Accuracy on test set: %.2f%% (%ld correct, %ld incorrect, %ld total)\n",(float)(correct)*100.0/totdoc,correct,incorrect,totdoc);
		printf("Precision/recall on test set: %.2f%%/%.2f%%\n",(float)(res_a)*100.0/(res_a+res_b),(float)(res_a)*100.0/(res_a+res_c));
		std::cout << std::endl;
	}

	float calcRmse(RLLib::StateToStateAction<T>* toStateAction, RLLib::GQ<T>* gq, const RLLib::Action<T>* action){
		float rmse = 0.f;
		typename std::vector<ExperienceLine<T>>::iterator iter;
		float deltasum = 0.f;
		float deltasumPos;
		float deltasumNeg;
		const RLLib::Vector<T>* x_t;
		const RLLib::Vector<T>* phi_t;
		const RLLib::Representations<T>* xas_t;
		for(iter=testNeg.begin(); iter!=testNeg.end(); iter++){
			x_t = createStateVector(iter->x0);
			xas_t = toStateAction->stateActions(x_t);
			phi_t = xas_t->at(action);
			deltasum += ::pow(iter->z - gq->predict(phi_t), 2.f);
		}

		deltasumNeg = deltasum;

		for(iter=testPos.begin(); iter!=testPos.end(); iter++){
			x_t = createStateVector(iter->x0);
			xas_t = toStateAction->stateActions(x_t);
			phi_t = xas_t->at(action);
			deltasum += ::pow(iter->z - gq->predict(phi_t), 2.f);
		}

		deltasumPos = deltasum - deltasumNeg;

		// RMSE all examples
		int N = testNeg.size() + testPos.size();
		rmse = ::pow(deltasum/N, 0.5f);
		std::ostringstream outstream;
		outstream << rmse;
		converter->appendStringToTextFile(outstream.str(), "RMSE");

		// RMSE negative examples
		N = testNeg.size();
		rmse = ::pow(deltasumNeg/N, 0.5f);
		outstream.str("");
		outstream.clear();
		outstream << rmse;
		converter->appendStringToTextFile(outstream.str(), "RMSE-Neg");

		// RMSE positive examples
		N = testPos.size();
		rmse = ::pow(deltasumPos/N, 0.5f);
		outstream.str("");
		outstream.clear();
		outstream << rmse;
		converter->appendStringToTextFile(outstream.str(), "RMSE-Pos");


		return rmse;
	}

	RLLib::Vector<T>* createStateVector(std::vector<T> vector){
		RLLib::Vector<T>* o_tp1 = new RLLib::PVector<T>(vector.size());
		for(unsigned int i=0; i<vector.size(); i++){
			o_tp1->setEntry(i, vector.at(i));
		}
		return o_tp1;
	}

	void printStatistics(std::vector<Event*>& events, std::string filename = "EpisodesStats") {
		for (typename std::vector<Event*>::iterator iter = events.begin();
				iter != events.end(); ++iter) {
			std::ostringstream outstream;
			outstream << (*iter)->averageTimePerStep << " ";
			outstream << (*iter)->episodeZ;
			converter->appendStringToTextFile(outstream.str(), filename);
		}
	}

	void printPredictions(std::vector<std::string>& predictions, std::string filename = "RobotPredictions") {
		converter->writeStringToTextFile(predictions, filename);
	}

	void convertWeightVectors(std::string filename = "") {
		converter->readBinaryVectorFile(v);
		converter->writeTextVectorFile("RobotWeightsVectText" + filename);
		converter->readBinaryVectorFile(w);
		converter->writeTextVectorFile("RobotWeightsVectWText" + filename);
	}
};

#endif /* ROBOCUPTEST_H_ */
