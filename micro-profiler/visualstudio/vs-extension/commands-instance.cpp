#include "command-ids.h"
#include "helpers.h"

#include <common/string.h>
#include <frontend/file.h>
#include <frontend/frontend_ui.h>
#include <frontend/function_list.h>
#include <frontend/image_patch_ui.h>
#include <frontend/persistence.h>
#include <strmd/serializer.h>
#include <windows.h>
#include <wpl/form.h>
#include <wpl/layout.h>
#include <wpl/vs/command-target.h>
#include <wpl/vs/factory.h>

using namespace std;
using namespace wpl::vs;

namespace micro_profiler
{
	namespace integration
	{
		void init_instance_menu(list< shared_ptr<void> > &running_objects, const wpl::vs::factory &factory,
			command_target &target, const frontend_ui_context &context)
		{
			auto model = context.model;
			auto executable = context.executable;

			target.add_command(cmdidPauseUpdates, [model] (unsigned) {
				model->updates_enabled = false;
			}, false, [model] (unsigned, unsigned &state) {
				return state = (model->updates_enabled ? command_target::enabled : 0) | command_target::supported | command_target::visible, true;
			});

			target.add_command(cmdidResumeUpdates, [model] (unsigned) {
				model->updates_enabled = true;
			}, false, [model] (unsigned, unsigned &state) {
				return state = (model->updates_enabled ? 0 : command_target::enabled) | command_target::supported | command_target::visible, true;
			});

			target.add_command(cmdidSaveStatistics, [model, executable] (unsigned) {
				shared_ptr<functions_list> model2 = model;
				auto_ptr<write_stream> s = create_file(NULL/*get_frame_hwnd(ctx.shell)*/, executable);

				if (s.get())
				{
					strmd::serializer<write_stream, packer> ser(*s);
					snapshot_save<scontext::file_v4>(ser, *model2);
				}
			}, false, [] (unsigned, unsigned &state) {
				return state = command_target::visible | command_target::supported | command_target::enabled, true;
			});

			target.add_command(cmdidClearStatistics, [model] (unsigned) {
				model->clear();
			}, false, [] (unsigned, unsigned &state) {
				return state = command_target::visible | command_target::supported | command_target::enabled, true;
			});

			target.add_command(cmdidCopyStatistics, [model] (unsigned) {
				string result_utf8;

				model->print(result_utf8);

				wstring result = unicode(result_utf8);

				if (::OpenClipboard(NULL))
				{
					if (HGLOBAL gtext = ::GlobalAlloc(GMEM_MOVEABLE, (result.size() + 1) * sizeof(wchar_t)))
					{
						wchar_t *gtext_memory = static_cast<wchar_t *>(::GlobalLock(gtext));

						std::copy(result.c_str(), result.c_str() + result.size() + 1, gtext_memory);
						::GlobalUnlock(gtext_memory);
						::EmptyClipboard();
						::SetClipboardData(CF_UNICODETEXT, gtext);
					}
					::CloseClipboard();
				}
			}, false, [] (unsigned, unsigned &state) {
				return state = command_target::visible | command_target::supported | command_target::enabled, true;
			});

			auto symbols_requested = make_shared<bool>(false);

			target.add_command(cmdidProfileScope, [&running_objects, &factory, context, symbols_requested] (unsigned) {

				wpl::rect_i l = { 0, 0, 400, 300 }; // TODO: Center about form.
				const auto o = make_shared< pair< shared_ptr<wpl::form>, vector<wpl::slot_connection> > >();
				auto &running_objects_ = running_objects;
				const auto i = running_objects.insert(running_objects.end(), o);
				const auto onclose = [i, &running_objects_/*, hshell*/] {
					running_objects_.erase(i);
				};
				const auto root = make_shared<wpl::overlay>();
					root->add(factory.create_control<wpl::control>("background"));
					const auto about = make_shared<image_patch_ui>(factory, context.patches);
					root->add(wpl::pad_control(about, 5, 5));

				o->first = factory.create_modal();
				o->second.push_back(o->first->close += onclose);

				o->first->set_root(root);
				o->first->set_location(l);
				o->first->set_visible(true);

				if (!*symbols_requested)
					context.symbols->request_all_symbols(), *symbols_requested = true;

			}, false, [] (unsigned, unsigned &state) {
				return state = command_target::visible | command_target::supported | command_target::enabled, true;
			});
		}
	}
}
