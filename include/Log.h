/*
 * ApplicatioLog.h
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#ifndef SRC_APPLICATIOLOG_H_
#define SRC_APPLICATIOLOG_H_

#include <string>

class Log
{
public:
	static void SDLFault(std::string message);
	static void Fault(std::string message);
	static void Alert(std::string message);
	static void Info(std::string message);
};

#endif /* SRC_APPLICATIOLOG_H_ */
