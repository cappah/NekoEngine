/* NekoEngine
 *
 * EventManager.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine Event Manager
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

#include <System/Logger.h>
#include <Engine/Engine.h>
#include <Engine/EventManager.h>

#define EVT_MGR_MODULE	"EventManager"

using namespace std;

vector<EventHandlers> EventManager::_eventHandlers;

int EventManager::Initialize()
{
	return ENGINE_OK;
}

uint32_t EventManager::RegisterHandler(int32_t id, std::function<void(int32_t, void *)> handler)
{
	EventHandler evtHandler;
	evtHandler.handler = handler;

	for (EventHandlers &eventHandlers : _eventHandlers)
	{
		if (eventHandlers.id != id)
			continue;

		evtHandler.id = eventHandlers.nextHandlerId++;
		eventHandlers.handlers.push_back(evtHandler);

		return evtHandler.id;
	}

	EventHandlers handlers{};
	handlers.id = id;
	handlers.nextHandlerId = 1;
	evtHandler.id = 0;
	handlers.handlers.push_back(evtHandler);

	_eventHandlers.push_back(handlers);

	return evtHandler.id;
}

void EventManager::UnregisterHandler(int32_t id, uint32_t handler)
{
	Logger::Log(EVT_MGR_MODULE, LOG_DEBUG, "Removing handler %d for event %d", handler, id);

	for (EventHandlers &eventHandlers : _eventHandlers)
	{
		if (eventHandlers.id != id)
			continue;

		size_t index;

		for (index = 0; index < eventHandlers.handlers.size(); ++index)
			if ((uint32_t)eventHandlers.handlers[index].id == handler)
				break;
		
		eventHandlers.handlers.erase(eventHandlers.handlers.begin() + index);

		return;
	}
}

void EventManager::Broadcast(int id, void *eventArgs)
{
	for (EventHandlers &eventHandlers : _eventHandlers)
	{
		if (eventHandlers.id != id)
			continue;

		for (EventHandler &handler : eventHandlers.handlers)
			handler.handler(id, eventArgs);

		return;
	}
}

void EventManager::Release()
{
	for (EventHandlers &eventHandlers : _eventHandlers)
		eventHandlers.handlers.clear();
	_eventHandlers.clear();
}
