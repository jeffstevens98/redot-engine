/**************************************************************************/
/*  mcp_editor_plugin_v2.cpp                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "mcp_editor_plugin_v2.h"

#ifdef TOOLS_ENABLED

#include "editor/settings/editor_settings.h"

MCPEditorPluginV2::MCPEditorPluginV2() {
	// Register settings
	if (!EditorSettings::get_singleton()->has_setting("mcp/enable_claude_assistant")) {
		EditorSettings::get_singleton()->set_setting("mcp/enable_claude_assistant", true);
	}
	EditorSettings::get_singleton()->set_initial_value("mcp/enable_claude_assistant", true, true);
	EditorSettings::get_singleton()->add_property_hint(PropertyInfo(Variant::BOOL, "mcp/enable_claude_assistant", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT));

	// Register panel location setting
	if (!EditorSettings::get_singleton()->has_setting("mcp/claude_panel_location")) {
		EditorSettings::get_singleton()->set_setting("mcp/claude_panel_location", 0); // 0 = Bottom, 1 = Right
	}
	EditorSettings::get_singleton()->set_initial_value("mcp/claude_panel_location", 0, true);
	EditorSettings::get_singleton()->add_property_hint(PropertyInfo(Variant::INT, "mcp/claude_panel_location", PROPERTY_HINT_ENUM, "Bottom Panel,Right Dock", PROPERTY_USAGE_DEFAULT));

	// Check if plugin is enabled
	plugin_enabled = EditorSettings::get_singleton()->get_setting("mcp/enable_claude_assistant");

	if (!plugin_enabled) {
		print_line("Claude Assistant plugin is disabled in Editor Settings (mcp/enable_claude_assistant)");
		return;
	}

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

	// Add to UI based on user preference
	int panel_location = EditorSettings::get_singleton()->get_setting("mcp/claude_panel_location");
	if (panel_location == 0) {
		// Bottom panel (like Output, Debugger tabs)
		bottom_panel_button = add_control_to_bottom_panel(chat_panel, "Claude");
		use_bottom_panel = true;
	} else {
		// Right dock (like Inspector, Scene panels)
		add_control_to_dock(DOCK_SLOT_RIGHT_UL, chat_panel);
		use_bottom_panel = false;
	}

	// Create settings dialog
	settings_dialog = memnew(ClaudeSettingsDialog);
	settings_dialog->connect("settings_changed", callable_mp(this, &MCPEditorPluginV2::_on_settings_changed));
	add_child(settings_dialog);

	// Add menu item to Tools menu for easy access
	add_tool_menu_item("Toggle Claude Assistant", callable_mp(this, &MCPEditorPluginV2::_toggle_claude_panel));

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
	if (plugin_enabled) {
		// Remove menu item
		remove_tool_menu_item("Toggle Claude Assistant");

		// Remove panel from wherever it was added
		if (chat_panel) {
			if (use_bottom_panel) {
				remove_control_from_bottom_panel(chat_panel);
			} else {
				remove_control_from_dock(chat_panel);
			}
			chat_panel->queue_free();
		}
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
				callable_mp_static(&MCPTools::tool_read_file));
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
				callable_mp_static(&MCPTools::tool_list_scenes));
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
				callable_mp_static(&MCPTools::tool_get_project_settings));
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
				callable_mp_static(&MCPTools::tool_search_in_files));
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
				callable_mp_static(&MCPTools::tool_list_directory));
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
				callable_mp_static(&MCPTools::tool_get_scene_info));
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

void MCPEditorPluginV2::_toggle_claude_panel() {
	if (!plugin_enabled || !chat_panel) {
		return;
	}

	// Toggle visibility based on panel location
	if (use_bottom_panel) {
		make_bottom_panel_item_visible(chat_panel);
	} else {
		// For dock, just show/hide the control
		chat_panel->set_visible(!chat_panel->is_visible());
	}
}

void MCPEditorPluginV2::_enable_plugin() {
	// Called when plugin is enabled via Editor Settings
	plugin_enabled = true;
	print_line("Claude Assistant plugin enabled");
}

void MCPEditorPluginV2::_disable_plugin() {
	// Called when plugin is disabled via Editor Settings
	plugin_enabled = false;
	print_line("Claude Assistant plugin disabled");
}

#endif // TOOLS_ENABLED
