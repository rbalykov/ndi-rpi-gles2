/*
 * ApplicatioLog.cpp
 *
 *  Created on: 16 окт. 2021 г.
 *      Author: pi
 */

#include "Log.h"

#include <iostream>
#include "SDL2/SDL_error.h"

void Log::SDLFault(std::string message)
{
	std::cerr << "SDL Failure: " << message << "\n" << SDL_GetError() << "\n";
}

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
	std::cout << message << "\n";
}

