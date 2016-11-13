/* NekoEngine
 *
 * NThreadPool.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Runtime
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

#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <assert.h>
#include <condition_variable>

class NThreadPool
{
public:
	NThreadPool(size_t numWorkers) :
		_stop(false),
		_numWorkers(numWorkers)
	{
		assert(numWorkers > 0);

		_stop = false;
		_workers.resize(_numWorkers);

		for (size_t i = 0; i < _numWorkers; ++i)
		{
			_workers[i] = std::thread([this]()
			{
				while (!this->_stop)
				{
					std::function<void(void)> task;

					{
						std::unique_lock<std::mutex> lock(this->_taskMutex);

						this->_condition.wait(lock, [this] { return this->_stop || !this->_tasks.empty(); });

						if (this->_stop)
							return;

						task = std::move(this->_tasks.front());
						this->_tasks.pop();
					}

					if (task) task();
				}
			});
		}
	}

	size_t GetWorkerCount() { return _numWorkers; }

	void Stop()
	{
		_stop = true;
		_condition.notify_all();
	}

	void Join()
	{
		for (std::thread &worker : _workers)
			if(worker.joinable()) worker.join();
	}

	void Wait()
	{
		while (!_tasks.empty());
	}

	void Enqueue(std::function<void(void)> task)
	{
		this->_taskMutex.lock();
		_tasks.emplace(task);
		this->_taskMutex.unlock();
		_condition.notify_one();
	}

	virtual ~NThreadPool()
	{
		Stop();
		Join();
	}

private:
	std::vector<std::thread> _workers;
	std::queue<std::function<void(void)>> _tasks;
	bool _stop;
	size_t _numWorkers;
	std::mutex _taskMutex;
	std::condition_variable _condition;
};
