#!/usr/bin/env python3
"""
Integration test for MCP module
Tests that all files are present and properly structured
"""

import os
import sys

def test_file_structure():
    """Verify all required files exist"""
    mcp_dir = "/home/user/redot-engine/modules/mcp"

    required_files = [
        # Core
        "mcp_server.h", "mcp_server.cpp",
        "register_types.h", "register_types.cpp",

        # Tools
        "tools/mcp_tools.h", "tools/mcp_tools.cpp",

        # Editor (new chat interface)
        "editor/claude_api_client.h", "editor/claude_api_client.cpp",
        "editor/claude_chat_panel.h", "editor/claude_chat_panel.cpp",
        "editor/claude_settings_dialog.h", "editor/claude_settings_dialog.cpp",
        "editor/mcp_editor_plugin_v2.h", "editor/mcp_editor_plugin_v2.cpp",

        # Build
        "config.py", "SCsub",

        # External library
        "stevensStringLib.h",

        # Tests
        "tests/test_mcp_server.h", "tests/test_mcp_tools.h",

        # Docs
        "README.md", "QUICKSTART.md", "HOWTO.md"
    ]

    missing = []
    for file in required_files:
        full_path = os.path.join(mcp_dir, file)
        if not os.path.exists(full_path):
            missing.append(file)
        else:
            print(f"✓ {file}")

    if missing:
        print(f"\n❌ Missing files: {missing}")
        return False

    print(f"\n✅ All {len(required_files)} required files present!")
    return True

def test_class_definitions():
    """Verify all expected classes are defined"""
    mcp_dir = "/home/user/redot-engine/modules/mcp"

    expected_classes = {
        "mcp_server.h": ["MCPServer"],
        "tools/mcp_tools.h": ["MCPTools"],
        "editor/claude_api_client.h": ["ClaudeAPIClient"],
        "editor/claude_chat_panel.h": ["ChatMessage", "ToolCallDisplay", "ClaudeChatPanel"],
        "editor/claude_settings_dialog.h": ["ClaudeSettingsDialog"],
        "editor/mcp_editor_plugin_v2.h": ["MCPEditorPluginV2"],
    }

    print("\n=== Class Definitions ===")
    all_found = True

    for file, classes in expected_classes.items():
        full_path = os.path.join(mcp_dir, file)
        try:
            with open(full_path, 'r') as f:
                content = f.read()
                for cls in classes:
                    if f"class {cls}" in content:
                        print(f"✓ {cls} found in {file}")
                    else:
                        print(f"❌ {cls} NOT found in {file}")
                        all_found = False
        except Exception as e:
            print(f"❌ Error reading {file}: {e}")
            all_found = False

    if all_found:
        print("\n✅ All class definitions found!")
    return all_found

def test_registration():
    """Verify module registration is correct"""
    print("\n=== Module Registration ===")
    reg_file = "/home/user/redot-engine/modules/mcp/register_types.cpp"

    try:
        with open(reg_file, 'r') as f:
            content = f.read()

        checks = [
            ("MCPServer registration", "GDREGISTER_CLASS(MCPServer)"),
            ("MCPTools registration", "GDREGISTER_CLASS(MCPTools)"),
            ("Plugin registration", "EditorPlugins::add_by_type<MCPEditorPluginV2>()"),
            ("Includes V2 plugin", "#include \"editor/mcp_editor_plugin_v2.h\""),
        ]

        all_passed = True
        for name, pattern in checks:
            if pattern in content:
                print(f"✓ {name}")
            else:
                print(f"❌ {name} - pattern not found: {pattern}")
                all_passed = False

        if all_passed:
            print("\n✅ Module registration is correct!")
        return all_passed

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_tool_count():
    """Verify all 6 tools are registered"""
    print("\n=== Tool Registration ===")
    plugin_file = "/home/user/redot-engine/modules/mcp/editor/mcp_editor_plugin_v2.cpp"

    tools = [
        "read_file",
        "list_scenes",
        "get_project_settings",
        "search_in_files",
        "list_directory",
        "get_scene_info"
    ]

    try:
        with open(plugin_file, 'r') as f:
            content = f.read()

        found_tools = []
        for tool in tools:
            if f'"{tool}"' in content and "register_tool" in content:
                print(f"✓ {tool} tool registered")
                found_tools.append(tool)
            else:
                print(f"❌ {tool} tool NOT found")

        if len(found_tools) == 6:
            print(f"\n✅ All 6 tools registered!")
            return True
        else:
            print(f"\n❌ Only {len(found_tools)}/6 tools found")
            return False

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_stevensStringLib_integration():
    """Verify stevensStringLib is integrated"""
    print("\n=== stevensStringLib Integration ===")
    tools_file = "/home/user/redot-engine/modules/mcp/tools/mcp_tools.cpp"

    try:
        with open(tools_file, 'r') as f:
            content = f.read()

        usages = [
            ("Include", '#include "../stevensStringLib.h"'),
            ("findAll()", "stevensStringLib::findAll"),
            ("separate()", "stevensStringLib::separate"),
            ("trimWhitespace()", "stevensStringLib::trimWhitespace"),
            ("countLines()", "stevensStringLib::countLines"),
        ]

        all_found = True
        for name, pattern in usages:
            if pattern in content:
                print(f"✓ {name} used")
            else:
                print(f"❌ {name} NOT found")
                all_found = False

        if all_found:
            print("\n✅ stevensStringLib properly integrated!")
        return all_found

    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_documentation():
    """Verify documentation is complete"""
    print("\n=== Documentation ===")
    mcp_dir = "/home/user/redot-engine/modules/mcp"

    docs = {
        "README.md": ["Claude for Redot", "Embedded Chat", "QUICKSTART.md"],
        "QUICKSTART.md": ["3 Simple Steps", "API key", "Settings"],
        "HOWTO.md": ["Architecture", "Component", "Credits"],
    }

    all_good = True
    for doc, keywords in docs.items():
        try:
            with open(os.path.join(mcp_dir, doc), 'r') as f:
                content = f.read()

            found = all(kw in content for kw in keywords)
            if found:
                print(f"✓ {doc} contains key sections")
            else:
                print(f"❌ {doc} missing some sections")
                all_good = False
        except Exception as e:
            print(f"❌ Error reading {doc}: {e}")
            all_good = False

    if all_good:
        print("\n✅ Documentation complete!")
    return all_good

def main():
    """Run all tests"""
    print("=" * 60)
    print("MCP MODULE INTEGRATION TEST")
    print("=" * 60)

    tests = [
        ("File Structure", test_file_structure),
        ("Class Definitions", test_class_definitions),
        ("Module Registration", test_registration),
        ("Tool Registration", test_tool_count),
        ("stevensStringLib", test_stevensStringLib_integration),
        ("Documentation", test_documentation),
    ]

    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, result))
        except Exception as e:
            print(f"\n❌ Test '{name}' crashed: {e}")
            results.append((name, False))

    print("\n" + "=" * 60)
    print("RESULTS SUMMARY")
    print("=" * 60)

    for name, passed in results:
        status = "✅ PASS" if passed else "❌ FAIL"
        print(f"{status}: {name}")

    total = len(results)
    passed = sum(1 for _, p in results if p)

    print("\n" + "=" * 60)
    print(f"TOTAL: {passed}/{total} tests passed")

    if passed == total:
        print("🎉 ALL TESTS PASSED! Integration is ready!")
        print("=" * 60)
        return 0
    else:
        print("⚠️  Some tests failed. Review above for details.")
        print("=" * 60)
        return 1

if __name__ == "__main__":
    sys.exit(main())
