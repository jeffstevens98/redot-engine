/**************************************************************************/
/*  claude_chat_panel.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "claude_chat_panel.h"

#ifdef TOOLS_ENABLED

#include "../mcp_server.h"
#include "../tools/mcp_tools.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"

// ========== ChatMessage ==========

ChatMessage::ChatMessage(const String &p_role, const String &p_content) {
	is_user_message = (p_role == "user");

	// Style based on role
	if (is_user_message) {
		add_theme_stylebox_override("panel", memnew(StyleBoxFlat));
		// User messages: light blue background
	} else {
		add_theme_stylebox_override("panel", memnew(StyleBoxFlat));
		// Assistant messages: light gray background
	}

	VBoxContainer *vbox = memnew(VBoxContainer);
	add_child(vbox);

	MarginContainer *margin = memnew(MarginContainer);
	margin->add_theme_constant_override("margin_left", 10);
	margin->add_theme_constant_override("margin_right", 10);
	margin->add_theme_constant_override("margin_top", 8);
	margin->add_theme_constant_override("margin_bottom", 8);
	vbox->add_child(margin);

	VBoxContainer *content_vbox = memnew(VBoxContainer);
	margin->add_child(content_vbox);

	// Role label
	role_label = memnew(Label);
	role_label->set_text(p_role == "user" ? "You" : "Claude");
	role_label->add_theme_font_size_override("font_size", 12);
	content_vbox->add_child(role_label);

	// Content
	content_label = memnew(RichTextLabel);
	content_label->set_use_bbcode(true);
	content_label->set_fit_content(true);
	content_label->set_selection_enabled(true);
	content_label->set_text(p_content);
	content_vbox->add_child(content_label);
}

void ChatMessage::set_content(const String &p_content) {
	content_label->set_text(p_content);
}

void ChatMessage::append_content(const String &p_content) {
	String current = content_label->get_text();
	content_label->set_text(current + p_content);
}

// ========== ToolCallDisplay ==========

ToolCallDisplay::ToolCallDisplay(const String &p_tool_name, const Dictionary &p_input) {
	set_custom_minimum_size(Size2(0, 80));

	VBoxContainer *vbox = memnew(VBoxContainer);
	add_child(vbox);

	MarginContainer *margin = memnew(MarginContainer);
	margin->add_theme_constant_override("margin_left", 20);
	margin->add_theme_constant_override("margin_right", 10);
	margin->add_theme_constant_override("margin_top", 5);
	margin->add_theme_constant_override("margin_bottom", 5);
	vbox->add_child(margin);

	VBoxContainer *content = memnew(VBoxContainer);
	margin->add_child(content);

	// Tool name
	tool_name_label = memnew(Label);
	tool_name_label->set_text("🔧 Tool: " + p_tool_name);
	tool_name_label->add_theme_font_size_override("font_size", 11);
	content->add_child(tool_name_label);

	// Input
	input_label = memnew(RichTextLabel);
	input_label->set_use_bbcode(true);
	input_label->set_fit_content(true);
	input_label->set_custom_minimum_size(Size2(0, 30));
	input_label->set_text("[i]Input:[/i] " + JSON::stringify(p_input, "  ", false));
	content->add_child(input_label);

	// Output (will be set later)
	output_label = memnew(RichTextLabel);
	output_label->set_use_bbcode(true);
	output_label->set_fit_content(true);
	output_label->set_custom_minimum_size(Size2(0, 30));
	output_label->set_text("[i]Executing...[/i]");
	content->add_child(output_label);
}

void ToolCallDisplay::set_result(const String &p_output) {
	is_executing = false;
	output_label->set_text("[i]Result:[/i]\n" + p_output);
}

void ToolCallDisplay::set_error(const String &p_error) {
	is_executing = false;
	output_label->set_text("[color=red][i]Error:[/i] " + p_error + "[/color]");
}

// ========== ClaudeChatPanel ==========

ClaudeChatPanel::ClaudeChatPanel() {
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_h_size_flags(SIZE_EXPAND_FILL);

	// Header
	header = memnew(HBoxContainer);
	add_child(header);

	Label *title = memnew(Label);
	title->set_text("Claude Assistant");
	title->add_theme_font_size_override("font_size", 16);
	title->set_h_size_flags(SIZE_EXPAND_FILL);
	header->add_child(title);

	status_label = memnew(Label);
	status_label->set_text("● Ready");
	status_label->add_theme_color_override("font_color", Color(0.3, 0.8, 0.3));
	header->add_child(status_label);

	settings_button = memnew(Button);
	settings_button->set_text("Settings");
	settings_button->connect("pressed", callable_mp(this, &ClaudeChatPanel::_on_settings_pressed));
	header->add_child(settings_button);

	clear_button = memnew(Button);
	clear_button->set_text("Clear");
	clear_button->connect("pressed", callable_mp(this, &ClaudeChatPanel::_on_clear_pressed));
	header->add_child(clear_button);

	add_child(memnew(HSeparator));

	// Messages area
	messages_scroll = memnew(ScrollContainer);
	messages_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
	messages_scroll->set_h_size_flags(SIZE_EXPAND_FILL);
	messages_scroll->set_custom_minimum_size(Size2(0, 400));
	add_child(messages_scroll);

	messages_container = memnew(VBoxContainer);
	messages_container->set_h_size_flags(SIZE_EXPAND_FILL);
	messages_scroll->add_child(messages_container);

	add_child(memnew(HSeparator));

	// Input area
	input_container = memnew(HBoxContainer);
	add_child(input_container);

	input_box = memnew(TextEdit);
	input_box->set_custom_minimum_size(Size2(0, 80));
	input_box->set_h_size_flags(SIZE_EXPAND_FILL);
	input_box->set_placeholder("Ask Claude about your Redot project...");
	input_box->connect("text_changed", callable_mp(this, &ClaudeChatPanel::_on_input_text_changed));
	input_container->add_child(input_box);

	send_button = memnew(Button);
	send_button->set_text("Send");
	send_button->set_custom_minimum_size(Size2(80, 0));
	send_button->connect("pressed", callable_mp(this, &ClaudeChatPanel::_on_send_pressed));
	input_container->add_child(send_button);

	// Initialize API client
	api_client.instantiate();

	// Show welcome message
	add_system_message(
			"Welcome to Claude for Redot! 👋\n\n"
			"I'm your AI assistant for game development. I can help you:\n"
			"• Understand your project structure\n"
			"• Read and analyze your scripts\n"
			"• Search through your code\n"
			"• Explain scene hierarchies\n"
			"• Suggest improvements\n\n"
			"[i]Configure your Anthropic API key in Settings to get started.[/i]");
}

ClaudeChatPanel::~ClaudeChatPanel() {
}

void ClaudeChatPanel::_bind_methods() {
}

void ClaudeChatPanel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Setup complete
		} break;
	}
}

void ClaudeChatPanel::set_api_client(Ref<ClaudeAPIClient> p_client) {
	api_client = p_client;
}

void ClaudeChatPanel::set_mcp_server(Ref<MCPServer> p_server) {
	mcp_server = p_server;
}

void ClaudeChatPanel::set_api_key(const String &p_key) {
	if (api_client.is_valid()) {
		api_client->set_api_key(p_key);
		if (api_client->has_api_key()) {
			add_system_message("✓ API key configured. You can now chat with Claude!");
			_update_status("Ready");
		}
	}
}

void ClaudeChatPanel::_add_message(const String &p_role, const String &p_content) {
	ChatMessage *msg = memnew(ChatMessage(p_role, p_content));
	messages_container->add_child(msg);

	// Add to history
	Dictionary msg_dict;
	msg_dict["role"] = p_role;
	msg_dict["content"] = p_content;
	message_history.push_back(msg_dict);

	call_deferred("_scroll_to_bottom");
}

void ClaudeChatPanel::_add_tool_call(const String &p_tool_name, const Dictionary &p_input, const String &p_result) {
	ToolCallDisplay *tool_display = memnew(ToolCallDisplay(p_tool_name, p_input));
	tool_display->set_result(p_result);
	messages_container->add_child(tool_display);

	call_deferred("_scroll_to_bottom");
}

void ClaudeChatPanel::add_system_message(const String &p_message) {
	ChatMessage *msg = memnew(ChatMessage("system", p_message));
	messages_container->add_child(msg);
	call_deferred("_scroll_to_bottom");
}

void ClaudeChatPanel::_update_status(const String &p_status) {
	status_label->set_text("● " + p_status);

	if (p_status == "Processing...") {
		status_label->add_theme_color_override("font_color", Color(0.8, 0.6, 0.2));
	} else if (p_status == "Ready") {
		status_label->add_theme_color_override("font_color", Color(0.3, 0.8, 0.3));
	} else {
		status_label->add_theme_color_override("font_color", Color(0.8, 0.3, 0.3));
	}
}

void ClaudeChatPanel::_scroll_to_bottom() {
	if (messages_scroll) {
		messages_scroll->set_v_scroll(messages_scroll->get_v_scroll_bar()->get_max());
	}
}

void ClaudeChatPanel::_on_send_pressed() {
	String text = input_box->get_text().strip_edges();

	if (text.is_empty()) {
		return;
	}

	if (!api_client.is_valid() || !api_client->has_api_key()) {
		add_system_message("[color=red]Please configure your API key in Settings first.[/color]");
		return;
	}

	// Add user message
	_add_message("user", text);
	input_box->set_text("");
	is_processing = true;
	send_button->set_disabled(true);
	_update_status("Processing...");

	// Process response (will be implemented with actual API call)
	call_deferred("_process_claude_response");
}

void ClaudeChatPanel::_on_clear_pressed() {
	// Clear all messages except welcome
	for (int i = messages_container->get_child_count() - 1; i >= 0; i--) {
		Node *child = messages_container->get_child(i);
		child->queue_free();
	}

	message_history.clear();

	add_system_message(
			"Conversation cleared.\n\n"
			"[i]Start a new conversation by typing your question below.[/i]");
}

void ClaudeChatPanel::_on_settings_pressed() {
	// TODO: Open settings dialog
	add_system_message("[i]Settings dialog will be implemented here.[/i]");
}

void ClaudeChatPanel::_on_input_text_changed() {
	// Enable/disable send button based on input
	send_button->set_disabled(input_box->get_text().strip_edges().is_empty());
}

void ClaudeChatPanel::_process_claude_response() {
	if (!mcp_server.is_valid()) {
		add_system_message("[color=red]MCP server not initialized.[/color]");
		is_processing = false;
		send_button->set_disabled(false);
		_update_status("Error");
		return;
	}

	// Build tools array from MCP server
	// For now, show a placeholder response
	String response_text = "I received your message! The actual Claude API integration is in progress.\n\n"
						   "[i]When complete, I'll be able to:\n"
						   "• Read your project files\n"
						   "• Search through code\n"
						   "• Analyze scenes\n"
						   "• And much more![/i]";

	_add_message("assistant", response_text);

	is_processing = false;
	send_button->set_disabled(false);
	_update_status("Ready");
}

void ClaudeChatPanel::_execute_tool(const String &p_tool_name, const Dictionary &p_input, String &r_result) {
	// Execute MCP tool
	if (p_tool_name == "read_file") {
		r_result = MCPTools::tool_read_file(p_input);
	} else if (p_tool_name == "list_scenes") {
		r_result = MCPTools::tool_list_scenes(p_input);
	} else if (p_tool_name == "search_in_files") {
		r_result = MCPTools::tool_search_in_files(p_input);
	} else if (p_tool_name == "get_project_settings") {
		r_result = MCPTools::tool_get_project_settings(p_input);
	} else if (p_tool_name == "list_directory") {
		r_result = MCPTools::tool_list_directory(p_input);
	} else if (p_tool_name == "get_scene_info") {
		r_result = MCPTools::tool_get_scene_info(p_input);
	} else {
		r_result = "{\"error\": \"Unknown tool: " + p_tool_name + "\"}";
	}
}

#endif // TOOLS_ENABLED
