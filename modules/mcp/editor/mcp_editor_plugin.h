/**************************************************************************/
/*  mcp_editor_plugin.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "../mcp_server.h"
#include "../tools/mcp_tools.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/rich_text_label.h"

// MCP Control Panel UI
class MCPControlPanel : public PanelContainer {
	GDCLASS(MCPControlPanel, PanelContainer)

private:
	Label *status_label = nullptr;
	Button *toggle_button = nullptr;
	RichTextLabel *info_label = nullptr;
	Label *tools_label = nullptr;

	bool server_running = false;

	void _on_toggle_pressed();

protected:
	static void _bind_methods();

public:
	MCPControlPanel();
	void update_status(bool p_running, int p_tool_count);

	Signal signal_toggle_server;
};

// Editor plugin that integrates MCP server into Redot Editor
// Manages stdio transport and tool registration
class MCPEditorPlugin : public EditorPlugin {
	GDCLASS(MCPEditorPlugin, EditorPlugin)

private:
	Ref<MCPServer> mcp_server;
	bool server_enabled = false;
	MCPControlPanel *control_panel = nullptr;

	// For stdio transport
	void _register_all_tools();
	void _process_stdio();
	void _on_toggle_server();

protected:
	static void _bind_methods();

public:
	MCPEditorPlugin();
	~MCPEditorPlugin();

	virtual String get_plugin_name() const override { return "MCPServer"; }

	void enable_server();
	void disable_server();
};

#endif // TOOLS_ENABLED
