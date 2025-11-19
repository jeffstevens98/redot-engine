/**************************************************************************/
/*  mcp_editor_plugin_v2.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "../mcp_server.h"
#include "../tools/mcp_tools.h"
#include "claude_api_client.h"
#include "claude_chat_panel.h"
#include "claude_settings_dialog.h"
#include "editor/editor_plugin.h"

// New MCP Editor Plugin with embedded Claude chat interface
// This creates a "Claude Code for Redot" experience
class MCPEditorPluginV2 : public EditorPlugin {
	GDCLASS(MCPEditorPluginV2, EditorPlugin)

private:
	Ref<MCPServer> mcp_server;
	Ref<ClaudeAPIClient> api_client;

	ClaudeChatPanel *chat_panel = nullptr;
	ClaudeSettingsDialog *settings_dialog = nullptr;

	void _register_all_tools();
	void _on_settings_changed(const String &p_api_key, const String &p_model);
	void _show_settings();

protected:
	static void _bind_methods();

public:
	MCPEditorPluginV2();
	~MCPEditorPluginV2();

	virtual String get_plugin_name() const override { return "Claude Assistant"; }
	virtual bool has_main_screen() const override { return false; }
};

#endif // TOOLS_ENABLED
