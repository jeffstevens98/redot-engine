# Claude for Redot - AI Assistant Integration

**Chat with Claude directly in the Redot Editor!**

This module brings Claude AI assistant into Redot Engine, giving you an intelligent coding companion that understands your game project.

## 🎯 What Is This?

A complete Claude AI integration embedded directly in Redot Editor. Think "Claude Code" but for game development!

**No external apps. No configuration files. Just a chat panel in your editor.**

## ✨ Features

### 💬 Embedded Chat Interface
- Chat panel built into Redot Editor (bottom panel, like Output/Debugger)
- Real-time conversation with Claude
- Message history
- Tool call visualization (see what Claude is doing)

### 🛠️ Powerful Tools
Claude has access to 6 tools to understand your project:

1. **read_file** - Read any project file
2. **list_scenes** - Find all .tscn scene files
3. **search_in_files** - Search code with **stevensStringLib** (fast!)
4. **get_project_settings** - View project configuration
5. **list_directory** - Browse project structure
6. **get_scene_info** - Analyze scene node trees

### 🚀 Super Simple Setup
1. Build Redot (MCP module is included by default)
2. Get Anthropic API key
3. Paste into Settings
4. Start chatting!

## 📸 Quick Look

```
Redot Editor
├── [Your usual editor layout]
└── Bottom Panel
    ├── Output
    ├── Debugger
    └── ⭐ Claude ← Click here!
        ├── Chat history
        ├── Tool visualizations
        └── Input box
```

When you ask Claude a question, you'll see:
- Your message
- Tool calls being executed (e.g., "🔧 Tool: read_file")
- Results from tools
- Claude's response

## 🚀 Quick Start

See **[QUICKSTART.md](QUICKSTART.md)** for the 3-step setup guide!

**TL;DR:**
1. Build: `scons platform=linuxbsd target=editor`
2. Get API key from https://console.anthropic.com
3. Open Redot → Claude tab → Settings → Paste key

## 💡 Example Conversations

### "What scenes are in my project?"
```
You: What scenes are in my project?

🔧 Tool: list_scenes
    Input: {}
    Result: Found 5 scenes...

Claude: You have 5 scenes in your project:
• main.tscn - Your main game scene
• player.tscn - Player character
• enemy.tscn - Enemy character
• level1.tscn - First level
• ui.tscn - User interface
```

### "Read player.gd and explain it"
```
You: Read player.gd and explain what it does

🔧 Tool: read_file
    Input: {"path": "player.gd"}
    Result: 45 lines, 1.2 KB

Claude: Your player.gd script extends CharacterBody2D and implements:

1. Movement System (lines 10-25)
   - WASD input handling
   - Velocity-based movement
   - Speed variable at 300 pixels/sec

2. Jumping (lines 27-35)
   - Jump height: 400 pixels
   - Gravity: 980 (standard)

Suggestions:
- Consider adding coyote time for better jump feel
- Could extract constants to @export variables
```

### "Find all uses of 'velocity'"
```
You: Search for 'velocity' in my scripts

🔧 Tool: search_in_files
    Input: {"query": "velocity", "extension": "gd"}
    Result: Found 12 occurrences in 3 files

Claude: Found velocity used in 3 files:

player.gd (5 occurrences):
  Line 12: var velocity = Vector2()
  Line 23: velocity.x = direction * speed
  ...

enemy.gd (4 occurrences):
  ...

Would you like me to explain the velocity patterns or suggest improvements?
```

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│         Redot Editor                │
│                                     │
│  ┌────────────────────────────┐    │
│  │  Claude Chat Panel         │    │
│  │  - Message history         │    │
│  │  - Input box               │    │
│  │  - Tool visualization      │    │
│  └─────────┬──────────────────┘    │
│            │                        │
│  ┌─────────▼──────────────────┐    │
│  │  Claude API Client         │    │
│  │  - HTTP to api.anthropic.com│    │
│  │  - Streaming responses     │    │
│  └─────────┬──────────────────┘    │
│            │                        │
│  ┌─────────▼──────────────────┐    │
│  │  MCP Server (Internal)     │    │
│  │  - Tool registry           │    │
│  │  - Execution               │    │
│  └─────────┬──────────────────┘    │
│            │                        │
│  ┌─────────▼──────────────────┐    │
│  │  MCP Tools                 │    │
│  │  - read_file()             │    │
│  │  - list_scenes()           │    │
│  │  - search_in_files()       │    │
│  │  - stevensStringLib ✨     │    │
│  └────────────────────────────┘    │
│                                     │
└─────────────────────────────────────┘
```

## 🔧 Technical Details

### Built With
- **C++17** - Core implementation
- **Redot GUI** - Native UI components
- **Anthropic API** - Claude AI
- **stevensStringLib** - Fast string operations ([github](https://github.com/Bucephalus-Studios/stevensStringLib))
- **MCP Protocol** - Tool framework

### Files
```
modules/mcp/
├── README.md                      # This file
├── QUICKSTART.md                  # Quick setup guide
├── HOWTO.md                       # Detailed documentation
├── stevensStringLib.h             # String utilities
├── config.py, SCsub               # Build configuration
├── mcp_server.h/cpp               # MCP protocol core
├── register_types.h/cpp           # Module registration
├── tools/
│   └── mcp_tools.h/cpp           # Redot-specific tools
├── editor/
│   ├── claude_api_client.h/cpp   # Anthropic API client
│   ├── claude_chat_panel.h/cpp   # Chat UI
│   ├── claude_settings_dialog.h/cpp  # Settings dialog
│   └── mcp_editor_plugin_v2.h/cpp    # Editor integration
└── tests/
    ├── test_mcp_server.h         # Protocol tests
    └── test_mcp_tools.h          # Tool tests
