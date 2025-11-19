# Redot MCP Integration

Model Context Protocol (MCP) server implementation for Redot Engine, enabling AI assistants like Claude to interact with your Redot projects.

## What is MCP?

The Model Context Protocol (MCP) is an open standard created by Anthropic that allows AI assistants to securely connect to applications and access their data. This integration brings Claude's AI capabilities directly into your Redot game development workflow.

## Features

This module provides an MCP server that exposes Redot project functionality through the following tools:

### Available Tools

1. **read_file** - Read contents of any file in your project
   - Parameters: `path` (string) - Path to file relative to project root
   - Returns: File contents, size, and metadata

2. **list_scenes** - List all scene files in your project
   - Parameters: `path` (optional) - Directory to search (default: res://)
   - Returns: Array of scene files with paths and names

3. **get_project_settings** - Get project configuration
   - Parameters: None
   - Returns: Project name, main scene, display settings, etc.

4. **search_in_files** - Search for text across project files
   - Parameters:
     - `query` (string) - Text to search for
     - `path` (optional) - Directory to search (default: res://)
     - `extension` (optional) - File extension (default: gd)
   - Returns: Matching files with line numbers and context

5. **list_directory** - List files and directories
   - Parameters:
     - `path` (optional) - Directory path (default: res://)
     - `recursive` (optional) - Search recursively
   - Returns: Files and directories with metadata

6. **get_scene_info** - Analyze scene structure
   - Parameters: `path` (string) - Path to .tscn file
   - Returns: Node tree structure with types and hierarchy

## Architecture

```
┌──────────────────┐         stdio/JSON-RPC         ┌──────────────────┐
│  Claude Desktop  │ ◄────────────────────────────► │   Redot Editor   │
│   (MCP Client)   │                                 │   (MCP Server)   │
└──────────────────┘                                 └──────────────────┘
                                                      ├─ Project Files
                                                      ├─ Scene Tree
                                                      ├─ Scripts
                                                      └─ Resources
```

### Components

- **MCPServer** (`mcp_server.h/cpp`) - Core MCP protocol implementation
  - JSON-RPC 2.0 message handling
  - Capability negotiation
  - Tool and resource management

- **MCPTools** (`tools/mcp_tools.h/cpp`) - Redot-specific tool implementations
  - File system access
  - Scene parsing
  - Project introspection

- **MCPEditorPlugin** (`editor/mcp_editor_plugin.h/cpp`) - Editor integration
  - Automatic server startup
  - Tool registration
  - Stdio transport management

## Building

The MCP module is compiled into the Redot engine. To build:

```bash
# Build Redot with MCP module enabled (default)
scons platform=linuxbsd target=editor

# Explicitly enable/disable MCP module
scons platform=linuxbsd target=editor module_mcp_enabled=yes
```

## Usage with Claude Desktop

1. **Build Redot** with the MCP module
2. **Configure Claude Desktop** to use Redot as an MCP server
3. **Start working** - Claude can now read your scenes, scripts, and project structure!

### Claude Desktop Configuration

Add to your Claude Desktop MCP settings:

```json
{
  "mcpServers": {
    "redot": {
      "command": "/path/to/redot-editor",
      "args": ["--headless", "--mcp"],
      "env": {}
    }
  }
}
```

## Protocol Details

- **Protocol Version**: 2025-03-15
- **Transport**: stdio (standard input/output)
- **Message Format**: JSON-RPC 2.0
- **Capabilities**: tools, resources

## Example Queries for Claude

Once connected, you can ask Claude things like:

- "What scenes are in my project?"
- "Read the player.gd script and explain what it does"
- "Find all uses of 'velocity' in my GDScript files"
- "Show me the node structure of main.tscn"
- "What are my project settings?"

## Extending

To add new tools:

1. Add tool function to `MCPTools` class in `tools/mcp_tools.h/cpp`
2. Register tool in `MCPEditorPlugin::_register_all_tools()`
3. Rebuild Redot

Example:

```cpp
// In mcp_tools.h
static String tool_my_custom_tool(const Dictionary &p_args);

// In mcp_tools.cpp
String MCPTools::tool_my_custom_tool(const Dictionary &p_args) {
    // Your implementation
    Dictionary result;
    result["data"] = "something useful";
    return result.to_json_string();
}

// In mcp_editor_plugin.cpp
void MCPEditorPlugin::_register_all_tools() {
    // ... existing tools ...

    Dictionary schema;
    // Define your schema
    mcp_server->register_tool(
        "my_custom_tool",
        "Description of what it does",
        schema,
        Callable(MCPTools::tool_my_custom_tool));
}
```

## License

This module is part of Redot Engine and follows the same MIT license.

## Contributing

Contributions welcome! Ideas for new tools:
- Animation timeline access
- Shader analysis
- Asset statistics
- Build system integration
- Debugger integration
- Git integration

## Credits

Built for the Redot Engine community with ❤️

MCP Protocol by Anthropic: https://modelcontextprotocol.io
