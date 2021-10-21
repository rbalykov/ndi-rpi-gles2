/*
 * ApplicatioLog.cpp
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#include "Log.h"

#include <iostream>


void Log::Fault(std::string message)
{
	std::cerr << "Failure: " << message << "\n";
}

void Log::Alert(std::string message)
{
	std::cerr << "Alert: " << message << "\n";
}

void Log::Info(std::string message)
{
	std::clog << message << "\n";
}


void LOG_DEBUG()
{
	std::cerr << std::endl;
}
