/**************************************************************************/
/*  mcp_editor_plugin_v2.cpp                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "mcp_editor_plugin_v2.h"

#ifdef TOOLS_ENABLED

#include "editor/editor_settings.h"

MCPEditorPluginV2::MCPEditorPluginV2() {
	// Initialize MCP server
	mcp_server.instantiate();
	_register_all_tools();

	// Initialize API client
	api_client.instantiate();

	// Load saved API key
	if (EditorSettings::get_singleton()->has_setting("mcp/claude_api_key")) {
		String saved_key = EditorSettings::get_singleton()->get_setting("mcp/claude_api_key");
		api_client->set_api_key(saved_key);
	}

	if (EditorSettings::get_singleton()->has_setting("mcp/claude_model")) {
		String saved_model = EditorSettings::get_singleton()->get_setting("mcp/claude_model");
		api_client->set_model(saved_model);
	}

	// Create chat panel
	chat_panel = memnew(ClaudeChatPanel);
	chat_panel->set_api_client(api_client);
	chat_panel->set_mcp_server(mcp_server);
	chat_panel->connect("settings_requested", callable_mp(this, &MCPEditorPluginV2::_show_settings));

	// Add to bottom panel (like Output, Debugger tabs)
	add_control_to_bottom_panel(chat_panel, "Claude");

	// Create settings dialog
	settings_dialog = memnew(ClaudeSettingsDialog);
	settings_dialog->connect("settings_changed", callable_mp(this, &MCPEditorPluginV2::_on_settings_changed));
	add_child(settings_dialog);

	// If no API key, show welcome message prompting to configure
	if (!api_client->has_api_key()) {
		chat_panel->add_system_message(
				"[b]Welcome to Claude for Redot![/b]\n\n"
				"To get started, you need to configure your Anthropic API key.\n\n"
				"Click the [b]Settings[/b] button above to enter your API key.");
	}

	print_line("Claude Assistant plugin loaded successfully");
}

MCPEditorPluginV2::~MCPEditorPluginV2() {
	if (chat_panel) {
		remove_control_from_bottom_panel(chat_panel);
		chat_panel->queue_free();
	}
}

void MCPEditorPluginV2::_bind_methods() {
}

void MCPEditorPluginV2::_register_all_tools() {
	// Register all 6 MCP tools

	// read_file
	{
		Dictionary schema;
		Dictionary properties;
		properties["path"] = MCPTools::make_property_string("Path to file (relative to project root)");
		Array required;
		required.push_back("path");
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"read_file",
				"Read contents of a file in the Redot project",
				schema,
				Callable(MCPTools::tool_read_file));
	}

	// list_scenes
	{
		Dictionary schema;
		Dictionary properties;
		properties["path"] = MCPTools::make_property_string("Directory to search (default: res://)");
		Array required;
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"list_scenes",
				"List all scene files in the project",
				schema,
				Callable(MCPTools::tool_list_scenes));
	}

	// get_project_settings
	{
		Dictionary schema;
		Dictionary properties;
		Array required;
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"get_project_settings",
				"Get project settings and configuration",
				schema,
				Callable(MCPTools::tool_get_project_settings));
	}

	// search_in_files
	{
		Dictionary schema;
		Dictionary properties;
		properties["query"] = MCPTools::make_property_string("Text to search for");
		properties["path"] = MCPTools::make_property_string("Directory to search (default: res://)");
		properties["extension"] = MCPTools::make_property_string("File extension (default: gd)");
		Array required;
		required.push_back("query");
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"search_in_files",
				"Search for text in project files",
				schema,
				Callable(MCPTools::tool_search_in_files));
	}

	// list_directory
	{
		Dictionary schema;
		Dictionary properties;
		properties["path"] = MCPTools::make_property_string("Directory path (default: res://)");
		properties["recursive"] = MCPTools::make_property_string("Search recursively (true/false)");
		Array required;
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"list_directory",
				"List files and directories",
				schema,
				Callable(MCPTools::tool_list_directory));
	}

	// get_scene_info
	{
		Dictionary schema;
		Dictionary properties;
		properties["path"] = MCPTools::make_property_string("Path to scene file");
		Array required;
		required.push_back("path");
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"get_scene_info",
				"Get node tree structure and information about a scene",
				schema,
				Callable(MCPTools::tool_get_scene_info));
	}

	print_line("MCP: Registered 6 tools for Claude");
}

void MCPEditorPluginV2::_on_settings_changed(const String &p_api_key, const String &p_model) {
	if (api_client.is_valid()) {
		api_client->set_api_key(p_api_key);
		api_client->set_model(p_model);
	}

	if (chat_panel) {
		chat_panel->set_api_key(p_api_key);
	}

	print_line("Claude settings updated - API key configured: " + String(p_api_key.is_empty() ? "No" : "Yes"));
}

void MCPEditorPluginV2::_show_settings() {
	if (settings_dialog) {
		settings_dialog->popup_centered();
	} else {
		print_line("Settings dialog not initialized!");
	}
}

#endif // TOOLS_ENABLED
