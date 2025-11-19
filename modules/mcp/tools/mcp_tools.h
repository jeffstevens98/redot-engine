/**************************************************************************/
/*  mcp_tools.h                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#include "core/io/dir_access.h"
#include "core/object/ref_counted.h"
#include "core/variant/variant.h"

// Collection of MCP tools for Redot Engine
// These tools expose Redot project functionality to MCP clients
class MCPTools : public Object {
	GDCLASS(MCPTools, Object)

public:
	// Read file contents from the project
	static String tool_read_file(const Dictionary &p_args);

	// List all scenes in the project
	static String tool_list_scenes(const Dictionary &p_args);

	// Get project settings
	static String tool_get_project_settings(const Dictionary &p_args);

	// Search for text in project files
	static String tool_search_in_files(const Dictionary &p_args);

	// List all files in a directory
	static String tool_list_directory(const Dictionary &p_args);

	// Get scene tree structure for a scene file
	static String tool_get_scene_info(const Dictionary &p_args);

	// Helper to create JSON schema for tool parameters
	static Dictionary make_schema_object(const Array &p_required, const Dictionary &p_properties);
	static Dictionary make_property_string(const String &p_description);

private:
	// Recursive helper functions
	static void _list_scenes_recursive(Ref<DirAccess> p_dir, const String &p_path, Array &r_scenes);
	static void _search_in_files_recursive(Ref<DirAccess> p_dir, const String &p_path, const String &p_query, const String &p_extension, Array &r_results);
	static void _list_directory_recursive(Ref<DirAccess> p_dir, const String &p_path, Array &r_files, Array &r_directories);
};
