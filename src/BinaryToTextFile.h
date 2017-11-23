/*
 * BinaryToTextFile.h
 *
 *  Created on: Nov 12, 2017
 *      Author: claudio
 */

#ifndef BINARYTOTEXTFILE_H_
#define BINARYTOTEXTFILE_H_
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

template<typename T>
class BinaryToTextFile {
private:
	std::vector<T> data;
	int vectorType, capacity;
	typedef typename std::vector<T>::iterator iterator;
public:
	BinaryToTextFile() :
			data(std::vector<T>()), vectorType(0), capacity(0) {
	}

	~BinaryToTextFile() {
		delete &data;
	}


	void readBinaryVectorFile(const std::string& inputFilename) {
		if(inputFilename==""){
			std::cout << "Set Input Filename" << std::endl;
			return;
		}
		std::ifstream ifs;
		ifs.open(inputFilename.c_str());
		if (ifs.is_open()) {
			// Read vectorType
			char *s;
			s = (char *) &vectorType;
			ifs.read(s, sizeof(vectorType));
			// Read capacity
			s = (char *) &capacity;
			ifs.read(s, sizeof(capacity));
			data = std::vector<T>(capacity);
			typename std::vector<T>::iterator iter;
			for (iter = data.begin(); iter != data.end(); iter++) {
				s = (char*) &(*iter);
				ifs.read(s, sizeof(*iter));
			}
		}
	}

	void writeTextVectorFile(const std::string& outputFilename){
		if(outputFilename==""){
			std::cout << "Set Output Filename" << std::endl;
			return;
		}
		std::ofstream ofs;
		ofs.open(outputFilename.c_str());
		if (ofs.is_open()) {
			ofs << vectorType << std::endl;
			ofs << capacity << std::endl;
			for (iterator iter = data.begin(); iter != data.end(); iter++) {
				ofs << *iter;
				ofs << std::endl;
			}
		}
	}

	void writeStringToTextFile(const std::vector<std::string>& textLines, const std::string& outputFilename){
		std::ofstream ofs;
		ofs.open(outputFilename.c_str(), std::ios::app);
		if (ofs.is_open()) {
			std::vector<std::string>::const_iterator iter;
			for(iter=textLines.begin(); iter!=textLines.end(); iter++){
				ofs << *iter << std::endl;
			}
		}
	}

	void appendStringToTextFile(const std::string& line, const std::string& outputFilename){
		if(outputFilename==""){
			std::cout << "Set Output Filename" << std::endl;
			return;
		}
		std::ofstream ofs;
		ofs.open(outputFilename.c_str(), std::ios::app);
		if (ofs.is_open()) {
			ofs << line << std::endl;
		}
	}
};

#endif /* BINARYTOTEXTFILE_H_ */
