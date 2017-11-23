/*
 * RobocupRunner.h
 *
 *  Created on: Nov 10, 2017
 *      Author: claudio
 */

#ifndef ROBOCUPRUNNER_H_
#define ROBOCUPRUNNER_H_
#pragma once

#include "Learning/RL.h"

template<typename T>
class RobocupRunner: public RLLib::RLRunner<T> {

private:
	typedef RLLib::RLRunner<T> Base;
	bool verbose;
	const std::vector<std::string> &outputStrings;

public:

	RobocupRunner(RLLib::RLAgent<T>* agent, RLLib::RLProblem<T>* problem,
			const int& maxEpisodeTimeSteps, const int nbEpisodes,
			const int nbRuns, const std::vector<std::string> &strings, bool verbose=false) :
			Base(agent, problem, maxEpisodeTimeSteps, nbEpisodes, nbRuns),
			verbose(verbose), outputStrings(strings){
	}

	~RobocupRunner(){

	}

	void step() {
		/*Initialize the problem variables*/
		if(!Base::agentAction)
			Base::problem->initialize();
		else
			Base::problem->updateTuple();

		RLLib::TRStep<T>* step = Base::problem->getTRStep();

		if (!Base::agentAction) {
			/*Initialize the control agent and get the first action*/
			Base::agentAction = Base::agent->initialize(step);
		}
		else {
			++Base::timeStep;
			Base::agentAction = Base::agent->getAtp1(step);

			if (Base::problem->endOfEpisode() || (Base::timeStep == Base::maxEpisodeTimeSteps)) {
				/*Set the initial marker*/
				Base::agentAction = 0;
				++Base::nbEpisodeDone;
				if(verbose){
					std::cout << Base::nbEpisodeDone << "   " << outputStrings.back() << std::endl;
				}
			} else {
				Base::problem->step(Base::agentAction);
			}
		}
	}

	void runEpisodes(){
		RLLib::Timer runTime = RLLib::Timer();
		runTime.start();
		do
		{
		  step();
		}
		while (Base::nbEpisodeDone < Base::nbEpisodes);
		Base::nbEpisodeDone = 0;
		runTime.stop();
		std::cout << "\n@@ Run Time in Sec =" << runTime.getElapsedTime() << std::endl;
	}
};

#endif /* ROBOCUPRUNNER_H_ */
