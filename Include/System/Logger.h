/* Neko Engine
 *
 * Logger.h
 * Author: Alexandru Naiman
 *
 * Engine logger
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <string>
#include <vector>
#include <stdarg.h>

#define LOG_DEBUG		0
#define LOG_INFORMATION		1
#define LOG_WARNING		2
#define LOG_CRITICAL		3

#define	LOG_ALL			LOG_INFORMATION

struct LogMessage
{
	LogMessage(std::string module,
		unsigned int severity,
		std::string message) :
		Module(module),
		Severity(severity),
		Message(message) 
	{ }

	std::string Module;
	unsigned int Severity;
	std::string Message;
};

class Logger
{
public:
	static void Initialize(std::string file, unsigned int severity) noexcept;
	static void Log(std::string Module, unsigned int severity, const char* format, ...);
	static void Log(std::string Module, unsigned int severity, std::string Message);
	static void LogRendererDebugMessage(const char* message) noexcept;
	static void EnqueueLogMessage(std::string Module, unsigned int severity, std::string Message) noexcept;
	static void EnqueueLogMessage(std::string Module, unsigned int severity, const char* format, ...) noexcept;
	static void Flush();

private:
	static std::string _logFile;
	static unsigned int _logSeverity;
	static std::vector<LogMessage> _logQueue;

	static void _WriteMessage(LogMessage& msg);
};

