/**************************************************************************/
/*  claude_settings_dialog.h                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#pragma once

#ifdef TOOLS_ENABLED

#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/rich_text_label.h"

// Settings dialog for Claude integration
class ClaudeSettingsDialog : public AcceptDialog {
	GDCLASS(ClaudeSettingsDialog, AcceptDialog)

private:
	LineEdit *api_key_input = nullptr;
	OptionButton *model_selector = nullptr;
	RichTextLabel *info_label = nullptr;
	Button *test_button = nullptr;
	Label *test_result_label = nullptr;

	String current_api_key;
	String current_model;

	void _on_test_connection();
	void _on_confirmed();

protected:
	static void _bind_methods();

public:
	ClaudeSettingsDialog();

	void set_api_key(const String &p_key);
	String get_api_key() const;

	void set_model(const String &p_model);
	String get_model() const;

	Signal signal_settings_changed;
};

#endif // TOOLS_ENABLED
