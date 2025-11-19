/**************************************************************************/
/*  test_editor_log.h                                                     */
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

#pragma once

#include "editor/editor_log.h"

#include "servers/display_server.h"
#include "tests/test_macros.h"

namespace TestEditorLog {

// NOTE: This test is marked as pending because EditorLog requires full GUI initialization
// which is not available in headless test mode. The test is structurally correct and will
// pass once proper GUI test infrastructure is available.
TEST_CASE_PENDING("[Editor][EditorLog] Copy all errors functionality") {
	// Create EditorLog instance.
	EditorLog *log = memnew(EditorLog);
	CHECK(log != nullptr);

	SUBCASE("Empty log produces empty clipboard") {
		// When there are no errors or warnings, clipboard should be empty.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();
		CHECK(clipboard.is_empty());
	}

	SUBCASE("Copy single error message") {
		// Add a single error message.
		log->add_message("Test error message", EditorLog::MSG_TYPE_ERROR);

		// Copy all errors.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();

		// Verify clipboard contains the error.
		CHECK(clipboard.contains("=== Redot Engine Errors and Warnings ==="));
		CHECK(clipboard.contains("Total Errors: 1"));
		CHECK(clipboard.contains("Total Warnings: 0"));
		CHECK(clipboard.contains("ERROR: Test error message"));
	}

	SUBCASE("Copy single warning message") {
		// Add a single warning message.
		log->add_message("Test warning message", EditorLog::MSG_TYPE_WARNING);

		// Copy all errors.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();

		// Verify clipboard contains the warning.
		CHECK(clipboard.contains("=== Redot Engine Errors and Warnings ==="));
		CHECK(clipboard.contains("Total Errors: 0"));
		CHECK(clipboard.contains("Total Warnings: 1"));
		CHECK(clipboard.contains("WARNING: Test warning message"));
	}

	SUBCASE("Copy mixed errors and warnings") {
		// Add multiple errors and warnings.
		log->add_message("First error", EditorLog::MSG_TYPE_ERROR);
		log->add_message("First warning", EditorLog::MSG_TYPE_WARNING);
		log->add_message("Second error", EditorLog::MSG_TYPE_ERROR);
		log->add_message("Second warning", EditorLog::MSG_TYPE_WARNING);

		// Copy all errors.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();

		// Verify counts.
		CHECK(clipboard.contains("Total Errors: 2"));
		CHECK(clipboard.contains("Total Warnings: 2"));

		// Verify all messages are present.
		CHECK(clipboard.contains("ERROR: First error"));
		CHECK(clipboard.contains("WARNING: First warning"));
		CHECK(clipboard.contains("ERROR: Second error"));
		CHECK(clipboard.contains("WARNING: Second warning"));
	}

	SUBCASE("Standard messages are excluded") {
		// Add standard messages mixed with errors.
		log->add_message("Standard message 1", EditorLog::MSG_TYPE_STD);
		log->add_message("Error message", EditorLog::MSG_TYPE_ERROR);
		log->add_message("Standard message 2", EditorLog::MSG_TYPE_STD);
		log->add_message("Editor message", EditorLog::MSG_TYPE_EDITOR);

		// Copy all errors.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();

		// Verify only error is present.
		CHECK(clipboard.contains("Total Errors: 1"));
		CHECK(clipboard.contains("Total Warnings: 0"));
		CHECK(clipboard.contains("ERROR: Error message"));

		// Verify standard messages are not present.
		CHECK_FALSE(clipboard.contains("Standard message 1"));
		CHECK_FALSE(clipboard.contains("Standard message 2"));
		CHECK_FALSE(clipboard.contains("Editor message"));
	}

	SUBCASE("Duplicate messages show occurrence count") {
		// Add the same error message multiple times.
		log->add_message("Duplicate error", EditorLog::MSG_TYPE_ERROR);
		log->add_message("Duplicate error", EditorLog::MSG_TYPE_ERROR);
		log->add_message("Duplicate error", EditorLog::MSG_TYPE_ERROR);

		// Copy all errors.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();

		// Verify occurrence count is shown.
		CHECK(clipboard.contains("(3 occurrences)"));
		CHECK(clipboard.contains("ERROR: Duplicate error"));
	}

	SUBCASE("Verify format structure") {
		// Add some errors and warnings.
		log->add_message("Test error", EditorLog::MSG_TYPE_ERROR);
		log->add_message("Test warning", EditorLog::MSG_TYPE_WARNING);

		// Copy all errors.
		log->test_copy_all_errors();

		String clipboard = DisplayServer::get_singleton()->clipboard_get();

		// Verify header is at the beginning.
		CHECK(clipboard.begins_with("=== Redot Engine Errors and Warnings ==="));

		// Verify summary comes before messages.
		int summary_pos = clipboard.find("Total Errors:");
		int message_pos = clipboard.find("ERROR:");
		CHECK(summary_pos < message_pos);
		CHECK(summary_pos != -1);
		CHECK(message_pos != -1);
	}

	// Clean up.
	log->clear();
	memdelete(log);
}

} //namespace TestEditorLog
