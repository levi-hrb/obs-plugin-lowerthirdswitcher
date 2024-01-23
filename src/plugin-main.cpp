#include <lowerthirdswitcher-widget.hpp>
#include "plugin-macros.generated.h"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Herber")

LowerthirdswitcherDockWidget *lowerthirdswitcherDockWidget = nullptr;

bool obs_module_load(void) {
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	lowerthirdswitcherDockWidget = new LowerthirdswitcherDockWidget(main_window);

	obs_frontend_add_dock(lowerthirdswitcherDockWidget);

	blog(LOG_INFO, "LowerThirdSwitcher plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload() {
	blog(LOG_INFO, "LowerThirdSwitcher plugin unloaded");
}

const char *obs_module_name(void) {
	return "Lower Third Switcher";
}

const char *obs_module_description(void) {
	return "Lower Third Switcher Description";
}
