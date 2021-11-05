//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include "endpoint.h"

#include <common/noncopyable.h>

namespace scheduler
{
	struct queue;
}

namespace micro_profiler
{
	namespace ipc
	{
		class lifetime;

		class marshalled_server : public server, noncopyable
		{
		public:
			marshalled_server(std::shared_ptr<server> underlying, scheduler::queue &apartment_queue);
			~marshalled_server();

			void stop();

		private:
			virtual channel_ptr_t create_session(channel &outbound) override;

		private:
			const std::shared_ptr<lifetime> _lifetime;
			std::shared_ptr<server> _underlying;
			scheduler::queue &_apartment_queue;
		};
	}
}
