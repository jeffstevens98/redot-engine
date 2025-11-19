/**************************************************************************/
/*  mcp_server.cpp                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2024-present Redot Engine contributors                   */
/*                                          (see REDOT_AUTHORS.md)        */
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "mcp_server.h"

#include "core/io/json.h"
#include "core/version.h"

MCPServer::MCPServer() {
	server_info["name"] = "redot-mcp-server";
	server_info["version"] = VERSION_FULL_NAME;
}

MCPServer::~MCPServer() {
}

void MCPServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_tool", "name", "description", "schema", "callback"), &MCPServer::register_tool);
	ClassDB::bind_method(D_METHOD("register_resource", "uri", "name", "description", "mime_type", "reader"), &MCPServer::register_resource);
	ClassDB::bind_method(D_METHOD("process_message", "json"), &MCPServer::process_message);
	ClassDB::bind_method(D_METHOD("start"), &MCPServer::start);
	ClassDB::bind_method(D_METHOD("stop"), &MCPServer::stop);
	ClassDB::bind_method(D_METHOD("is_running"), &MCPServer::is_running);
	ClassDB::bind_method(D_METHOD("get_protocol_version"), &MCPServer::get_protocol_version);
	ClassDB::bind_method(D_METHOD("get_server_info"), &MCPServer::get_server_info);

	BIND_ENUM_CONSTANT(PARSE_ERROR);
	BIND_ENUM_CONSTANT(INVALID_REQUEST);
	BIND_ENUM_CONSTANT(METHOD_NOT_FOUND);
	BIND_ENUM_CONSTANT(INVALID_PARAMS);
	BIND_ENUM_CONSTANT(INTERNAL_ERROR);
}

void MCPServer::register_tool(const String &p_name, const String &p_description, const Dictionary &p_schema, const Callable &p_callback) {
	ToolDefinition tool;
	tool.name = p_name;
	tool.description = p_description;
	tool.input_schema = p_schema;
	tool.callback = p_callback;
	tools[p_name] = tool;
}

void MCPServer::register_resource(const String &p_uri, const String &p_name, const String &p_description, const String &p_mime_type, const Callable &p_reader) {
	ResourceDefinition resource;
	resource.uri = p_uri;
	resource.name = p_name;
	resource.description = p_description;
	resource.mime_type = p_mime_type;
	resource.reader = p_reader;
	resources[p_uri] = resource;
}

Dictionary MCPServer::_make_response(const Variant &p_result, const Variant &p_id) {
	Dictionary response;
	response["jsonrpc"] = "2.0";
	response["id"] = p_id;
	response["result"] = p_result;
	return response;
}

Dictionary MCPServer::_make_error(int p_code, const String &p_message, const Variant &p_id) {
	Dictionary response;
	response["jsonrpc"] = "2.0";
	response["id"] = p_id;

	Dictionary error;
	error["code"] = p_code;
	error["message"] = p_message;
	response["error"] = error;

	return response;
}

Variant MCPServer::_process_request(const Dictionary &p_request) {
	// Validate JSON-RPC 2.0 structure
	if (!p_request.has("jsonrpc") || p_request["jsonrpc"] != "2.0") {
		return _make_error(INVALID_REQUEST, "Invalid JSON-RPC version");
	}

	if (!p_request.has("method")) {
		return _make_error(INVALID_REQUEST, "Missing method field");
	}

	String method = p_request["method"];
	Variant id = p_request.get("id", Variant());
	Dictionary params = p_request.get("params", Dictionary());

	// Route to appropriate handler
	if (method == "initialize") {
		return _handle_initialize(params, id);
	} else if (method == "tools/list") {
		return _handle_tools_list(id);
	} else if (method == "tools/call") {
		return _handle_tools_call(params, id);
	} else if (method == "resources/list") {
		return _handle_resources_list(id);
	} else if (method == "resources/read") {
		return _handle_resources_read(params, id);
	} else if (method == "initialized") {
		// Client acknowledgment after initialize - no response needed
		return Variant();
	} else {
		return _make_error(METHOD_NOT_FOUND, "Method not found: " + method, id);
	}
}

Dictionary MCPServer::_handle_initialize(const Dictionary &p_params, const Variant &p_id) {
	// Store client info
	if (p_params.has("clientInfo")) {
		client_info = p_params["clientInfo"];
	}

	// Validate protocol version
	if (p_params.has("protocolVersion")) {
		String requested_version = p_params["protocolVersion"];
		// For now, accept any version (could add version negotiation logic here)
	}

	// Build capabilities dictionary
	Dictionary capabilities;

	if (supports_tools) {
		capabilities["tools"] = Dictionary();
	}

	if (supports_resources) {
		capabilities["resources"] = Dictionary();
	}

	// Build result
	Dictionary result;
	result["protocolVersion"] = protocol_version;
	result["serverInfo"] = server_info;
	result["capabilities"] = capabilities;

	initialized = true;

	return _make_response(result, p_id);
}

