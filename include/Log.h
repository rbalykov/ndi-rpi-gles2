/*
 * ApplicatioLog.h
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#ifndef SRC_APPLICATIOLOG_H_
#define SRC_APPLICATIOLOG_H_

#include <string>
#include <iostream>


extern "C" void LOG_DEBUG();

template<typename HEAD, typename... ARGS>
void LOG_DEBUG(const HEAD& head, const ARGS&... args )
{
    std::cerr << " " << head;
    LOG_DEBUG(args...);
}

#define DBG(...) LOG_DEBUG(__func__,":line", __LINE__, ":",__VA_ARGS__)


class Log
{
public:
	static void Fault(std::string message);
	static void Alert(std::string message);
	static void Info(std::string message);

	static void Debug(std::string message);
};

#endif /* SRC_APPLICATIOLOG_H_ */
