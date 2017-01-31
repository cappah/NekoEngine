/* NekoEngine
 *
 * Android.cpp
 * Author: Alexandru Naiman
 *
 * Android platform support
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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

#include <Engine/Engine.h>
#include <System/Logger.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <android/log.h>
#include <android/sensor.h>

#define ANDROID_PLATFORM_MODULE	"AndroidPlatform"
#define	DL_BUFF_SIZE			512

static struct android_app *_app;
static ASensorManager *_sensorManager;

static const ASensor *_accelerometer;
static const ASensor *_gyroscope;
static const ASensor *_lightSensor;

static ASensorEventQueue *_sensorEventQueue;

static NString _sdkVersion;

static NString _internalDataPath;
static NString _externalDataPath;

int32_t _android_handle_input_event(struct android_app* app, AInputEvent* event);
void _android_handle_sensor_event(ASensorEventQueue *queue, const ASensor *_accelerometer, const ASensor *_gyroscope, const ASensor *_lightSensor);

bool _android_download_file(const char *address, const char *file)
{
	int sock{ 0 };

	char c{};
	int i{}, j{}, k{}, l{}, m{};
	FILE *f{ fopen(file, "wb") };
	char host[512]{};
	const char *url{ address + (strchr(address, '/') - address) };
	struct addrinfo *ai{ nullptr };
	struct addrinfo hints{};
	char buff[DL_BUFF_SIZE]{};
	snprintf(host, strchr(address, '/') - address + 1, "%s", address);

	if (!f)
	{
		fclose(f);
		Logger::Log(ANDROID_PLATFORM_MODULE, LOG_CRITICAL, "failed to open file for writing: %s", file);
		return false;
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	snprintf(buff, DL_BUFF_SIZE, "GET %s HTTP/1.1\r\n", url);

	if (i = getaddrinfo(host, "80", &hints, &ai))
	{
		fclose(f);
		Logger::Log(ANDROID_PLATFORM_MODULE, LOG_CRITICAL, "getaddrinfo() failed: %s", gai_strerror(i));
		return false;
	}

	snprintf(buff + strlen(buff), DL_BUFF_SIZE, "Host: %s\r\n\r\n", host);

	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (connect(sock, ai->ai_addr, ai->ai_addrlen))
	{
		fclose(f);
		freeaddrinfo(ai);
		Logger::Log(ANDROID_PLATFORM_MODULE, LOG_CRITICAL, "connect() failed");
		return false;
	}

	freeaddrinfo(ai);

	i = send(sock, buff, strlen(buff), 0);
	if ( i < strlen(buff) || i == -1)
	{
		fclose(f);
		Logger::Log(ANDROID_PLATFORM_MODULE, LOG_CRITICAL, "send() failed");
		return false;
	}

	while(strncmp(buff, "\r\n", 2))
	{
		for (i = 0; strcmp(buff + i - 2, "\r\n"); i++) { recv(sock, buff + i, 1, 0); buff[i + 1] = 0x0; }
		if (strstr(buff, "HTTP/") == buff)
		{
			fputs(strchr(buff, ' ') + 1, stdout);
			if (strcmp(strchr(buff, ' ') + 1, "200 OK\r\n"))
			{
				Logger::Log(ANDROID_PLATFORM_MODULE, LOG_CRITICAL, "http response: %s", buff);
				return false;
			}
		}
		if (strstr(buff, "Content-Length:") == buff)
		{
			*strchr(buff, '\r') = 0x0;
			j = atoi(strchr(buff, ' ') + 1);
			l = j / 100;
		}
	}

	for (i = 0, k = 0, m = 0; i < j; i++, k++)
	{
		recv(sock, &c, 1, 0);
		fputc(c, f);
		if (k == l) { m++; k = 0; }
	}

	fclose(f);

	return true;
}

void _android_handle_app_cmd(struct android_app *app, int32_t cmd)
{
	struct stat st{};
	switch (cmd)
	{
		case APP_CMD_INIT_WINDOW:
		{
			NString args = "--log=";
			args.Append(_externalDataPath);
			args.Append("/engine.log --data=");
			args.Append(_externalDataPath);
			args.Append("/data --ini=");
			args.Append(_externalDataPath);
			args.Append("/engine.ini");

			// check & download data

			char buff[512];
			snprintf(buff, 512, "%s/data", *_externalDataPath);
			mkdir(*_externalDataPath, 0777);
			mkdir(buff, 0777);

			snprintf(buff, 512, "%s/data/core_android.nar", *_externalDataPath);
			if(access(buff, 0)) _android_download_file("192.168.7.6/runnergame_android/core_android.nar", buff);

			snprintf(buff, 512, "%s/engine.ini", *_externalDataPath);
			if(access(buff, 0)) _android_download_file("192.168.7.6/runnergame_android/engine.ini", buff);

			if(Engine::Initialize(*args, false) != ENGINE_OK)
				Platform::Exit();

			Engine::Run();
		}
		break;
		case APP_CMD_TERM_WINDOW:
			Platform::Exit();
		break;
	}
}

void android_main(struct android_app *state)
{
	int ident{ 0 };
	int events{ 0 };
	struct android_poll_source *source{ nullptr };
	struct stat st;
	bool exit{ false };

	app_dummy();

	_app = state;
	_app->onAppCmd = _android_handle_app_cmd;
	_app->onInputEvent = _android_handle_input_event;

	_internalDataPath = _app->activity->internalDataPath;
	_externalDataPath = _app->activity->externalDataPath;

	if(stat(*_internalDataPath, &st) || !(st.st_mode & S_IFDIR))
		mkdir(*_internalDataPath, 0770);

	if(stat(*_externalDataPath, &st) || !(st.st_mode & S_IFDIR))
		mkdir(*_externalDataPath, 0770);

	_sensorManager = ASensorManager_getInstance();
	_accelerometer = ASensorManager_getDefaultSensor(_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	_gyroscope = ASensorManager_getDefaultSensor(_sensorManager, ASENSOR_TYPE_GYROSCOPE);
	_lightSensor = ASensorManager_getDefaultSensor(_sensorManager, ASENSOR_TYPE_LIGHT);

	_sensorEventQueue = ASensorManager_createEventQueue(_sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

	int ver = AConfiguration_getSdkVersion(_app->config);
	_sdkVersion = NString::StringWithFormat(10, "%d", ver);

	while(!exit)
	{
		while((ident = ALooper_pollAll(0, NULL, &events, (void **)&source)) >= 0)
		{
			if (source) source->process(_app, source);

			if (ident == LOOPER_ID_USER)
			{
				//
			}

			if (state->destroyRequested)
				Engine::Exit();
		}

		Engine::Frame();
	}
}

PlatformWindowType Platform::_activeWindow = nullptr;

const char* Platform::GetName()
{
	return "Android";
}

const char* Platform::GetVersion()
{
	return *_sdkVersion;
}

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	return _app->window;
}

void Platform::SetWindowTitle(PlatformWindowType hWnd, const char* title) { (void)hWnd; (void)title; }

bool Platform::EnterFullscreen(int width, int height) { (void)width; (void)height; return true; }

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
/*	NSAlert *alert = [[NSAlert alloc] init];
	if(!alert)
	{ DIE("Failed to create NSAlert"); }
	
	switch (buttons)
	{
		case MessageBoxButtons::YesNo:
		{
			[alert addButtonWithTitle:@"Yes"];
			[alert addButtonWithTitle:@"No"];
		}
		break;
		case MessageBoxButtons::OK:
		default:
			[alert addButtonWithTitle:@"OK"];
		break;
	}
	
	[alert setMessageText:[NSString stringWithUTF8String:message]];
	
	switch (icon)
	{
		case MessageBoxIcon::Warning:
			[alert setAlertStyle:NSWarningAlertStyle];
		break;
		case MessageBoxIcon::Error:
			[alert setAlertStyle:NSCriticalAlertStyle];
		break;
		case MessageBoxIcon::Question:
			[alert setAlertStyle:NSInformationalAlertStyle];
		break;
		case MessageBoxIcon::Information:
		default:
			[alert setAlertStyle:NSInformationalAlertStyle];
		break;
	}
	
	if([alert runModal] == NSAlertFirstButtonReturn)
	{
		if(buttons == MessageBoxButtons::OK)*/
			return MessageBoxResult::OK;
	/*	else
			return MessageBoxResult::Yes;
	}
	else
		return MessageBoxResult::No;*/
}

void Platform::LogDebugMessage(const char* message)
{
	__android_log_write(ANDROID_LOG_INFO, "DEBUG_MESSAGE", message);
}

int Platform::MainLoop() { return 0; }

void Platform::CleanUp()
{
}
