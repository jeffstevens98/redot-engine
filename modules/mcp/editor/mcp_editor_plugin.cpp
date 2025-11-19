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

MCPEditorPlugin::MCPEditorPlugin() {
	mcp_server.instantiate();
}

MCPEditorPlugin::~MCPEditorPlugin() {
	if (server_enabled) {
		disable_server();
	}
}

void MCPEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("enable_server"), &MCPEditorPlugin::enable_server);
	ClassDB::bind_method(D_METHOD("disable_server"), &MCPEditorPlugin::disable_server);
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
				Callable(MCPTools::tool_read_file));
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
				Callable(MCPTools::tool_list_scenes));
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
				Callable(MCPTools::tool_get_project_settings));
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
				Callable(MCPTools::tool_search_in_files));
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
				Callable(MCPTools::tool_list_directory));
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
				Callable(MCPTools::tool_get_scene_info));
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
	print_line("MCP: Server stopped");
}

#endif // TOOLS_ENABLED
