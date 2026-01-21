//
// Created by baizeyv on 1/20/2026.
//
#include "export_data.hpp"

#include "../io/csv_writer.h"

std::string export_data::to_csv_content() const {
	std::string ret;
	ret += std::to_string(id);
	ret += ",";
	ret += seed;
	ret += ",";
	ret += std::to_string(forcedness);
	ret += ",";
	ret += std::to_string(compression_score);
	ret += ",";
	ret += std::to_string(stagnation);
	ret += ",";
	ret += std::to_string(efficiency);
	ret += ",";
	ret += std::to_string(auto_move_flag);
	ret += ",";
	ret += result;
	return ret;
}

std::string export_data::to_csv_header() const {
	return "id,seed,forcedness,compression_score,stagnation,efficiency,auto_move_flag,result";
}
