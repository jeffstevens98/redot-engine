/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "register_types.h"

#include "mcp_server.h"
#include "tools/mcp_tools.h"

#include "core/object/class_db.h"

#ifdef TOOLS_ENABLED
#include "editor/mcp_editor_plugin_v2.h"
#endif

void initialize_mcp_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Register core MCP classes
		GDREGISTER_CLASS(MCPServer);
		GDREGISTER_CLASS(MCPTools);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Register editor plugin (new V2 with embedded chat)
		EditorPlugins::add_by_type<MCPEditorPluginV2>();
	}
#endif
}

void uninitialize_mcp_module(ModuleInitializationLevel p_level) {
	// Cleanup if needed
}
