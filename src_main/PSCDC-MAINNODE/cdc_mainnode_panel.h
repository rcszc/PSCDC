// cdc_mainnode_panel. RCSZ.

#ifndef _CDC_MAINNODE_PANEL_H
#define _CDC_MAINNODE_PANEL_H
#include <functional>

#include "cdc_mainnode_net.h"

#define PANEL_INFOHEAD "[PANEL]: "
#define TEXT_FILE_LENGTH 5000 // view code panel char_max.

std::string __get_current_time();
#define TIEM_CURRENT __get_current_time()

/*
* @param const char*           (window title)
* @param uint32_t uint32_t     (window width(x), height(y))
* @param std::function<void()> (draw gui function)
*/
void opengl_panel_init(
	const char* window_title,
	uint32_t window_szx,
	uint32_t window_szy,
	std::function<void()> render = {}
);

namespace GLFW_IMGUI_PANEL {
	void draw_imgui_panel();
}

#endif