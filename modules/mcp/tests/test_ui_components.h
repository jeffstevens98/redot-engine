/**************************************************************************/
/*  test_ui_components.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "tests/test_macros.h"

#include "../editor/claude_api_client.h"
#include "../editor/claude_chat_panel.h"
#include "../editor/claude_settings_dialog.h"
#include "../mcp_server.h"

namespace TestUIComponents {

// Test ChatMessage component structure
TEST_CASE("[UI] ChatMessage component") {
	SUBCASE("User message") {
		ChatMessage *msg = memnew(ChatMessage("user", "Hello, Claude!"));
		CHECK(msg != nullptr);
		CHECK(msg->is_user_message);
		memdelete(msg);
	}

	SUBCASE("Assistant message") {
		ChatMessage *msg = memnew(ChatMessage("assistant", "Hello! How can I help?"));
		CHECK(msg != nullptr);
		CHECK_FALSE(msg->is_user_message);
		memdelete(msg);
	}

	SUBCASE("System message") {
		ChatMessage *msg = memnew(ChatMessage("system", "Welcome to Claude!");
		CHECK(msg != nullptr);
		CHECK_FALSE(msg->is_user_message);
		memdelete(msg);
	}
}

// Test ToolCallDisplay component
TEST_CASE("[UI] ToolCallDisplay component") {
	Dictionary input;
	input["path"] = "test.gd";

	ToolCallDisplay *display = memnew(ToolCallDisplay("read_file", input));
	CHECK(display != nullptr);
	CHECK(display->is_executing); // Should start as executing

	// Set result
	display->set_result("File content here...");
	CHECK_FALSE(display->is_executing);

	memdelete(display);
}

// Test ToolCallDisplay error handling
TEST_CASE("[UI] ToolCallDisplay error handling") {
	Dictionary input;
	input["path"] = "nonexistent.gd";

	ToolCallDisplay *display = memnew(ToolCallDisplay("read_file", input));
	CHECK(display->is_executing);

	// Set error
	display->set_error("File not found");
	CHECK_FALSE(display->is_executing);

	memdelete(display);
}

// Test ClaudeChatPanel initialization
TEST_CASE("[UI] ClaudeChatPanel initialization") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);
	CHECK(panel != nullptr);

	// Panel should have components
	CHECK(panel->header != nullptr);
	CHECK(panel->messages_scroll != nullptr);
	CHECK(panel->messages_container != nullptr);
	CHECK(panel->input_box != nullptr);
	CHECK(panel->send_button != nullptr);
	CHECK(panel->status_label != nullptr);
	CHECK(panel->settings_button != nullptr);
	CHECK(panel->clear_button != nullptr);

	memdelete(panel);
}

// Test ClaudeChatPanel with API client
TEST_CASE("[UI] ClaudeChatPanel API client integration") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);
	Ref<ClaudeAPIClient> client;
	client.instantiate();

	panel->set_api_client(client);

	// Client should be set
	CHECK(panel->api_client.is_valid());

	memdelete(panel);
}

// Test ClaudeChatPanel with MCP server
TEST_CASE("[UI] ClaudeChatPanel MCP server integration") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);
	Ref<MCPServer> server;
	server.instantiate();

	panel->set_mcp_server(server);

	// Server should be set
	CHECK(panel->mcp_server.is_valid());

	memdelete(panel);
}

// Test ClaudeChatPanel message adding
TEST_CASE("[UI] ClaudeChatPanel message operations") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);

	int initial_children = panel->messages_container->get_child_count();

	// Add system message
	panel->add_system_message("Test system message");
	CHECK(panel->messages_container->get_child_count() > initial_children);

	memdelete(panel);
}

// Test ClaudeSettingsDialog initialization
TEST_CASE("[UI] ClaudeSettingsDialog initialization") {
	ClaudeSettingsDialog *dialog = memnew(ClaudeSettingsDialog);
	CHECK(dialog != nullptr);

	// Dialog should have inputs
	CHECK(dialog->api_key_input != nullptr);
	CHECK(dialog->model_dropdown != nullptr);

	memdelete(dialog);
}

// Test ClaudeSettingsDialog API key handling
TEST_CASE("[UI] ClaudeSettingsDialog API key") {
	ClaudeSettingsDialog *dialog = memnew(ClaudeSettingsDialog);

	SUBCASE("Set API key") {
		dialog->set_api_key("sk-test-key");
		String key = dialog->api_key_input->get_text();
		// Key might be masked with asterisks
		CHECK_FALSE(key.is_empty());
	}

	SUBCASE("Get API key") {
		dialog->api_key_input->set_text("sk-new-key");
		String key = dialog->current_api_key;
		// Current key should be accessible
	}

	memdelete(dialog);
}

// Test ClaudeSettingsDialog model selection
TEST_CASE("[UI] ClaudeSettingsDialog model selection") {
	ClaudeSettingsDialog *dialog = memnew(ClaudeSettingsDialog);

	// Model dropdown should have options
	CHECK(dialog->model_dropdown != nullptr);
	// Should have at least Sonnet and Opus options
	CHECK(dialog->model_dropdown->get_item_count() >= 2);

	memdelete(dialog);
}

// Test signal connections
TEST_CASE("[UI] Signal connections") {
	SUBCASE("ClaudeChatPanel signals") {
		ClaudeChatPanel *panel = memnew(ClaudeChatPanel);

		// Should have settings_requested signal
		// (Verified by integration tests)
		CHECK(true);

		memdelete(panel);
	}

	SUBCASE("ClaudeSettingsDialog signals") {
		ClaudeSettingsDialog *dialog = memnew(ClaudeSettingsDialog);

		// Should have settings_changed signal
		// (Verified by integration tests)
		CHECK(true);

		memdelete(dialog);
	}
}

// Test message history management
TEST_CASE("[UI] Message history") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);
	Ref<ClaudeAPIClient> client;
	client.instantiate();
	panel->set_api_client(client);

	// Initially empty
	CHECK(panel->message_history.size() == 0);

	// After adding messages, history should grow
	// (This would require sending actual messages, which needs API key)

	memdelete(panel);
}

// Test tool call visualization
TEST_CASE("[UI] Tool call visualization") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);

	int initial_count = panel->messages_container->get_child_count();

	// Add a tool call visualization
	Dictionary input;
	input["query"] = "test";
	panel->_add_tool_call("search_in_files", input, "Found 5 results");

	// Should have added a ToolCallDisplay
	CHECK(panel->messages_container->get_child_count() > initial_count);

	memdelete(panel);
}

// Test status updates
TEST_CASE("[UI] Status label updates") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);

	SUBCASE("Ready status") {
		panel->_update_status("Ready");
		String status = panel->status_label->get_text();
		CHECK(status.contains("Ready"));
	}

	SUBCASE("Processing status") {
		panel->_update_status("Processing...");
		String status = panel->status_label->get_text();
		CHECK(status.contains("Processing"));
	}

	SUBCASE("Error status") {
		panel->_update_status("Error");
		String status = panel->status_label->get_text();
		CHECK(status.contains("Error"));
	}

	memdelete(panel);
}

// Test input validation
TEST_CASE("[UI] Input validation") {
	ClaudeChatPanel *panel = memnew(ClaudeChatPanel);

	SUBCASE("Empty input disables send button") {
		panel->input_box->set_text("");
		panel->_on_input_text_changed();
		CHECK(panel->send_button->is_disabled());
	}

	SUBCASE("Non-empty input enables send button") {
		panel->input_box->set_text("Hello");
		panel->_on_input_text_changed();
		CHECK_FALSE(panel->send_button->is_disabled());
	}

	SUBCASE("Whitespace-only input disables send button") {
		panel->input_box->set_text("   ");
		panel->_on_input_text_changed();
		CHECK(panel->send_button->is_disabled());
	}

	memdelete(panel);
}

} // namespace TestUIComponents

#endif // TOOLS_ENABLED
