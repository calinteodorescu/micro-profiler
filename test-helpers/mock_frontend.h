#pragma once

#include <common/module.h>
#include <common/noncopyable.h>
#include <common/primitives.h>
#include <common/protocol.h>
#include <functional>
#include <ipc/endpoint.h>

namespace micro_profiler
{
	namespace tests
	{
		namespace mocks
		{
			typedef containers::unordered_map<unsigned /*threadid*/, statistic_types_t<unsigned>::map_detailed>
				thread_statistics_map;

			class frontend_state : noncopyable, public std::enable_shared_from_this<frontend_state>
			{
			public:
				explicit frontend_state(const std::shared_ptr<void> &ownee = std::shared_ptr<void>());

				std::shared_ptr<ipc::channel> create();

			public:
				std::function<void ()> constructed;
				std::function<void ()> destroyed;

				std::function<void (const initialization_data &id)> initialized;
				std::function<void (const loaded_modules &m)> modules_loaded;
				std::function<void (const thread_statistics_map &u)> updated;
				std::function<void (const unloaded_modules &m)> modules_unloaded;
				std::function<void (unsigned id, const module_info_metadata &md)> metadata_received;
				std::function<void (const std::vector< std::pair<unsigned /*thread_id*/, thread_info> > &threads)> threads_received;

			private:
				std::shared_ptr<void> _ownee;
			};



			template <typename ArchiveT>
			inline void serialize(ArchiveT &a, frontend_state &state, unsigned int /*version*/)
			{
				unsigned metadata_id;
				messages_id c;
				initialization_data id;
				loaded_modules lm;
				thread_statistics_map u;
				unloaded_modules um;
				module_info_metadata md;
				std::vector< std::pair<unsigned /*thread_id*/, thread_info> > threads;

				a(c);
				switch (c)
				{
				case init:
					if (state.initialized)
						a(id), state.initialized(id);
					break;

				case response_modules_loaded:
					if (state.modules_loaded)
						a(lm), state.modules_loaded(lm);
					break;

				case response_statistics_update:
					if (state.updated)
						a(u), state.updated(u);
					break;

				case response_modules_unloaded:
					if (state.modules_unloaded)
						a(um), state.modules_unloaded(um);
					break;

				case response_module_metadata:
					if (state.metadata_received)
						a(metadata_id), a(md), state.metadata_received(metadata_id, md);
					break;

				case response_threads_info:
					if (state.threads_received)
						a(threads), state.threads_received(threads);
					break;

				default:
					break;
				}
			}


			inline frontend_state::frontend_state(const std::shared_ptr<void> &ownee)
				: _ownee(ownee)
			{	}
		}
	}
}