```

## 🎮 Use Cases

### Learning & Understanding
- "Explain how my player movement works"
- "What's the structure of my main scene?"
- "Show me all my scripts"

### Code Review & Suggestions
- "Review player.gd and suggest improvements"
- "Is there a better way to structure this code?"
- "Find potential bugs in my scripts"

### Project Navigation
- "Where do I handle collision?"
- "Which scenes use the Player node?"
- "List all scripts that extend Node2D"

### Refactoring Help
- "Help me rename this variable everywhere"
- "Find all hardcoded values I should make constants"
- "Suggest how to organize my project better"

## 🧪 Testing

Run tests:
```bash
scons tests=yes platform=linuxbsd target=editor
./bin/redot.linuxbsd.editor.x86_64 --test --test-case="*MCP*"
```

Tests cover:
- MCP protocol (JSON-RPC, initialization, tools)
- Tool execution (file reading, searching, etc.)
- stevensStringLib integration
- Error handling

## 🔐 Privacy & Security

- API key stored locally in Redot editor settings
- No data sent except to Anthropic API
- Your code/project data processed per [Anthropic's privacy policy](https://www.anthropic.com/legal/privacy)
- You control what Claude can access

## 💰 Pricing

Anthropic API is usage-based:
- **Claude Sonnet 4.5** (recommended): ~$3 per million input tokens
- Typical conversation: A few cents
- Reading a 1000-line file: ~$0.003

See: https://anthropic.com/pricing

## 🚧 Current Limitations

- API calls require internet connection
- Streaming responses not yet implemented (text arrives all at once)
- Markdown rendering in chat uses BBCode (basic formatting)

## 🎯 Roadmap

- [x] Chat UI
- [x] 6 core tools
- [x] Settings dialog
- [x] stevensStringLib integration
- [x] Complete HTTP API client ✨ NEW!
- [x] Multi-turn tool use (Claude can call tools → see results → respond) ✨ NEW!
- [x] Settings dialog wired up ✨ NEW!
- [ ] Streaming responses (word-by-word)
- [ ] Better markdown rendering
- [ ] Code modification tools
- [ ] Git integration
- [ ] Custom tool API for plugin developers
- [ ] Conversation history save/load

## 🤝 Contributing

Want to add more tools? Easy!

```cpp
// In mcp_tools.h
static String tool_my_feature(const Dictionary &p_args);

// In mcp_tools.cpp
String MCPTools::tool_my_feature(const Dictionary &p_args) {
    // Use stevensStringLib for string ops!
    std::string text = stevensStringLib::trimWhitespace(input);

    Dictionary result;
    result["data"] = "something useful";
    return result.to_json_string();
}

// In mcp_editor_plugin_v2.cpp, add to _register_all_tools():
mcp_server->register_tool(
    "my_feature",
    "Description of what it does",
    schema,
    Callable(MCPTools::tool_my_feature));
```

## 📚 More Documentation

- **QUICKSTART.md** - 3-step setup guide
- **HOWTO.md** - Detailed architecture and usage
- **tests/** - Code examples in unit tests

## 🙏 Credits

- **MCP Integration**: Built for Redot Engine
- **stevensStringLib**: [Jeff Stevens / Bucephalus Studios](https://github.com/Bucephalus-Studios/stevensStringLib) (MIT License)
- **MCP Protocol**: [Anthropic](https://modelcontextprotocol.io)
- **Redot Engine**: [redotengine.org](https://redotengine.org)

## 📄 License

MIT License - Same as Redot Engine

---

**Start building games with AI assistance today! 🎮🤖**
