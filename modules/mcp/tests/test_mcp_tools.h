/**************************************************************************/
/*  test_mcp_tools.h                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#include "tests/test_macros.h"

#include "../stevensStringLib.h"
#include "../tools/mcp_tools.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"

namespace TestMCPTools {

// Test schema creation helpers
TEST_CASE("[MCPTools] Schema creation") {
	Array required;
	required.push_back("test_param");

	Dictionary properties;
	properties["test_param"] = MCPTools::make_property_string("Test parameter");

	Dictionary schema = MCPTools::make_schema_object(required, properties);

	CHECK(schema.has("type"));
	CHECK(schema["type"] == "object");
	CHECK(schema.has("properties"));
	CHECK(schema.has("required"));

	Dictionary props = schema["properties"];
	CHECK(props.has("test_param"));
}

// Test property string creation
TEST_CASE("[MCPTools] Property string creation") {
	Dictionary prop = MCPTools::make_property_string("Test description");

	CHECK(prop.has("type"));
	CHECK(prop["type"] == "string");
	CHECK(prop.has("description"));
	CHECK(prop["description"] == "Test description");
}

// Test read_file tool (requires actual file system)
TEST_CASE("[MCPTools] Read file tool") {
	// Create a temporary test file
	String test_path = "res://test_file.txt";
	Ref<FileAccess> file = FileAccess::open(test_path, FileAccess::WRITE);
	if (file.is_valid()) {
		String test_content = "Hello, World!\nThis is a test file.\nLine 3";
		file->store_string(test_content);
		file->close();

		// Test reading the file
		Dictionary args;
		args["path"] = "test_file.txt";

		String result_json = MCPTools::tool_read_file(args);

		CHECK(result_json.contains("test_file.txt"));
		CHECK(result_json.contains("Hello, World!"));
		CHECK(result_json.contains("\"lines\":3")); // stevensStringLib line counting!

		// Cleanup
		Ref<DirAccess> dir = DirAccess::open("res://");
		if (dir.is_valid()) {
			dir->remove(test_path);
		}
	}
}

// Test get_project_settings tool
TEST_CASE("[MCPTools] Get project settings") {
	Dictionary args;
	String result = MCPTools::tool_get_project_settings(args);

	// Should return valid JSON
	JSON json;
	Error err = json.parse(result);
	CHECK(err == OK);

	Dictionary result_dict = json.get_data();
	CHECK(result_dict.has("project_name"));
	CHECK(result_dict.has("project_dir"));
}

// Test stevensStringLib integration
TEST_CASE("[MCPTools] stevensStringLib integration") {
	SUBCASE("String separation") {
		std::string test = "one,two,three";
		std::vector<std::string> parts = stevensStringLib::separate(test, ',', false);

		CHECK(parts.size() == 3);
		CHECK(parts[0] == "one");
		CHECK(parts[1] == "two");
		CHECK(parts[2] == "three");
	}

	SUBCASE("Find all occurrences") {
		std::string text = "hello world, hello universe, hello redot";
		std::vector<size_t> positions = stevensStringLib::findAll(text, "hello");

		CHECK(positions.size() == 3);
		CHECK(positions[0] == 0);
		CHECK(positions[1] == 13);
		CHECK(positions[2] == 30);
	}

	SUBCASE("Trim whitespace") {
		std::string text = "   hello world   ";
		std::string trimmed = stevensStringLib::trimWhitespace(text);

		CHECK(trimmed == "hello world");
	}

	SUBCASE("Count lines") {
		std::string text = "line1\nline2\nline3";
		unsigned long long count = stevensStringLib::countLines(text);

		CHECK(count == 3);
	}
}

// Test list_directory tool
TEST_CASE("[MCPTools] List directory") {
	Dictionary args;
	args["path"] = "res://";
	args["recursive"] = false;

	String result = MCPTools::tool_list_directory(args);

	CHECK(result.contains("\"path\":\"res://\""));
	CHECK(result.contains("files"));
	CHECK(result.contains("directories"));
}

// Test search integration with stevensStringLib
TEST_CASE("[MCPTools] Search with stevensStringLib") {
	// Create test files
	Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	dir->make_dir("res://test_search");

	Ref<FileAccess> file1 = FileAccess::open("res://test_search/file1.gd", FileAccess::WRITE);
	if (file1.is_valid()) {
		file1->store_string("extends Node\nfunc test():\n\tprint('hello')");
		file1->close();
	}

	Ref<FileAccess> file2 = FileAccess::open("res://test_search/file2.gd", FileAccess::WRITE);
	if (file2.is_valid()) {
		file2->store_string("class_name TestClass\nfunc hello():\n\tpass");
		file2->close();
	}

	// Search for "hello"
	Dictionary args;
	args["query"] = "hello";
	args["path"] = "res://test_search";
	args["extension"] = "gd";

	String result = MCPTools::tool_search_in_files(args);

	CHECK(result.contains("hello"));
	CHECK(result.contains("occurrences")); // Added by stevensStringLib integration!

	// Cleanup
	dir->remove("res://test_search/file1.gd");
	dir->remove("res://test_search/file2.gd");
	dir->remove("res://test_search");
}

} // namespace TestMCPTools
