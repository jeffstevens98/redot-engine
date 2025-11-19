/**************************************************************************/
/*  claude_api_client.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "core/io/http_client.h"
#include "core/io/json.h"
#include "core/object/ref_counted.h"

// Claude API client for direct communication with Anthropic's API
// Provides streaming responses and tool use support
class ClaudeAPIClient : public RefCounted {
	GDCLASS(ClaudeAPIClient, RefCounted)

public:
	struct Message {
		String role; // "user" or "assistant"
		String content;
	};

	struct ToolCall {
		String id;
		String name;
		Dictionary input;
	};

	struct Response {
		String content;
		Array tool_calls;
		bool finished = false;
		String stop_reason;
	};

private:
	String api_key;
	String api_base_url = "https://api.anthropic.com/v1/messages";
	String model = "claude-sonnet-4-5-20250929"; // Latest Sonnet 4.5

	Ref<HTTPClient> http_client;
	bool is_streaming = false;
	bool is_connected = false;

	// Build request payload
	Dictionary _build_request_payload(const Array &p_messages, const Array &p_tools);

	// HTTP helpers
	Error _connect_to_api();
	Error _send_request(const String &p_json_payload);
	String _read_response();

	// Parse streaming response
	void _parse_streaming_chunk(const String &p_chunk, Response &r_response);

	// Parse tool calls from response
	void _parse_tool_calls(const Dictionary &p_response, Array &r_tool_calls);

protected:
	static void _bind_methods();

public:
	ClaudeAPIClient();
	~ClaudeAPIClient();

	void set_api_key(const String &p_key);
	String get_api_key() const { return api_key; }

	void set_model(const String &p_model) { model = p_model; }
	String get_model() const { return model; }

	// Send message to Claude with tool support
	Response send_message(const Array &p_messages, const Array &p_tools = Array());

	// Check if API key is configured
	bool has_api_key() const { return !api_key.is_empty(); }
};

#endif // TOOLS_ENABLED
