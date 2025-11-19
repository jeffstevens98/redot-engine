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

// Editor plugin that integrates MCP server into Redot Editor
// Manages stdio transport and tool registration
class MCPEditorPlugin : public EditorPlugin {
	GDCLASS(MCPEditorPlugin, EditorPlugin)

private:
	Ref<MCPServer> mcp_server;
	bool server_enabled = false;

	// For stdio transport
	void _register_all_tools();
	void _process_stdio();

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
