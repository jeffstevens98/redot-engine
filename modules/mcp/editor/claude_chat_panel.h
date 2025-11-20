/**************************************************************************/
/*  claude_chat_panel.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "claude_api_client.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/text_edit.h"

class MCPServer; // Forward declaration

// Individual message display
class ChatMessage : public PanelContainer {
	GDCLASS(ChatMessage, PanelContainer)

private:
	RichTextLabel *content_label = nullptr;
	Label *role_label = nullptr;
	bool is_user_message = false;

protected:
	static void _bind_methods() {}

public:
	ChatMessage(const String &p_role, const String &p_content);

	void set_content(const String &p_content);
	void append_content(const String &p_content);
};

// Tool call display
class ToolCallDisplay : public PanelContainer {
	GDCLASS(ToolCallDisplay, PanelContainer)

private:
	Label *tool_name_label = nullptr;
	RichTextLabel *input_label = nullptr;
	RichTextLabel *output_label = nullptr;
	bool is_executing = true;

protected:
	static void _bind_methods() {}

public:
	ToolCallDisplay(const String &p_tool_name, const Dictionary &p_input);

	void set_result(const String &p_output);
	void set_error(const String &p_error);
};

// Main chat panel - the "Claude Code" interface for Redot
class ClaudeChatPanel : public VBoxContainer {
	GDCLASS(ClaudeChatPanel, VBoxContainer)

private:
	// UI Components
	HBoxContainer *header = nullptr;
	Label *status_label = nullptr;
	Button *settings_button = nullptr;
	Button *clear_button = nullptr;

	ScrollContainer *messages_scroll = nullptr;
	VBoxContainer *messages_container = nullptr;

	HBoxContainer *input_container = nullptr;
	TextEdit *input_box = nullptr;
	Button *send_button = nullptr;

	// Backend
	Ref<ClaudeAPIClient> api_client;
	Ref<MCPServer> mcp_server;

	Array message_history;
	Array available_tools;

	bool is_processing = false;

	// UI Updates
	void _add_message(const String &p_role, const String &p_content);
	void _add_tool_call(const String &p_tool_name, const Dictionary &p_input, const String &p_result);
	void _update_status(const String &p_status);
	void _scroll_to_bottom();

	// Event handlers
	void _on_send_pressed();
	void _on_clear_pressed();
	void _on_settings_pressed();
	void _on_input_text_changed();
	void _on_input_gui_input(const Ref<InputEvent> &p_event);

	// Claude interaction
	void _process_claude_response();
	void _execute_tool(const String &p_tool_name, const Dictionary &p_input, String &r_result);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	ClaudeChatPanel();
	~ClaudeChatPanel();

	void set_api_client(Ref<ClaudeAPIClient> p_client);
	void set_mcp_server(Ref<MCPServer> p_server);
	void set_api_key(const String &p_key);

	void add_system_message(const String &p_message);
};

#endif // TOOLS_ENABLED
