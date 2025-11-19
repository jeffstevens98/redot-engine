/**************************************************************************/
/*  test_mcp_server.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#include "tests/test_macros.h"

#include "../mcp_server.h"

namespace TestMCPServer {

// Test basic MCP server initialization
TEST_CASE("[MCP] Server initialization") {
	Ref<MCPServer> server;
	server.instantiate();

	CHECK_FALSE(server->is_running());
	CHECK(server->get_protocol_version() == "2025-03-15");

	Dictionary server_info = server->get_server_info();
	CHECK(server_info.has("name"));
	CHECK(server_info.has("version"));
}

// Test tool registration
TEST_CASE("[MCP] Tool registration") {
	Ref<MCPServer> server;
	server.instantiate();

	Dictionary schema;
	schema["type"] = "object";

	Callable callback = Callable();
	server->register_tool("test_tool", "A test tool", schema, callback);

	// Start server to initialize
	server->start();

	// Request tools list
	String request = R"({"jsonrpc":"2.0","method":"tools/list","id":1})";
	String response = server->process_message(request);

	CHECK(response.contains("test_tool"));
	CHECK(response.contains("A test tool"));
}

// Test JSON-RPC message parsing
TEST_CASE("[MCP] JSON-RPC message parsing") {
	Ref<MCPServer> server;
	server.instantiate();

	SUBCASE("Valid initialize request") {
		String request = R"({
			"jsonrpc": "2.0",
			"method": "initialize",
			"params": {
				"protocolVersion": "2025-03-15",
				"clientInfo": {
					"name": "test-client",
					"version": "1.0.0"
				}
			},
			"id": 1
		})";

		String response = server->process_message(request);

		CHECK(response.contains("\"jsonrpc\":\"2.0\""));
		CHECK(response.contains("\"id\":1"));
		CHECK(response.contains("protocolVersion"));
		CHECK(response.contains("serverInfo"));
		CHECK(response.contains("capabilities"));
	}

	SUBCASE("Invalid JSON") {
		String request = "not valid json";
		String response = server->process_message(request);

		CHECK(response.contains("error"));
		CHECK(response.contains("-32700")); // PARSE_ERROR
	}

	SUBCASE("Missing method") {
		String request = R"({"jsonrpc":"2.0","id":1})";
		String response = server->process_message(request);

		CHECK(response.contains("error"));
		CHECK(response.contains("-32600")); // INVALID_REQUEST
	}

	SUBCASE("Unknown method") {
		String request = R"({"jsonrpc":"2.0","method":"unknown_method","id":1})";
		String response = server->process_message(request);

		CHECK(response.contains("error"));
		CHECK(response.contains("-32601")); // METHOD_NOT_FOUND
	}
}

// Test initialization handshake
TEST_CASE("[MCP] Initialization handshake") {
	Ref<MCPServer> server;
	server.instantiate();

	CHECK_FALSE(server->is_running());

	// Send initialize request
	String init_request = R"({
		"jsonrpc": "2.0",
		"method": "initialize",
		"params": {
			"protocolVersion": "2025-03-15",
			"clientInfo": {"name": "test", "version": "1.0"}
		},
		"id": 1
	})";

	String response = server->process_message(init_request);

	// Verify response structure
	JSON json;
	Error err = json.parse(response);
	CHECK(err == OK);

	Dictionary response_dict = json.get_data();
	CHECK(response_dict.has("result"));

	Dictionary result = response_dict["result"];
	CHECK(result.has("protocolVersion"));
	CHECK(result.has("serverInfo"));
	CHECK(result.has("capabilities"));

	Dictionary capabilities = result["capabilities"];
	CHECK(capabilities.has("tools"));
	CHECK(capabilities.has("resources"));

	// Server should now be initialized
	CHECK(server->is_running());
}

// Test resource registration
TEST_CASE("[MCP] Resource registration") {
	Ref<MCPServer> server;
	server.instantiate();

	Callable reader = Callable();
	server->register_resource(
			"test://resource",
			"Test Resource",
			"A test resource",
			"text/plain",
			reader);

	server->start();

	String request = R"({"jsonrpc":"2.0","method":"resources/list","id":1})";
	String response = server->process_message(request);

	CHECK(response.contains("test://resource"));
	CHECK(response.contains("Test Resource"));
	CHECK(response.contains("text/plain"));
}

} // namespace TestMCPServer
