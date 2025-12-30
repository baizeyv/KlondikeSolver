//
// Created by baizeyv on 12/29/2025.
//

#ifndef KLONDIKESOLVER_BASE_MODE_H
#define KLONDIKESOLVER_BASE_MODE_H
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../helper/helper.h"

class base_mode {
public:
	virtual ~base_mode() = default;

	std::map<std::string, std::function<void(const std::string &)> > *arg_commands;

	std::map<std::string, std::function<void()> > *commands;

	virtual void setup() = 0;

	virtual void enter() {
		std::string input_content;
		while (input()) {
			std::cout << "> " << std::flush;
			std::getline(std::cin, input_content);
			if (input_content.empty())
				// # 输入内容为空
				continue;
			// # 使用 stringstream 解析输入的命令和参数
			std::istringstream stream(input_content);
			std::string command;
			stream >> command; // # 提取命令部分
			helper::ltrim(command);
			if (command.empty())
				continue;

			std::string arguments;
			std::getline(stream, arguments); // # 获取命令之后的所有部分
			helper::ltrim(arguments);

			if (arguments.empty()) {
				// # 没有参数
				if (commands->contains(command)) {
					commands->at(command)(); // # 调用命令函数
				} else {
					std::cout << "Unknown command: " << command << std::endl;
				}
			} else {
				if (arg_commands->contains(command)) {
					arg_commands->at(command)(arguments);
				} else {
					std::cout << "Unknown command: " << command << std::endl;
				}
			}
		}
	}

	virtual bool input() = 0;
};

#endif //KLONDIKESOLVER_BASE_MODE_H
