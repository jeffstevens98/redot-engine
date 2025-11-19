# How the Redot MCP Integration Works

## 📚 Complete Guide to Connecting Claude Desktop with Redot

This document explains in detail how the MCP integration works and how to connect Claude Desktop to your Redot projects.

---

## 🧠 What is This?

This integration creates a **bridge** between:
- **Redot Engine** (your game development environment)
- **Claude Desktop** (AI assistant)

Using the **Model Context Protocol (MCP)**, Claude can "see" and understand your Redot project files, scenes, and scripts in real-time!

---

## 🏗️ Architecture Overview

### The Big Picture

```
┌─────────────────────────────────────┐
│      Claude Desktop App             │
│  (MCP Client - AI Assistant)        │
└────────────┬────────────────────────┘
             │
             │ JSON-RPC 2.0 Messages
             │ via stdio
             │ (Standard Input/Output)
             │
┌────────────▼────────────────────────┐
│   Redot Editor Process              │
│                                     │
│   ┌─────────────────────────────┐  │
│   │  MCP Server Module          │  │
│   │  - Receives JSON-RPC msgs   │  │
│   │  - Processes requests       │  │
│   │  - Returns responses        │  │
│   └────────┬────────────────────┘  │
│            │                        │
│   ┌────────▼────────────────────┐  │
│   │  MCP Tools                  │  │
│   │  - read_file()              │  │
│   │  - list_scenes()            │  │
│   │  - search_in_files()        │  │
│   │  - get_scene_info()         │  │
│   │  - etc...                   │  │
│   └────────┬────────────────────┘  │
│            │                        │
│   ┌────────▼────────────────────┐  │
│   │  Your Redot Project         │  │
│   │  - Scenes (.tscn)           │  │
│   │  - Scripts (.gd)            │  │
│   │  - Resources                │  │
│   │  - Assets                   │  │
│   └─────────────────────────────┘  │
└─────────────────────────────────────┘
```

### Communication Flow

1. **You ask Claude a question** (e.g., "What scenes are in my project?")
2. **Claude Desktop sends an MCP request** via stdin to Redot
3. **Redot's MCP Server receives the JSON-RPC message**
4. **The MCP Server calls the appropriate tool** (e.g., `list_scenes`)
5. **The tool accesses your project files**
6. **Results are returned as JSON** to Claude via stdout
7. **Claude understands your project** and responds to you!

---

## 🔧 How It Works Internally

### Component Breakdown

#### 1. **MCPServer Class** (`mcp_server.h/cpp`)

This is the **core protocol handler**:

```cpp
class MCPServer {
    // Handles JSON-RPC 2.0 messages
    String process_message(const String &p_json);

    // Manages tools and resources
    void register_tool(...);
    void register_resource(...);

    // Protocol lifecycle
    Dictionary _handle_initialize();
    Dictionary _handle_tools_list();
    Dictionary _handle_tools_call();
}
```

**What it does:**
- Parses incoming JSON-RPC messages
- Routes requests to appropriate handlers
- Manages the initialization handshake with clients
- Enforces MCP protocol specifications

**Example Message Flow:**

**Request from Claude:**
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "read_file",
    "arguments": { "path": "player.gd" }
  },
  "id": 1
}
```

**Response from Redot:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "content": [{
      "type": "text",
      "text": "{\"path\":\"res://player.gd\",\"content\":\"extends CharacterBody2D...\"}"
    }]
  }
}
```

#### 2. **MCPTools Class** (`tools/mcp_tools.h/cpp`)

This provides **Redot-specific functionality**:

```cpp
class MCPTools {
    static String tool_read_file(const Dictionary &p_args);
    static String tool_list_scenes(const Dictionary &p_args);
    static String tool_search_in_files(const Dictionary &p_args);
    // ... more tools
}
```

**With stevensStringLib integration!** Your library is used for:
- **String parsing**: `stevensStringLib::separate()` splits scene file content
- **Efficient searching**: `stevensStringLib::findAll()` finds all text occurrences
- **Line counting**: `stevensStringLib::countLines()` counts file lines
- **Whitespace handling**: `stevensStringLib::trimWhitespace()` cleans up text

Example usage in code:
```cpp
// In search_in_files tool:
std::string content_std = content.utf8().get_data();
std::vector<size_t> occurrences = stevensStringLib::findAll(content_std, query_std);
// Returns ALL positions where the search term appears!
```

#### 3. **MCPEditorPlugin Class** (`editor/mcp_editor_plugin.h/cpp`)

This **integrates everything into the Redot Editor**:

```cpp
class MCPEditorPlugin : public EditorPlugin {
    void enable_server();
    void disable_server();
    MCPControlPanel *control_panel; // GUI interface
}
```

**What it does:**
- Creates the MCP Server instance
- Registers all tools at startup
- Provides a GUI panel in the editor bottom dock
- Manages server lifecycle

#### 4. **MCPControlPanel Class** (`editor/mcp_editor_plugin.h/cpp`)

This is the **GUI you see in Redot**:

