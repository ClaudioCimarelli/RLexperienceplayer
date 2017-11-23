/*
 * RobocupProblem.h
 *
 *  Created on: Nov 10, 2017
 *      Author: claudio
 */

#ifndef ROBOCUPPROBLEM_H_
#define ROBOCUPPROBLEM_H_

#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <ctime>

#include "ExperienceLine.h"
#include "Learning/RL.h"
#include "Learning/Action.h"

template<typename T>
class RobocupProblem: public RLLib::RLProblem<T> {

private:
	typedef RLLib::RLProblem<T> Base;
	static const int ACTION_NUM = 1;
	static const int VARS_NUM = 18;

	std::vector<ExperienceLine<T>> positiveExp;
	std::vector<ExperienceLine<T>> negativeExp;

	std::vector<int> shuffledPositive;
	std::vector<int> shuffledNegative;
	int counter;

	ExperienceLine<T> currentExperience;

public:
	RobocupProblem(RLLib::Random<T>* random,
			const std::vector<ExperienceLine<T>>& positiveExp_,
			const std::vector<ExperienceLine<T>>& negativeExp_) :
			RLLib::RLProblem<T>(random, VARS_NUM, ACTION_NUM, 0), positiveExp(
					positiveExp_), negativeExp(negativeExp_), counter(0) {
		std::srand(unsigned(std::time(0)));
		random->reseed(unsigned(std::time(0)));

		resetShuffleVector(shuffledPositive, positiveExp.size());
		resetShuffleVector(shuffledNegative, negativeExp.size());

		Base::discreteActions->push_back(0, 0.f);
	}

	void initialize() {
		setCurrentExperience();
		typename std::vector<T>::iterator iter;
		int index = 0;
		for(iter=currentExperience.x0.begin(); iter!=currentExperience.x0.end(); iter++){
			Base::output->o_tp1->setEntry(index, *iter);
			index++;
		}
		Base::output->updateTRStep(0.f, 0.f, false);

	}

	void step(const RLLib::Action<T>* action) {

	}

	void updateTRStep() {
		typename std::vector<T>::iterator iter;
		int index = 0;
		for(iter=currentExperience.x1.begin(); iter!=currentExperience.x1.end(); iter++){
			Base::output->o_tp1->setEntry(index, *iter);
			index++;
		}
	}

	bool endOfEpisode() const {
		return currentExperience.endOfEpisode;
	}

	T r() const {
		return currentExperience.r;
	}
	T z() const {
		return currentExperience.z;
	}

	void resetShuffleVector(std::vector<int> &shuffleVector, const int size) {
		shuffleVector = std::vector<int>(size);
		std::iota(shuffleVector.begin(), shuffleVector.end(), 0);
		std::random_shuffle(shuffleVector.begin(), shuffleVector.end());
	}

	void setCurrentExperience() {
		std::vector<int>& tempIndex = (counter % 5 == 0)? shuffledPositive: shuffledNegative;
		std::vector<ExperienceLine<T>>& tempExp = (counter % 5 == 0)? positiveExp: negativeExp;

		int index = tempIndex.back();
		currentExperience = tempExp.at(index);
		tempIndex.pop_back();
		if (tempIndex.size() == 0) {
			resetShuffleVector(tempIndex, tempExp.size());
		}
		counter++;
	}

};

#endif /* ROBOCUPPROBLEM_H_ */
