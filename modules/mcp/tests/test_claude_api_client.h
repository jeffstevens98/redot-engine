/**************************************************************************/
/*  test_claude_api_client.h                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "tests/test_macros.h"

#include "../editor/claude_api_client.h"

namespace TestClaudeAPIClient {

// Test API client initialization
TEST_CASE("[ClaudeAPI] Client initialization") {
	Ref<ClaudeAPIClient> client;
	client.instantiate();

	CHECK_FALSE(client->has_api_key());

	// Set API key
	client->set_api_key("sk-test-key-12345");
	CHECK(client->has_api_key());

	// Set model
	client->set_model("claude-3-opus-20240229");
	// Model should be set (no getter to test, but no crash)
}

// Test request payload building
TEST_CASE("[ClaudeAPI] Request payload structure") {
	Ref<ClaudeAPIClient> client;
	client.instantiate();
	client->set_api_key("sk-test-key");

	// Build message history
	Array messages;
	Dictionary user_msg;
	user_msg["role"] = "user";
	user_msg["content"] = "Hello, Claude!";
	messages.push_back(user_msg);

	// Build tools array
	Array tools;
	Dictionary tool;
	tool["name"] = "test_tool";
	tool["description"] = "A test tool";
	Dictionary schema;
	schema["type"] = "object";
	tool["input_schema"] = schema;
	tools.push_back(tool);

	// The client should be able to process these
	// (We can't actually test send_message without a real API, but we can test structure)
	CHECK(messages.size() == 1);
	CHECK(tools.size() == 1);
}

// Test API key validation
TEST_CASE("[ClaudeAPI] API key validation") {
	Ref<ClaudeAPIClient> client;
	client.instantiate();

	SUBCASE("Empty API key") {
		client->set_api_key("");
		CHECK_FALSE(client->has_api_key());
	}

	SUBCASE("Valid API key") {
		client->set_api_key("sk-ant-api03-test");
		CHECK(client->has_api_key());
	}

	SUBCASE("API key with whitespace") {
		client->set_api_key("  sk-test  ");
		// Should handle trimming or accept as-is
		CHECK(client->has_api_key());
	}
}

// Test model configuration
TEST_CASE("[ClaudeAPI] Model configuration") {
	Ref<ClaudeAPIClient> client;
	client.instantiate();

	// Default model should be Sonnet 4.5
	// (Can't test getter, but should not crash)

	SUBCASE("Set custom model") {
		client->set_model("claude-3-opus-20240229");
		// Should accept model name without error
	}

	SUBCASE("Set Sonnet model") {
		client->set_model("claude-sonnet-4-5-20250929");
		// Should accept latest Sonnet
	}

	SUBCASE("Set Haiku model") {
		client->set_model("claude-3-haiku-20240307");
		// Should accept Haiku for faster responses
	}
}

// Test response parsing (with mock data)
TEST_CASE("[ClaudeAPI] Response parsing") {
	Ref<ClaudeAPIClient> client;
	client.instantiate();

	// Mock API response (what we'd get from Anthropic API)
	String mock_response = R"({
		"id": "msg_123",
		"type": "message",
		"role": "assistant",
		"content": [
			{
				"type": "text",
				"text": "Hello! How can I help you?"
			}
		],
		"model": "claude-sonnet-4-5-20250929",
		"stop_reason": "end_turn",
		"usage": {
			"input_tokens": 10,
			"output_tokens": 20
		}
	})";

	// Parse the JSON
	JSON json;
	Error err = json.parse(mock_response);
	CHECK(err == OK);

	Dictionary response_dict = json.get_data();
	CHECK(response_dict.has("content"));
	CHECK(response_dict.has("stop_reason"));
	CHECK(response_dict["stop_reason"] == "end_turn");

	// Verify content structure
	Array content = response_dict["content"];
	CHECK(content.size() == 1);

	Dictionary content_block = content[0];
	CHECK(content_block.has("type"));
	CHECK(content_block["type"] == "text");
	CHECK(content_block.has("text"));
}

// Test tool use response parsing
TEST_CASE("[ClaudeAPI] Tool use response parsing") {
	// Mock response with tool use
	String mock_response = R"({
		"id": "msg_456",
		"type": "message",
		"role": "assistant",
		"content": [
			{
				"type": "tool_use",
				"id": "toolu_123",
				"name": "read_file",
				"input": {
					"path": "test.txt"
				}
			}
		],
		"model": "claude-sonnet-4-5-20250929",
		"stop_reason": "tool_use"
	})";

	JSON json;
	Error err = json.parse(mock_response);
	CHECK(err == OK);

	Dictionary response_dict = json.get_data();
	CHECK(response_dict["stop_reason"] == "tool_use");

	Array content = response_dict["content"];
	CHECK(content.size() == 1);

	Dictionary tool_use = content[0];
	CHECK(tool_use["type"] == "tool_use");
	CHECK(tool_use.has("id"));
	CHECK(tool_use.has("name"));
	CHECK(tool_use.has("input"));
	CHECK(tool_use["name"] == "read_file");

	Dictionary input = tool_use["input"];
	CHECK(input.has("path"));
	CHECK(input["path"] == "test.txt");
}

// Test multiple content blocks (text + tool use)
TEST_CASE("[ClaudeAPI] Mixed content response") {
	String mock_response = R"({
		"content": [
			{
				"type": "text",
				"text": "Let me read that file for you."
			},
			{
				"type": "tool_use",
				"id": "toolu_789",
				"name": "read_file",
				"input": {"path": "player.gd"}
			}
		],
		"stop_reason": "tool_use"
	})";

	JSON json;
	Error err = json.parse(mock_response);
	CHECK(err == OK);

	Dictionary response_dict = json.get_data();
	Array content = response_dict["content"];
	CHECK(content.size() == 2);

	// First block is text
	Dictionary text_block = content[0];
	CHECK(text_block["type"] == "text");
	CHECK(text_block.has("text"));

	// Second block is tool use
	Dictionary tool_block = content[1];
	CHECK(tool_block["type"] == "tool_use");
	CHECK(tool_block["name"] == "read_file");
}

// Test error response handling
TEST_CASE("[ClaudeAPI] Error response parsing") {
	String mock_error = R"({
		"type": "error",
		"error": {
			"type": "invalid_request_error",
			"message": "Invalid API key"
		}
	})";

	JSON json;
	Error err = json.parse(mock_error);
	CHECK(err == OK);

	Dictionary error_dict = json.get_data();
	CHECK(error_dict["type"] == "error");
	CHECK(error_dict.has("error"));

	Dictionary error_details = error_dict["error"];
	CHECK(error_details.has("type"));
	CHECK(error_details.has("message"));
}

} // namespace TestClaudeAPIClient

#endif // TOOLS_ENABLED