```cpp
class MCPControlPanel : public PanelContainer {
    Label *status_label;
    Button *toggle_button;
    RichTextLabel *info_label;
}
```

**Features:**
- Shows server status (Running/Stopped)
- Displays number of registered tools
- Start/Stop button for the MCP server
- Real-time connection information

---

## 🚀 Setup Guide: Connecting Claude Desktop

### Step 1: Build Redot with MCP Module

```bash
cd /path/to/redot-engine

# Build Redot editor with MCP module enabled
scons platform=linuxbsd target=editor module_mcp_enabled=yes -j4

# Or for Windows:
# scons platform=windows target=editor module_mcp_enabled=yes

# Or for macOS:
# scons platform=macos target=editor module_mcp_enabled=yes
```

This compiles Redot with the MCP module built-in.

### Step 2: Locate Your Redot Editor Binary

After building, find the editor executable:

```bash
# Linux:
./bin/redot.linuxbsd.editor.x86_64

# Windows:
./bin/redot.windows.editor.x86_64.exe

# macOS:
./bin/redot.macos.editor.universal
```

**Copy the full path!** You'll need it for the next step.

### Step 3: Configure Claude Desktop

Claude Desktop uses a configuration file to know about MCP servers.

**Configuration file location:**

- **macOS**: `~/Library/Application Support/Claude/claude_desktop_config.json`
- **Windows**: `%APPDATA%\Claude\claude_desktop_config.json`
- **Linux**: `~/.config/Claude/claude_desktop_config.json`

**Edit this file** and add the Redot MCP server:

```json
{
  "mcpServers": {
    "redot": {
      "command": "/full/path/to/your/redot.linuxbsd.editor.x86_64",
      "args": ["--path", "/path/to/your/game/project"],
      "env": {}
    }
  }
}
```

**Important Parameters:**
- `command`: Full path to your Redot editor executable
- `args`:
  - `--path`: Points to your specific game project directory
  - You can add `--headless` if you don't want the editor window to open

**Example configuration:**

```json
{
  "mcpServers": {
    "my-platformer-game": {
      "command": "/home/jeff/redot-engine/bin/redot.linuxbsd.editor.x86_64",
      "args": ["--path", "/home/jeff/games/platformer"],
      "env": {}
    },
    "my-rpg-game": {
      "command": "/home/jeff/redot-engine/bin/redot.linuxbsd.editor.x86_64",
      "args": ["--path", "/home/jeff/games/rpg-adventure"],
      "env": {}
    }
  }
}
```

You can have **multiple projects** configured!

### Step 4: Restart Claude Desktop

Close and reopen Claude Desktop for the configuration to take effect.

### Step 5: Verify Connection

In Claude Desktop, you should now see MCP tools available. Try asking:

- "What tools do you have access to?"
- "List the MCP servers connected"
- "What scenes are in my Redot project?"

Claude should respond with information about your Redot project!

---

## 🎮 Using the MCP GUI Panel in Redot

When you open Redot Editor, look for the **"MCP" tab** in the bottom panel (next to Output, Debugger, etc.).

### GUI Features:

1. **Status Display**
   - Shows if the MCP server is running or stopped
   - Displays protocol version and connection details

2. **Tool Count**
   - Shows how many tools are registered (should show 6)

3. **Start/Stop Button**
   - Click to toggle the MCP server on/off
   - Green status when running

4. **Information Panel**
   - Displays current server state
   - Shows protocol details
   - Lists available capabilities

---

## 📝 Available MCP Tools

### 1. **read_file**
Reads any file in your project.

**Claude Usage:**
> "Show me the contents of player.gd"

**Parameters:**
- `path` (required): File path relative to project root

**Returns:**
```json
{
  "path": "res://player.gd",
  "content": "extends CharacterBody2D...",
  "size": 1234,
  "lines": 45
}
```

Note: Line counting powered by **stevensStringLib**!

### 2. **list_scenes**
Lists all scene files in your project.

**Claude Usage:**
> "What scenes are in my game?"

