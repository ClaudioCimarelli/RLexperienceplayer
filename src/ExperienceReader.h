/*
 * ExperienceReader.h
 *
 *  Created on: Nov 8, 2017
 *      Author: claudio
 */

#ifndef EXPERIENCEREADER_H
#define EXPERIENCEREADER_H
#pragma once

#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include "LineParser.h"
#include "ExperienceLine.h"

template<typename T>
class ExperienceReader {

private:
	static const int VARS_NUM = 18;
	static const int ATTR_NUM = 40;

	std::string filename;
	std::vector<ExperienceLine<T>> experienceLines;
	typedef typename std::vector<ExperienceLine<T>>::const_iterator Iterator;

public:

	ExperienceReader(const std::string& filename_) :
			filename(filename_), experienceLines() {
		readExperienceFile();
		std::srand(unsigned(std::time(0)));
	}

	~ExperienceReader() {
	}

	std::vector<ExperienceLine<T>> getExperience() {
		return experienceLines;
	}

	int readExperienceFile() {
		std::ifstream inputFile;
		std::string line;
		LineParser parser;
		int lineCount = 0;
		inputFile.open(filename.c_str());
		if (inputFile.is_open() && getline(inputFile, line)) {
			int size = std::stoi(line);
			experienceLines.clear();
			experienceLines.reserve(size);
			while (getline(inputFile, line)) {
				std::vector<std::string> parsedLine;
				parsedLine = parser.parseLine(line);
				createExperienceLine(parsedLine);
				lineCount++;
			}
		} else {
			std::cout << "Unable to open file" << std::endl;
		}
		return lineCount;
	}

	void createExperienceLine(const std::vector<std::string>& tokens) {
		int vars_n = VARS_NUM;

		ExperienceLine<T> exp;
		exp.x0.reserve(VARS_NUM);
		exp.x1.reserve(VARS_NUM);

		if (tokens.size() == 32){
			vars_n = 14;
		}

		int i = 0;
		std::vector<std::string>::const_iterator iter;
		for (iter = tokens.begin(); iter != tokens.end(); iter++) {
			if (i < vars_n)
				exp.x0.push_back(convert_to(*iter));
			else if (i == vars_n)
				exp.a_id = std::stoi(*iter);
			else if (i == vars_n + 1){
				exp.endOfEpisode = std::stoi(*iter)==1;
			}
			else if (i < vars_n * 2 + 2)
				exp.x1.push_back(convert_to(*iter));
			else if (i == vars_n * 2 + 2)
				exp.r = convert_to(*iter);
			else{
				exp.z = convert_to(*iter);
				exp.z = exp.z>0?T(1):T(-1);
			}

			i++;
		}

		if (tokens.size() == 32){
			for(int i = 0; i<4; i++){
				exp.x0.push_back(T(1));
				exp.x1.push_back(T(1));
			}
		}
		experienceLines.push_back(exp);
	}

	std::vector<std::vector<ExperienceLine<T>>> partitionByZ(const T z_value){
		typename std::vector<ExperienceLine<T>>::iterator bound;
		bound = std::partition(experienceLines.begin(), experienceLines.end(), [&z_value](ExperienceLine<T> exp) {return exp.z==z_value;});

		std::vector<ExperienceLine<T>> part1(experienceLines.begin(), bound);
		std::vector<ExperienceLine<T>> part2(bound, experienceLines.end());
		std::vector<std::vector<ExperienceLine<T>>> partition;
		partition.reserve(2);
		partition.push_back(part1);
		partition.push_back(part2);
		return partition;
	}

	std::vector<std::vector<ExperienceLine<T>>> createTrainTestSet(std::vector<ExperienceLine<T>>& experienceSet, const float trainDim=0.8f){
		std::random_shuffle(experienceSet.begin(), experienceSet.end());
		std::vector<std::vector<ExperienceLine<T>>> trainAndTest;
		trainAndTest.reserve(2);
		std::vector<ExperienceLine<T>> temp;
		temp = std::vector<ExperienceLine<T>>(experienceSet.begin(), experienceSet.begin()+ (int)experienceSet.size()*trainDim);
		trainAndTest.push_back(temp);
		temp = std::vector<ExperienceLine<T>>(experienceSet.begin() + (int)experienceSet.size()*trainDim, experienceSet.end());
		trainAndTest.push_back(temp);
		return trainAndTest;
	}

	void create_svmfile(const std::vector<ExperienceLine<T>>& positives, std::vector<ExperienceLine<T>>& negatives, const std::string& filename){
		std::ofstream ofs;
		ofs.open(filename.c_str());
		if (ofs.is_open()) {
			Iterator iter;
			for(iter=positives.begin(); iter!=positives.end(); iter++){
				ofs << convertExperienceToSVMline(*iter) << std::endl;
			}
			std::random_shuffle(negatives.begin(), negatives.end());
			for(iter=negatives.begin(); iter!=negatives.begin() + (int)negatives.size()*0.09f; iter++){
				ofs << convertExperienceToSVMline(*iter) << std::endl;
			}
		}
	}

private:
	T convert_to (const std::string &str)
	{
	    std::istringstream ss(str);
	    T num;
	    ss >> num;
	    return num;
	}

	std::string convertExperienceToSVMline(const ExperienceLine<T>& experience){
		std::string svmline = std::to_string(experience.z>0?1:-1);
		int count = 1;
		for(typename std::vector<T>::const_iterator iter=experience.x0.begin(); iter!= experience.x0.end(); iter++){
			svmline += " " + std::to_string(count) + ":" + std::to_string(*iter);
			count++;
		}
		return svmline;
	}


};

#endif
