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

Error ClaudeAPIClient::_connect_to_api() {
	if (is_connected) {
		return OK;
	}

	Error err = http_client->connect_to_host("api.anthropic.com", 443, true);
	if (err != OK) {
		return err;
	}

	// Wait for connection
	while (http_client->get_status() == HTTPClient::STATUS_CONNECTING ||
			http_client->get_status() == HTTPClient::STATUS_RESOLVING) {
		http_client->poll();
		OS::get_singleton()->delay_usec(100000); // 100ms
	}

	if (http_client->get_status() != HTTPClient::STATUS_CONNECTED) {
		return FAILED;
	}

	is_connected = true;
	return OK;
}

Error ClaudeAPIClient::_send_request(const String &p_json_payload) {
	Vector<String> headers;
	headers.push_back("Content-Type: application/json");
	headers.push_back("x-api-key: " + api_key);
	headers.push_back("anthropic-version: 2023-06-01");

	PackedByteArray body = p_json_payload.to_utf8_buffer();

	Error err = http_client->request(
			HTTPClient::METHOD_POST,
			"/v1/messages",
			headers,
			body);

	return err;
}

String ClaudeAPIClient::_read_response() {
	// Wait for response
	while (http_client->get_status() == HTTPClient::STATUS_REQUESTING) {
		http_client->poll();
		OS::get_singleton()->delay_usec(50000); // 50ms
	}

	if (http_client->get_status() != HTTPClient::STATUS_BODY &&
			http_client->get_status() != HTTPClient::STATUS_CONNECTED) {
		return "";
	}

	// Read response body
	PackedByteArray response_body;

	while (http_client->get_status() == HTTPClient::STATUS_BODY) {
		http_client->poll();

		PackedByteArray chunk = http_client->read_response_body_chunk();
		if (chunk.size() > 0) {
			response_body.append_array(chunk);
		} else {
			OS::get_singleton()->delay_usec(10000); // 10ms
		}
	}

	return String::utf8((const char *)response_body.ptr(), response_body.size());
}

void ClaudeAPIClient::_parse_tool_calls(const Dictionary &p_response, Array &r_tool_calls) {
	if (!p_response.has("content")) {
		return;
	}

	Array content = p_response["content"];
	for (int i = 0; i < content.size(); i++) {
		Dictionary content_block = content[i];

		if (content_block.get("type", "") == "tool_use") {
			Dictionary tool_call;
			tool_call["id"] = content_block.get("id", "");
			tool_call["name"] = content_block.get("name", "");
			tool_call["input"] = content_block.get("input", Dictionary());
			r_tool_calls.push_back(tool_call);
		}
	}
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

	// Connect to API
	Error err = _connect_to_api();
	if (err != OK) {
		response.content = "Error: Failed to connect to Anthropic API. Check your internet connection.";
		response.finished = true;
		is_connected = false;
		return response;
	}

	// Send request
	err = _send_request(json_payload);
	if (err != OK) {
		response.content = "Error: Failed to send request to Anthropic API.";
		response.finished = true;
		is_connected = false;
		return response;
	}

	// Read response
	String response_text = _read_response();
	if (response_text.is_empty()) {
		response.content = "Error: Empty response from Anthropic API.";
		response.finished = true;
		is_connected = false;
		return response;
	}

	// Parse JSON response
	JSON json;
	err = json.parse(response_text);
	if (err != OK) {
		response.content = "Error: Invalid JSON response from API:\n" + response_text.substr(0, 500);
		response.finished = true;
		return response;
	}

	Dictionary response_data = json.get_data();

	// Check for API errors
	if (response_data.has("error")) {
		Dictionary error = response_data["error"];
		String error_message = error.get("message", "Unknown error");
		response.content = "API Error: " + error_message;
		response.finished = true;
		return response;
	}

	// Extract text content
	if (response_data.has("content")) {
		Array content = response_data["content"];
		for (int i = 0; i < content.size(); i++) {
			Dictionary block = content[i];
			if (block.get("type", "") == "text") {
				response.content += block.get("text", "");
			}
		}
	}

	// Extract tool calls
	_parse_tool_calls(response_data, response.tool_calls);

	// Extract stop reason
	response.stop_reason = response_data.get("stop_reason", "");
	response.finished = true;

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
