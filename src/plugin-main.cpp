/*
Lower Third Switcher Plugin
Copyright (C) 2025 Levi Herber <levisamuel.hrb@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include "lowerthirdswitcher-widget.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

LowerthirdswitcherDockWidget *lowerthirdswitcherDockWidget = nullptr;

bool obs_module_load(void)
{
	const auto main_window =
		static_cast<QMainWindow *>(obs_frontend_get_main_window());
	lowerthirdswitcherDockWidget =
		new LowerthirdswitcherDockWidget(main_window);

	obs_frontend_add_dock_by_id("LowerThirdSwitcherDock", "Lower Third Switcher", 
				     lowerthirdswitcherDockWidget);

	obs_log(LOG_INFO,
	     "LowerThirdSwitcher plugin loaded successfully (version %s)",
	     PLUGIN_VERSION);
	return true;
}

void obs_module_unload()
{
	obs_log(LOG_INFO, "LowerThirdSwitcher plugin unloaded");
}
