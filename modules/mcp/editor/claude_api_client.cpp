/**************************************************************************/
/*  claude_api_client.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "claude_api_client.h"

#ifdef TOOLS_ENABLED

#include "core/io/json.h"
#include "core/os/os.h"

ClaudeAPIClient::ClaudeAPIClient() {
	http_client.instantiate();
}

ClaudeAPIClient::~ClaudeAPIClient() {
}

void ClaudeAPIClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_api_key", "key"), &ClaudeAPIClient::set_api_key);
	ClassDB::bind_method(D_METHOD("get_api_key"), &ClaudeAPIClient::get_api_key);
	ClassDB::bind_method(D_METHOD("has_api_key"), &ClaudeAPIClient::has_api_key);
}

void ClaudeAPIClient::set_api_key(const String &p_key) {
	api_key = p_key;
}

Dictionary ClaudeAPIClient::_build_request_payload(const Array &p_messages, const Array &p_tools) {
	Dictionary payload;
	payload["model"] = model;
	payload["max_tokens"] = 4096;
	payload["messages"] = p_messages;

	if (!p_tools.is_empty()) {
		payload["tools"] = p_tools;
	}

	return payload;
}

ClaudeAPIClient::Response ClaudeAPIClient::send_message(const Array &p_messages, const Array &p_tools) {
	Response response;

	if (!has_api_key()) {
		response.content = "Error: API key not configured. Please set your Anthropic API key in the MCP settings.";
		response.finished = true;
		return response;
	}

	// Build the request
	Dictionary payload = _build_request_payload(p_messages, p_tools);
	String json_payload = JSON::stringify(payload);

	// TODO: Implement actual HTTP request to Anthropic API
	// For now, return a placeholder response
	response.content = "Claude API integration in progress. The HTTP client will send requests to:\n"
					   "POST https://api.anthropic.com/v1/messages\n\n"
					   "Your message:\n" + p_messages.back();
	response.finished = true;
	response.stop_reason = "end_turn";

	return response;
}

void ClaudeAPIClient::_parse_streaming_chunk(const String &p_chunk, Response &r_response) {
	// Parse Server-Sent Events (SSE) format
	// Format: data: {...json...}\n\n

	if (p_chunk.begins_with("data: ")) {
		String json_str = p_chunk.substr(6).strip_edges();

		if (json_str == "[DONE]") {
			r_response.finished = true;
			return;
		}

		JSON json;
		Error err = json.parse(json_str);
		if (err == OK) {
			Dictionary data = json.get_data();

			if (data.has("type")) {
				String type = data["type"];

				if (type == "content_block_delta") {
					Dictionary delta = data["delta"];
					if (delta.has("text")) {
						r_response.content += delta["text"];
					}
				} else if (type == "message_stop") {
					r_response.finished = true;
					if (data.has("stop_reason")) {
						r_response.stop_reason = data["stop_reason"];
					}
				}
			}
		}
	}
}

#endif // TOOLS_ENABLED
