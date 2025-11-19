/**************************************************************************/
/*  mcp_tools.cpp                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/

#include "mcp_tools.h"

#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "scene/main/node.h"

Dictionary MCPTools::make_schema_object(const Array &p_required, const Dictionary &p_properties) {
	Dictionary schema;
	schema["type"] = "object";
	schema["properties"] = p_properties;
	schema["required"] = p_required;
	return schema;
}

Dictionary MCPTools::make_property_string(const String &p_description) {
	Dictionary prop;
	prop["type"] = "string";
	prop["description"] = p_description;
	return prop;
}

String MCPTools::tool_read_file(const Dictionary &p_args) {
	if (!p_args.has("path")) {
		return "Error: Missing 'path' argument";
	}

	String path = p_args["path"];

	// Resolve to project directory
	if (!path.begins_with("res://") && !path.begins_with("user://")) {
		path = "res://" + path;
	}

	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
	if (file.is_null()) {
		return "Error: Cannot open file: " + path;
	}

	String content = file->get_as_utf8_string();
	file->close();

	Dictionary result;
	result["path"] = path;
	result["content"] = content;
	result["size"] = content.length();

	return result.to_json_string();
}

String MCPTools::tool_list_scenes(const Dictionary &p_args) {
	String search_path = p_args.get("path", "res://");

	Ref<DirAccess> dir = DirAccess::open(search_path);
	if (dir.is_null()) {
		return "Error: Cannot open directory: " + search_path;
	}

	Array scenes;
	MCPTools::_list_scenes_recursive(dir, search_path, scenes);

	Dictionary result;
	result["scenes"] = scenes;
	result["count"] = scenes.size();

	return result.to_json_string();
}

void MCPTools::_list_scenes_recursive(Ref<DirAccess> p_dir, const String &p_path, Array &r_scenes) {
	p_dir->list_dir_begin();
	String file_name = p_dir->get_next();

	while (!file_name.is_empty()) {
		if (file_name == "." || file_name == "..") {
			file_name = p_dir->get_next();
			continue;
		}

		String full_path = p_path.path_join(file_name);

		if (p_dir->current_is_dir()) {
			Ref<DirAccess> sub_dir = DirAccess::open(full_path);
			if (sub_dir.is_valid()) {
				_list_scenes_recursive(sub_dir, full_path, r_scenes);
			}
		} else if (file_name.ends_with(".tscn") || file_name.ends_with(".scn")) {
			Dictionary scene_info;
			scene_info["path"] = full_path;
			scene_info["name"] = file_name.get_basename();
			r_scenes.push_back(scene_info);
		}

		file_name = p_dir->get_next();
	}

	p_dir->list_dir_end();
}

String MCPTools::tool_get_project_settings(const Dictionary &p_args) {
	Dictionary result;

	result["project_name"] = GLOBAL_GET("application/config/name");
	result["main_scene"] = GLOBAL_GET("application/run/main_scene");
	result["project_dir"] = ProjectSettings::get_singleton()->get_resource_path();

	// Get display settings
	Dictionary display;
	display["width"] = GLOBAL_GET("display/window/size/viewport_width");
	display["height"] = GLOBAL_GET("display/window/size/viewport_height");
	display["resizable"] = GLOBAL_GET("display/window/size/resizable");
	result["display"] = display;

	return result.to_json_string();
}

String MCPTools::tool_search_in_files(const Dictionary &p_args) {
	if (!p_args.has("query")) {
		return "Error: Missing 'query' argument";
	}

	String query = p_args["query"];
	String search_path = p_args.get("path", "res://");
	String extension = p_args.get("extension", "gd"); // Default to GDScript files

	Ref<DirAccess> dir = DirAccess::open(search_path);
	if (dir.is_null()) {
		return "Error: Cannot open directory: " + search_path;
	}

	Array results;
	MCPTools::_search_in_files_recursive(dir, search_path, query, extension, results);

	Dictionary result;
	result["query"] = query;
	result["results"] = results;
	result["count"] = results.size();

	return result.to_json_string();
}

void MCPTools::_search_in_files_recursive(Ref<DirAccess> p_dir, const String &p_path, const String &p_query, const String &p_extension, Array &r_results) {
	p_dir->list_dir_begin();
	String file_name = p_dir->get_next();

	while (!file_name.is_empty()) {
		if (file_name == "." || file_name == "..") {
			file_name = p_dir->get_next();
			continue;
		}

		String full_path = p_path.path_join(file_name);

		if (p_dir->current_is_dir()) {
			// Skip .godot directory
			if (file_name != ".godot" && file_name != ".import") {
				Ref<DirAccess> sub_dir = DirAccess::open(full_path);
				if (sub_dir.is_valid()) {
					_search_in_files_recursive(sub_dir, full_path, p_query, p_extension, r_results);
				}
			}
		} else if (file_name.ends_with("." + p_extension)) {
			// Search in this file
			Ref<FileAccess> file = FileAccess::open(full_path, FileAccess::READ);
			if (file.is_valid()) {
				String content = file->get_as_utf8_string();
				file->close();

				if (content.contains(p_query)) {
					// Find line numbers
					PackedStringArray lines = content.split("\n");
					Array matches;

					for (int i = 0; i < lines.size(); i++) {
						if (lines[i].contains(p_query)) {
							Dictionary match;
							match["line"] = i + 1;
							match["text"] = lines[i].strip_edges();
							matches.push_back(match);
						}
					}

					Dictionary file_result;
					file_result["path"] = full_path;
					file_result["matches"] = matches;
					r_results.push_back(file_result);
				}
			}
		}

		file_name = p_dir->get_next();
	}

	p_dir->list_dir_end();
}

String MCPTools::tool_list_directory(const Dictionary &p_args) {
	String dir_path = p_args.get("path", "res://");
	bool recursive = p_args.get("recursive", false);

	Ref<DirAccess> dir = DirAccess::open(dir_path);
	if (dir.is_null()) {
		return "Error: Cannot open directory: " + dir_path;
	}

	Array files;
	Array directories;

	if (recursive) {
		MCPTools::_list_directory_recursive(dir, dir_path, files, directories);
	} else {
		dir->list_dir_begin();
		String item = dir->get_next();

		while (!item.is_empty()) {
			if (item != "." && item != "..") {
				String full_path = dir_path.path_join(item);

				if (dir->current_is_dir()) {
					directories.push_back(full_path);
				} else {
					Dictionary file_info;
					file_info["path"] = full_path;
					file_info["name"] = item;
					files.push_back(file_info);
				}
			}
			item = dir->get_next();
		}
		dir->list_dir_end();
	}

	Dictionary result;
	result["path"] = dir_path;
	result["files"] = files;
	result["directories"] = directories;
	result["file_count"] = files.size();
	result["dir_count"] = directories.size();

	return result.to_json_string();
}

void MCPTools::_list_directory_recursive(Ref<DirAccess> p_dir, const String &p_path, Array &r_files, Array &r_directories) {
	p_dir->list_dir_begin();
	String item = p_dir->get_next();

	while (!item.is_empty()) {
		if (item == "." || item == ".." || item == ".godot" || item == ".import") {
			item = p_dir->get_next();
			continue;
		}

		String full_path = p_path.path_join(item);

		if (p_dir->current_is_dir()) {
			r_directories.push_back(full_path);
			Ref<DirAccess> sub_dir = DirAccess::open(full_path);
			if (sub_dir.is_valid()) {
				_list_directory_recursive(sub_dir, full_path, r_files, r_directories);
			}
		} else {
			Dictionary file_info;
			file_info["path"] = full_path;
			file_info["name"] = item;
			r_files.push_back(file_info);
		}

		item = p_dir->get_next();
	}

	p_dir->list_dir_end();
}

String MCPTools::tool_get_scene_info(const Dictionary &p_args) {
	if (!p_args.has("path")) {
		return "Error: Missing 'path' argument";
	}

	String scene_path = p_args["path"];

	if (!scene_path.begins_with("res://")) {
		scene_path = "res://" + scene_path;
	}

	// For .tscn files, we can parse the text format
	if (scene_path.ends_with(".tscn")) {
		Ref<FileAccess> file = FileAccess::open(scene_path, FileAccess::READ);
		if (file.is_null()) {
			return "Error: Cannot open scene file: " + scene_path;
		}

		String content = file->get_as_utf8_string();
		file->close();

		// Parse scene structure from TSCN format
		Array nodes;
		PackedStringArray lines = content.split("\n");

		for (const String &line : lines) {
			if (line.begins_with("[node ")) {
				Dictionary node_info;

				// Extract node name
				int name_start = line.find("name=\"");
				if (name_start != -1) {
					name_start += 6;
					int name_end = line.find("\"", name_start);
					node_info["name"] = line.substr(name_start, name_end - name_start);
				}

				// Extract node type
				int type_start = line.find("type=\"");
				if (type_start != -1) {
					type_start += 6;
					int type_end = line.find("\"", type_start);
					node_info["type"] = line.substr(type_start, type_end - type_start);
				}

				// Extract parent
				int parent_start = line.find("parent=\"");
				if (parent_start != -1) {
					parent_start += 8;
					int parent_end = line.find("\"", parent_start);
					node_info["parent"] = line.substr(parent_start, parent_end - parent_start);
				} else {
					node_info["parent"] = "."; // Root node
				}

				if (!node_info.is_empty()) {
					nodes.push_back(node_info);
				}
			}
		}

		Dictionary result;
		result["path"] = scene_path;
		result["nodes"] = nodes;
		result["node_count"] = nodes.size();

		return result.to_json_string();
	}

	return "Error: Only .tscn files are currently supported";
}