Dictionary MCPServer::_handle_tools_list(const Variant &p_id) {
	if (!initialized) {
		return _make_error(INTERNAL_ERROR, "Server not initialized", p_id);
	}

	Array tools_array;
	for (const KeyValue<String, ToolDefinition> &E : tools) {
		Dictionary tool_dict;
		tool_dict["name"] = E.value.name;
		tool_dict["description"] = E.value.description;
		tool_dict["inputSchema"] = E.value.input_schema;
		tools_array.push_back(tool_dict);
	}

	Dictionary result;
	result["tools"] = tools_array;

	return _make_response(result, p_id);
}

Dictionary MCPServer::_handle_tools_call(const Dictionary &p_params, const Variant &p_id) {
	if (!initialized) {
		return _make_error(INTERNAL_ERROR, "Server not initialized", p_id);
	}

	if (!p_params.has("name")) {
		return _make_error(INVALID_PARAMS, "Missing tool name", p_id);
	}

	String tool_name = p_params["name"];

	if (!tools.has(tool_name)) {
		return _make_error(INVALID_PARAMS, "Unknown tool: " + tool_name, p_id);
	}

	Dictionary arguments = p_params.get("arguments", Dictionary());

	// Call the tool
	const ToolDefinition &tool = tools[tool_name];
	Array call_args;
	call_args.push_back(arguments);

	Variant tool_result = tool.callback.callv(call_args);

	// Format result
	Dictionary result;
	Array content_array;

	Dictionary content_item;
	content_item["type"] = "text";
	content_item["text"] = tool_result.stringify();
	content_array.push_back(content_item);

	result["content"] = content_array;

	return _make_response(result, p_id);
}

Dictionary MCPServer::_handle_resources_list(const Variant &p_id) {
	if (!initialized) {
		return _make_error(INTERNAL_ERROR, "Server not initialized", p_id);
	}

	Array resources_array;
	for (const KeyValue<String, ResourceDefinition> &E : resources) {
		Dictionary resource_dict;
		resource_dict["uri"] = E.value.uri;
		resource_dict["name"] = E.value.name;
		resource_dict["description"] = E.value.description;
		resource_dict["mimeType"] = E.value.mime_type;
		resources_array.push_back(resource_dict);
	}

	Dictionary result;
	result["resources"] = resources_array;

	return _make_response(result, p_id);
}

Dictionary MCPServer::_handle_resources_read(const Dictionary &p_params, const Variant &p_id) {
	if (!initialized) {
		return _make_error(INTERNAL_ERROR, "Server not initialized", p_id);
	}

	if (!p_params.has("uri")) {
		return _make_error(INVALID_PARAMS, "Missing resource URI", p_id);
	}

	String uri = p_params["uri"];

	if (!resources.has(uri)) {
		return _make_error(INVALID_PARAMS, "Unknown resource: " + uri, p_id);
	}

	// Read the resource
	const ResourceDefinition &resource = resources[uri];
	Variant resource_content = resource.reader.call();

	// Format result
	Dictionary result;
	Array contents_array;

	Dictionary content_item;
	content_item["uri"] = uri;
	content_item["mimeType"] = resource.mime_type;
	content_item["text"] = resource_content.stringify();
	contents_array.push_back(content_item);

	result["contents"] = contents_array;

	return _make_response(result, p_id);
}

String MCPServer::process_message(const String &p_json) {
	if (p_json.is_empty()) {
		return "";
	}

	JSON json;
	Error err = json.parse(p_json);

	if (err != OK) {
		Dictionary error = _make_error(PARSE_ERROR, "JSON parse error: " + json.get_error_message());
		return JSON::stringify(error);
	}

	Variant data = json.get_data();

	if (data.get_type() != Variant::DICTIONARY) {
		Dictionary error = _make_error(INVALID_REQUEST, "Request must be a JSON object");
		return JSON::stringify(error);
	}

	Variant response = _process_request(data);

	// Return empty string for notifications (no response)
	if (response.get_type() == Variant::NIL) {
		return "";
	}

	return JSON::stringify(response);
}

void MCPServer::start() {
	initialized = false;
}

void MCPServer::stop() {
	initialized = false;
	tools.clear();
	resources.clear();
}
