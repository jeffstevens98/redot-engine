/**************************************************************************/
/*  claude_settings_dialog.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "claude_settings_dialog.h"

#ifdef TOOLS_ENABLED

#include "editor/settings/editor_settings.h"
#include "scene/gui/box_container.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"

ClaudeSettingsDialog::ClaudeSettingsDialog() {
	set_title("Claude Assistant Settings");
	set_min_size(Size2(600, 400));

	VBoxContainer *vbox = memnew(VBoxContainer);
	add_child(vbox);

	// Info section
	info_label = memnew(RichTextLabel);
	info_label->set_use_bbcode(true);
	info_label->set_fit_content(true);
	info_label->set_text(
			"[b]Configure Claude AI Assistant[/b]\n\n"
			"To use Claude in Redot, you need an Anthropic API key.\n\n"
			"[i]Get your API key:[/i]\n"
			"1. Visit https://console.anthropic.com\n"
			"2. Sign up or log in\n"
			"3. Go to API Keys section\n"
			"4. Create a new key\n"
			"5. Paste it below\n\n"
			"[color=gray][i]Your API key is stored locally in Redot editor settings.[/i][/color]");
	vbox->add_child(info_label);

	vbox->add_child(memnew(HSeparator));

	// Settings grid
	GridContainer *grid = memnew(GridContainer);
	grid->set_columns(2);
	vbox->add_child(grid);

	// API Key
	Label *api_key_label = memnew(Label);
	api_key_label->set_text("API Key:");
	grid->add_child(api_key_label);

	api_key_input = memnew(LineEdit);
	api_key_input->set_placeholder("sk-ant-...");
	api_key_input->set_secret(true);
	api_key_input->set_custom_minimum_size(Size2(400, 0));
	api_key_input->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	grid->add_child(api_key_input);

	// Model Selection
	Label *model_label = memnew(Label);
	model_label->set_text("Model:");
	grid->add_child(model_label);

	model_selector = memnew(OptionButton);
	model_selector->add_item("Claude Sonnet 4.5 (Recommended)", 0);
	model_selector->add_item("Claude Opus 4", 1);
	model_selector->add_item("Claude Haiku 4", 2);
	model_selector->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	grid->add_child(model_selector);

	vbox->add_child(memnew(HSeparator));

	// Test connection
	HBoxContainer *test_hbox = memnew(HBoxContainer);
	vbox->add_child(test_hbox);

	test_button = memnew(Button);
	test_button->set_text("Test Connection");
	test_button->connect("pressed", callable_mp(this, &ClaudeSettingsDialog::_on_test_connection));
	test_hbox->add_child(test_button);

	test_result_label = memnew(Label);
	test_result_label->set_text("");
	test_result_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	test_hbox->add_child(test_result_label);

	// Load from editor settings
	if (EditorSettings::get_singleton()->has_setting("mcp/claude_api_key")) {
		String saved_key = EditorSettings::get_singleton()->get_setting("mcp/claude_api_key");
		api_key_input->set_text(saved_key);
		current_api_key = saved_key;
	}

	if (EditorSettings::get_singleton()->has_setting("mcp/claude_model")) {
		current_model = EditorSettings::get_singleton()->get_setting("mcp/claude_model");
	} else {
		current_model = "claude-sonnet-4-5-20250929";
	}

	// Connect OK button
	get_ok_button()->connect("pressed", callable_mp(this, &ClaudeSettingsDialog::_on_confirmed));
}

void ClaudeSettingsDialog::_bind_methods() {
	ADD_SIGNAL(MethodInfo("settings_changed", PropertyInfo(Variant::STRING, "api_key"), PropertyInfo(Variant::STRING, "model")));
}

void ClaudeSettingsDialog::_on_test_connection() {
	String test_key = api_key_input->get_text().strip_edges();

	if (test_key.is_empty()) {
		test_result_label->set_text("⚠ Please enter an API key");
		test_result_label->add_theme_color_override("font_color", Color(0.8, 0.6, 0.2));
		return;
	}

	if (!test_key.begins_with("sk-ant-")) {
		test_result_label->set_text("⚠ API key should start with 'sk-ant-'");
		test_result_label->add_theme_color_override("font_color", Color(0.8, 0.6, 0.2));
		return;
	}

	// TODO: Actually test the connection
	test_result_label->set_text("✓ API key format looks valid");
	test_result_label->add_theme_color_override("font_color", Color(0.3, 0.8, 0.3));
}

void ClaudeSettingsDialog::_on_confirmed() {
	current_api_key = api_key_input->get_text().strip_edges();

	// Determine model based on selection
	int selected_idx = model_selector->get_selected();
	switch (selected_idx) {
		case 0:
			current_model = "claude-sonnet-4-5-20250929";
			break;
		case 1:
			current_model = "claude-opus-4-20250514";
			break;
		case 2:
			current_model = "claude-haiku-4-20250611";
			break;
	}

	// Save to editor settings
	EditorSettings::get_singleton()->set_setting("mcp/claude_api_key", current_api_key);
	EditorSettings::get_singleton()->set_setting("mcp/claude_model", current_model);
	EditorSettings::get_singleton()->save();

	// Emit signal
	emit_signal("settings_changed", current_api_key, current_model);
}

void ClaudeSettingsDialog::set_api_key(const String &p_key) {
	current_api_key = p_key;
	api_key_input->set_text(p_key);
}

String ClaudeSettingsDialog::get_api_key() const {
	return current_api_key;
}

void ClaudeSettingsDialog::set_model(const String &p_model) {
	current_model = p_model;
}

String ClaudeSettingsDialog::get_model() const {
	return current_model;
}

#endif // TOOLS_ENABLED
