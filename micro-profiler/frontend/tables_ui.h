#pragma once

#include "containers.h"
#include "piechart.h"

#include <memory>
#include <vector>
#include <wpl/ui/listview.h>
#include <wpl/ui/container.h>

namespace micro_profiler
{
	class columns_model;
	class functions_list;
	struct hive;
	struct linked_statistics;

	class tables_ui : wpl::noncopyable, public container_with_background
	{
	public:
		tables_ui(const std::shared_ptr<functions_list> &model, hive &configuration);

		void save(hive &configuration);

	private:
		void on_selection_change(wpl::ui::listview::index_type index, bool selected);
		void on_piechart_selection_change(piechart::index_type index);

		void on_drilldown(const std::shared_ptr<linked_statistics> &view, wpl::ui::listview::index_type index);
		void on_children_selection_change(wpl::ui::listview::index_type index, bool selected);
		void on_children_piechart_selection_change(piechart::index_type index);

		void switch_linked(wpl::ui::table_model::index_type index);

	private:
		const std::shared_ptr<columns_model> _columns_main;
		const std::shared_ptr<functions_list> _statistics;
		const std::shared_ptr<wpl::ui::listview> _statistics_lv;
		const std::shared_ptr<piechart> _statistics_pc;

		const std::shared_ptr<columns_model> _columns_parents;
		std::shared_ptr<linked_statistics> _parents_statistics;
		const std::shared_ptr<wpl::ui::listview> _parents_lv;

		const std::shared_ptr<columns_model> _columns_children;
		std::shared_ptr<linked_statistics> _children_statistics;
		const std::shared_ptr<wpl::ui::listview> _children_lv;
		const std::shared_ptr<piechart> _children_pc;

		std::vector<wpl::slot_connection> _connections;
	};
}
