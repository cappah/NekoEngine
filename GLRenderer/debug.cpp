/* Neko Engine
 *
 * debug.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL Renderer Implementation
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

#include <stdio.h>

#include "GLRenderer.h"

void LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg)
{
	const char *sourceString, *typeString, *severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source)
	{
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API:
		sourceString = "OpenGL|API";
		break;
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION:
		sourceString = "OpenGL|Application";
		break;
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sourceString = "OpenGL|Window System";
		break;
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sourceString = "OpenGL|Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sourceString = "OpenGL|Third Party";
		break;
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER:
		sourceString = "OpenGL|Other";
		break;

	default:
		sourceString = "OpenGL|Unknown";
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		typeString = "Error";
		break;
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeString = "Deprecated Behavior";
		break;
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeString = "Undefined Behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		typeString = "Portability";
		break;
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeString = "Performance";
		break;
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER:
		typeString = "Other";
		break;
	default:
		typeString = "Unknown";
		break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		severityString = "High";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		severityString = "Medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		severityString = "Low";
		break;
	default:
		severityString = "Unknown";
		break;
	}

	if (_debugLogFunc)
	{
		char buff[1024];
		if (snprintf(buff, 1024, "[%s][%s][%s][%u]: %s", sourceString, severityString, typeString, id, msg) >= 1024)
			_debugLogFunc("MESSAGE TRUNCATED");;
		_debugLogFunc(buff);
	}
	else
		printf("[%s][%s][%s][%u]: %s\n", sourceString, severityString, typeString, id, msg);
}

void DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	LogDebugOutput(category, category, id, severity, message);
}

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam)
{
	LogDebugOutput(source, type, id, severity, message);
}
