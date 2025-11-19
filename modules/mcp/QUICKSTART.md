# Claude for Redot - Quick Start Guide

## 🚀 Getting Started in 3 Simple Steps!

### Step 1: Build Redot

```bash
cd /path/to/redot-engine
scons platform=linuxbsd target=editor -j4
```

That's it! The Claude assistant is built right in!

### Step 2: Get Your API Key

1. Go to https://console.anthropic.com
2. Sign up or log in
3. Navigate to **API Keys**
4. Click **Create Key**
5. Copy your API key (starts with `sk-ant-...`)

### Step 3: Configure in Redot

1. Open Redot Editor
2. Look for the **"Claude"** tab in the bottom panel (next to Output, Debugger)
3. Click the **"Settings"** button
4. Paste your API key
5. Click **OK**

## ✨ You're Ready!

Now you can chat with Claude directly in Redot!

### Try These Questions:

- "What scenes are in my project?"
- "Read player.gd and explain what it does"
- "Search for 'velocity' in my scripts"
- "Show me the node structure of main.tscn"
- "Help me fix this bug in my code"

## 💬 The Chat Interface

The Claude panel looks like this:

```
┌──────────────────────────────────────────────┐
│ Claude Assistant          ● Ready  [Settings][Clear] │
├──────────────────────────────────────────────┤
│  [Chat messages appear here]                 │
│                                              │
│  You: What scenes are in my project?        │
│                                              │
│  Claude: Let me check...                    │
│  🔧 Tool: list_scenes                       │
│      Input: {}                              │
│      Result: Found 5 scenes...              │
│                                              │
│  Here are your project scenes:               │
│  • main.tscn                                │
│  • player.tscn                              │
│  • enemy.tscn                               │
│  ...                                         │
├──────────────────────────────────────────────┤
│ [Type your message here...]                  │
│                                    [Send]    │
└──────────────────────────────────────────────┘
```

## 🔧 What Claude Can Do

Claude has access to **6 powerful tools** to help you:

1. **read_file** - Read any project file
2. **list_scenes** - Find all scenes
3. **search_in_files** - Search through code
4. **get_project_settings** - View project config
5. **list_directory** - Browse files
6. **get_scene_info** - Analyze scene structure

You'll see these tools being used in real-time when Claude answers your questions!

## 💡 Pro Tips

### 1. Be Specific
Instead of: "Fix my code"
Try: "Read player.gd and suggest improvements to the movement code"

### 2. Use Context
Claude can see your entire project, so you can say:
- "Compare player.gd and enemy.gd"
- "Find all scripts that use 'velocity'"
- "Analyze the main scene structure"

### 3. Ask for Explanations
- "Explain how the player movement works"
- "What does this function do?" (after showing code)
- "Help me understand scene hierarchies"

### 4. Get Suggestions
- "How can I optimize this script?"
- "What's a better way to structure my code?"
- "Review my project organization"

## 🎮 Example Conversations

### Example 1: Understanding Your Project

**You**: "Give me an overview of my game project"

**Claude**: *Uses multiple tools to explore your project*
- Calls `get_project_settings()` to see your game config
- Calls `list_scenes()` to find all scenes
- Calls `list_directory()` to see file structure

Then gives you a comprehensive overview!

### Example 2: Debugging

**You**: "I'm getting an error in player.gd line 45, can you help?"

**Claude**: *Reads the file and analyzes it*
- Calls `read_file("player.gd")`
- Looks at line 45
- Explains the issue
- Suggests a fix

### Example 3: Code Search

**You**: "Find everywhere I use 'get_node' in my project"

**Claude**: *Searches your codebase*
- Calls `search_in_files(query="get_node", extension="gd")`
- Shows all occurrences with line numbers
- Explains common patterns

## 🔐 Privacy & Security

- Your API key is stored **locally** in Redot editor settings
- No project data is sent anywhere except to Anthropic's API
- You control what Claude can see
- Your code stays private (covered by Anthropic's privacy policy)

## 💰 Costs

Claude uses Anthropic's API which has usage-based pricing:
- **Sonnet 4.5** (Recommended): ~$3 per million input tokens
- Typical conversation: A few cents
- Reading a large file: Fractions of a cent

Check current pricing at: https://anthropic.com/pricing

## ❓ Troubleshooting

### "Please configure your API key"
- Click Settings and enter your Anthropic API key
- Make sure it starts with `sk-ant-`

### "API key format looks invalid"
- Verify you copied the entire key from Anthropic console
- Check for extra spaces

### Chat panel doesn't appear
- Make sure you built with the MCP module (it's default)
- Check the bottom panel tabs for "Claude"
- Try restarting Redot

### "Error calling tool"
- Make sure your project is open
- Check file paths are correct (relative to project root)
- Look at Output tab for detailed errors

## 🆘 Getting Help

If you have issues:
1. Check the Output tab in Redot for error messages
2. Verify your API key is correct in Settings
3. Try the "Test Connection" button in Settings
4. Check Anthropic's status page for API outages

## 🎉 That's It!

You now have a powerful AI assistant built right into your game engine!

**No external apps. No configuration files. No copying paths. Just chat!**

Happy game development! 🎮✨

---

**More Info:**
- Full Documentation: See `README.md` and `HOWTO.md`
- API Reference: https://docs.anthropic.com
- Source Code: `modules/mcp/`
