/**************************************************************************/
/*  mcp_editor_plugin.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "mcp_editor_plugin.h"

#ifdef TOOLS_ENABLED

#include "core/io/json.h"
#include "core/os/os.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"

// ========== MCPControlPanel ==========

MCPControlPanel::MCPControlPanel() {
	set_custom_minimum_size(Size2(300, 0));

	VBoxContainer *vbox = memnew(VBoxContainer);
	add_child(vbox);

	MarginContainer *margin = memnew(MarginContainer);
	margin->add_theme_constant_override("margin_left", 10);
	margin->add_theme_constant_override("margin_right", 10);
	margin->add_theme_constant_override("margin_top", 10);
	margin->add_theme_constant_override("margin_bottom", 10);
	vbox->add_child(margin);

	VBoxContainer *content = memnew(VBoxContainer);
	content->set_v_size_flags(SIZE_EXPAND_FILL);
	margin->add_child(content);

	// Title
	Label *title = memnew(Label);
	title->set_text("Model Context Protocol (MCP)");
	title->add_theme_font_size_override("font_size", 16);
	content->add_child(title);

	content->add_child(memnew(HSeparator));

	// Status
	status_label = memnew(Label);
	status_label->set_text("Status: Stopped");
	content->add_child(status_label);

	// Tools count
	tools_label = memnew(Label);
	tools_label->set_text("Registered tools: 0");
	content->add_child(tools_label);

	content->add_child(memnew(HSeparator));

	// Toggle button
	toggle_button = memnew(Button);
	toggle_button->set_text("Start MCP Server");
	toggle_button->connect("pressed", callable_mp(this, &MCPControlPanel::_on_toggle_pressed));
	content->add_child(toggle_button);

	content->add_child(memnew(HSeparator));

	// Info
	info_label = memnew(RichTextLabel);
	info_label->set_custom_minimum_size(Size2(0, 150));
	info_label->set_use_bbcode(true);
	info_label->set_text(
			"[b]MCP Server Status[/b]\n\n"
			"The MCP server allows AI assistants like Claude to connect to this Redot project and access:\n\n"
			"• Project files and scenes\n"
			"• GDScript code\n"
			"• Project settings\n"
			"• Scene node trees\n\n"
			"[i]Start the server to enable MCP connections.[/i]");
	content->add_child(info_label);
}

void MCPControlPanel::_bind_methods() {
	ADD_SIGNAL(MethodInfo("toggle_server"));
}

void MCPControlPanel::_on_toggle_pressed() {
	emit_signal("toggle_server");
}

void MCPControlPanel::update_status(bool p_running, int p_tool_count) {
	server_running = p_running;

	if (p_running) {
		status_label->set_text("Status: ✓ Running");
		toggle_button->set_text("Stop MCP Server");
		info_label->set_text(
				"[b]MCP Server Running[/b]\n\n"
				"[color=green]Server is active and ready for connections.[/color]\n\n"
				"Protocol: JSON-RPC 2.0\n"
				"Transport: stdio\n"
				"Version: 2025-03-15\n\n"
				"Claude Desktop or other MCP clients can now connect to this Redot instance.");
	} else {
		status_label->set_text("Status: Stopped");
		toggle_button->set_text("Start MCP Server");
		info_label->set_text(
				"[b]MCP Server Stopped[/b]\n\n"
				"The MCP server allows AI assistants like Claude to connect to this Redot project.\n\n"
				"[i]Start the server to enable MCP connections.[/i]");
	}

	tools_label->set_text("Registered tools: " + itos(p_tool_count));
}

// ========== MCPEditorPlugin ==========

MCPEditorPlugin::MCPEditorPlugin() {
	mcp_server.instantiate();

	// Create control panel
	control_panel = memnew(MCPControlPanel);
	control_panel->connect("toggle_server", callable_mp(this, &MCPEditorPlugin::_on_toggle_server));

	// Add to bottom panel
	add_control_to_bottom_panel(control_panel, "MCP");

	// Auto-start server (optional - can be changed to manual)
	// enable_server();
}

MCPEditorPlugin::~MCPEditorPlugin() {
	if (server_enabled) {
		disable_server();
	}

	if (control_panel) {
		remove_control_from_bottom_panel(control_panel);
		control_panel->queue_free();
	}
}

void MCPEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("enable_server"), &MCPEditorPlugin::enable_server);
	ClassDB::bind_method(D_METHOD("disable_server"), &MCPEditorPlugin::disable_server);
}

void MCPEditorPlugin::_on_toggle_server() {
	if (server_enabled) {
		disable_server();
	} else {
		enable_server();
	}
}

void MCPEditorPlugin::_register_all_tools() {
	// Register read_file tool
	{
		Dictionary schema;
		Dictionary properties;
		properties["path"] = MCPTools::make_property_string("Path to the file to read (relative to project root)");
		Array required;
		required.push_back("path");
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"read_file",
				"Read contents of a file in the Redot project",
				schema,
				callable_mp_static(&MCPTools::tool_read_file));
	}

	// Register list_scenes tool
	{
		Dictionary schema;
		Dictionary properties;
		properties["path"] = MCPTools::make_property_string("Directory path to search (default: res://)");
		Array required; // No required params
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"list_scenes",
				"List all scene files (.tscn, .scn) in the project",
				schema,
				callable_mp_static(&MCPTools::tool_list_scenes));
	}

	// Register get_project_settings tool
	{
		Dictionary schema;
		Dictionary properties; // No params needed
		Array required;
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"get_project_settings",
				"Get project settings and configuration",
				schema,
				callable_mp_static(&MCPTools::tool_get_project_settings));
	}

	// Register search_in_files tool
	{
		Dictionary schema;
		Dictionary properties;
		properties["query"] = MCPTools::make_property_string("Text to search for");
		properties["path"] = MCPTools::make_property_string("Directory to search in (default: res://)");
		properties["extension"] = MCPTools::make_property_string("File extension to search (default: gd)");
		Array required;
		required.push_back("query");
		schema = MCPTools::make_schema_object(required, properties);

		mcp_server->register_tool(
				"search_in_files",
				"Search for text in project files",
				schema,
				callable_mp_static(&MCPTools::tool_search_in_files));
	}

	// Register list_directory tool
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

	// Register get_scene_info tool
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

	print_line("MCP: Registered " + itos(6) + " tools");
}

void MCPEditorPlugin::enable_server() {
	if (server_enabled) {
		return;
	}

	print_line("MCP: Starting Model Context Protocol server...");
	print_line("MCP: Protocol version: " + mcp_server->get_protocol_version());

	_register_all_tools();
	mcp_server->start();

	server_enabled = true;

	// Update GUI
	if (control_panel) {
		control_panel->update_status(true, 6); // 6 tools registered
	}

	print_line("MCP: Server started successfully");
	print_line("MCP: Ready to accept connections via stdio");
}

void MCPEditorPlugin::disable_server() {
	if (!server_enabled) {
		return;
	}

	print_line("MCP: Stopping server...");
	mcp_server->stop();
	server_enabled = false;

	// Update GUI
	if (control_panel) {
		control_panel->update_status(false, 0);
	}

	print_line("MCP: Server stopped");
}

#endif // TOOLS_ENABLED