**Parameters:**
- `path` (optional): Directory to search (default: res://)

**Returns:**
```json
{
  "scenes": [
    {"path": "res://main.tscn", "name": "main"},
    {"path": "res://levels/level1.tscn", "name": "level1"}
  ],
  "count": 2
}
```

### 3. **search_in_files**
Searches for text across your project files.

**Claude Usage:**
> "Find all uses of 'velocity' in my GDScript files"

**Parameters:**
- `query` (required): Text to search for
- `path` (optional): Directory to search
- `extension` (optional): File type (default: gd)

**Returns:**
```json
{
  "query": "velocity",
  "results": [
    {
      "path": "res://player.gd",
      "matches": [
        {"line": 12, "text": "var velocity = Vector2()"},
        {"line": 34, "text": "velocity.x = speed"}
      ],
      "occurrences": 2
    }
  ],
  "count": 1
}
```

**Powered by stevensStringLib:**
- Uses `findAll()` to locate ALL occurrences efficiently
- Uses `separate()` to split files into lines
- Uses `trimWhitespace()` to clean up matched lines

### 4. **get_project_settings**
Gets your project configuration.

**Claude Usage:**
> "What are my project settings?"

**Returns:**
```json
{
  "project_name": "My Awesome Game",
  "main_scene": "res://main.tscn",
  "project_dir": "/home/user/games/my-game",
  "display": {
    "width": 1920,
    "height": 1080,
    "resizable": true
  }
}
```

### 5. **list_directory**
Browses project directory structure.

**Claude Usage:**
> "Show me what's in the scripts folder"

**Parameters:**
- `path` (optional): Directory path
- `recursive` (optional): Search subdirectories

**Returns:**
```json
{
  "path": "res://scripts",
  "files": [
    {"path": "res://scripts/player.gd", "name": "player.gd"}
  ],
  "directories": ["res://scripts/enemies"],
  "file_count": 1,
  "dir_count": 1
}
```

### 6. **get_scene_info**
Analyzes scene file structure.

**Claude Usage:**
> "What nodes are in main.tscn?"

**Parameters:**
- `path` (required): Path to .tscn file

**Returns:**
```json
{
  "path": "res://main.tscn",
  "nodes": [
    {"name": "Main", "type": "Node2D", "parent": "."},
    {"name": "Player", "type": "CharacterBody2D", "parent": "."},
    {"name": "Sprite", "type": "Sprite2D", "parent": "Player"}
  ],
  "node_count": 3
}
```

---

## 🔍 Example Conversation

**You**: "Claude, analyze my player script and suggest improvements"

**Claude** (internally):
1. Calls `list_scenes()` to find scene files
2. Calls `get_scene_info("main.tscn")` to see the scene structure
3. Calls `read_file("player.gd")` to read the script
4. Analyzes the code and provides suggestions

**Claude** (to you): "I've analyzed your player.gd script. Here's what I found..."

---

## 🧪 Testing Your Setup

### Test 1: Verify Tools are Available

Ask Claude:
> "List all the MCP tools you have access to for my Redot project"

You should see all 6 tools listed.

### Test 2: Read a File

Ask Claude:
> "Read the contents of project.godot"

Claude should show you your project configuration file.

### Test 3: Search Across Project

Ask Claude:
> "Search for all occurrences of 'extends Node' in my project"

Claude should find and list all scripts that extend Node.

### Test 4: Scene Analysis

Ask Claude:
> "Show me the node structure of my main scene"

Claude should display the node tree.

---

## 🐛 Troubleshooting

### Problem: Claude can't see the MCP server

**Solutions:**
1. Check that Claude Desktop config file is valid JSON
2. Verify the path to Redot editor is correct
3. Make sure the `--path` points to a valid project
4. Restart Claude Desktop completely
5. Check Redot console output for MCP server startup messages

### Problem: MCP panel doesn't appear in Redot

**Solutions:**
1. Make sure you built with `module_mcp_enabled=yes`
2. Check that you're using the editor build, not runtime
3. Look in bottom panel tabs for "MCP"
4. Check console for MCP module initialization messages

### Problem: Tools return errors

**Solutions:**
1. Verify file paths are relative to project root
2. Check that the project is open in Redot
3. Ensure MCP server is started (green status in panel)
4. Check console output for error messages

---

## 📊 Performance Notes

Thanks to **stevensStringLib**, the MCP tools are highly optimized:

- **Searching**: `findAll()` uses efficient string searching algorithms
- **Parsing**: `separate()` minimizes allocations when splitting text
- **Line counting**: `countLines()` is faster than naive implementations
- **Memory efficient**: String views used where possible

---

## 🎯 What Can You Build With This?

- **AI-Assisted Game Development**: Claude can suggest code improvements
- **Project Documentation**: Automatically generate docs from your scenes
- **Code Reviews**: Claude can review GDScript code for issues
- **Scene Analysis**: Understand complex scene hierarchies
- **Refactoring**: Claude can help rename variables across files
- **Learning**: Ask Claude to explain how parts of your game work

---

## 🔮 Future Enhancements

Potential additions:
- Real-time code modification
- GDScript syntax validation
- Asset analysis (textures, audio)
- Git integration for version control
- Build system integration
- Debugger connection
- Performance profiling data

---

## 📄 Credits

- **MCP Integration**: Built for Redot Engine community
- **stevensStringLib**: [github.com/Bucephalus-Studios/stevensStringLib](https://github.com/Bucephalus-Studios/stevensStringLib)
  - Used for efficient string parsing, searching, and manipulation
  - MIT Licensed by Jeff Stevens / Bucephalus Studios
- **MCP Protocol**: [modelcontextprotocol.io](https://modelcontextprotocol.io) by Anthropic

---

## 📚 Additional Resources

- [MCP Specification](https://modelcontextprotocol.io)
- [Redot Engine Documentation](https://redotengine.org)
- [Claude Desktop](https://claude.ai/download)
- [JSON-RPC 2.0 Specification](https://www.jsonrpc.org/specification)

---

**Enjoy building games with AI assistance! 🎮🤖**
