//
// Created by baizeyv on 1/20/2026.
//

#include "csv_writer.h"

std::string csv_writer::escape_csv(const std::string &text) {
	// # 是否需要引号的标识符
	bool needs_quotes = false;
	if (text.find(',') != std::string::npos || text.find('"') != std::string::npos || text.find('\n') != std::string::npos) {
		needs_quotes = true;
	}

	std::string content = text;

	// # 处理字符串内部已有的双引号
	size_t pos = 0;
	while ((pos = text.find('"', pos)) != std::string::npos) {
		content.replace(pos, 1, "\"\"");
		pos += 2;
	}
	if (needs_quotes) {
		return "\"" + content + "\"";
	}

	return content;
}
