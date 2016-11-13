/* NekoEngine
 *
 * Logger.cpp
 * Author: Alexandru Naiman
 *
 * Engine logger implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <System/Logger.h>

#include <Platform/Platform.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define LOG_BUFF	4096

using namespace std;

string Logger::_logFile = "Engine.log";
unsigned int Logger::_logSeverity = 0;
vector<LogMessage> Logger::_logQueue;

static string _SeverityStr[4] =
{
	"Debug",
	"Information",
	"Warning",
	"Critical"
};

void Logger::Initialize(string file, unsigned int severity) noexcept
{
	_logFile = file;
	_logSeverity = severity;
	_logQueue.reserve(10);
}

void Logger::Log(std::string Module, unsigned int severity, const char *format, ...)
{
	va_list args;
	char buff[LOG_BUFF];
	memset(buff, 0x0, LOG_BUFF);

	va_start(args, format);
	vsnprintf(buff, LOG_BUFF, format, args);
	va_end(args);

	LogMessage msg(Module, severity, buff);
	_WriteMessage(msg);
}

void Logger::Log(string Module, unsigned int severity, string Message)
{
	LogMessage msg(Module, severity, Message);
	_WriteMessage(msg);
}

void Logger::LogRendererDebugMessage(const char *message) noexcept
{
	LogMessage lmsg("Renderer", LOG_DEBUG, message);
	_WriteMessage(lmsg);
}

void Logger::EnqueueLogMessage(std::string Module, unsigned int severity, const char *format, ...) noexcept
{
	va_list args;
	char buff[LOG_BUFF];
	memset(buff, 0x0, LOG_BUFF);

	va_start(args, format);
	vsnprintf(buff, LOG_BUFF, format, args);
	va_end(args);

	_logQueue.push_back(LogMessage(Module, severity, buff));
}

void Logger::EnqueueLogMessage(std::string Module, unsigned int severity, std::string Message) noexcept
{
	_logQueue.push_back(LogMessage(Module, severity, Message));
}

void Logger::Flush()
{
	for (LogMessage &msg : _logQueue)
		_WriteMessage(msg);
	_logQueue.clear();
}

void Logger::_WriteMessage(LogMessage &msg)
{
    FILE *fp = nullptr;
	
	// Log all messages in debug mode
#if !defined(NE_CONFIG_DEBUG) && !defined(NE_CONFIG_DEVELOPMENT) 
	if (msg.Severity < _logSeverity)
		return;
#else
	if (msg.Severity == LOG_CRITICAL)
		fprintf(stderr, "[%s][%s]: %s", msg.Module.c_str(), _SeverityStr[msg.Severity].c_str(), msg.Message.c_str());

	char buff[2048];
	if (snprintf(buff, 2048, "[%s][%s]: %s", msg.Module.c_str(), _SeverityStr[msg.Severity].c_str(), msg.Message.c_str()) >= 1024)
		Platform::LogDebugMessage("MESSAGE TRUNCATED");
	Platform::LogDebugMessage(buff);
#endif

	if (msg.Severity > LOG_ALL)
		msg.Severity = LOG_ALL;

	if((fp = fopen(_logFile.c_str(), "a+")) == nullptr)
	{
	    perror("failed to open log file for append\n");
		return;
	}

	time_t t = time(0);
	struct tm *tm = localtime(&t);

	fprintf(fp, "%d-%d-%d-%d:%d:%d [%s][%s]: %s",
	        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	        tm->tm_hour, tm->tm_min, tm->tm_sec,
	        msg.Module.c_str(), _SeverityStr[msg.Severity].c_str(), msg.Message.c_str());

	if(msg.Message[msg.Message.size() - 1] != '\n')
	    fprintf(fp, "\n");

	fclose(fp);
}
