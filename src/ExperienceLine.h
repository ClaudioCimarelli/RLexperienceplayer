/*
 * ExperienceLine.h
 *
 *  Created on: Nov 13, 2017
 *      Author: claudio
 */

#ifndef EXPERIENCELINE_H_
#define EXPERIENCELINE_H_
#pragma once

template<typename T>
struct ExperienceLine {
	std::vector<T> x0;
	int a_id;
	bool endOfEpisode;
	std::vector<T> x1;
	float r;
	float z;
};

#endif /* EXPERIENCELINE_H_ */
