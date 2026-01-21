//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_EXPORT_DATA_HPP
#define KLONDIKESOLVER_EXPORT_DATA_HPP
#include <string>

#include "../io/interface_csv_export.h"

struct export_data : public interface_csv_export {
	/**
	 * * 关卡ID
	 */
	int id;

	/**
	 * * 关卡种子
	 */
	std::string seed;

	/**
	 * * 强制度
	 */
	double forcedness;

	/**
	 * * 低自由度持续时间分数
	 */
	double compression_score;

	/**
	 * * 资源消耗惩罚
	 */
	double stagnation;

	/**
	 * * 信息释放效率 (一般高难局的efficiency很低)
	 */
	double efficiency;

	/**
	 * * 是否可以auto_move解题
	 */
	bool auto_move_flag;

	/**
	 * * 解题结果
	 */
	std::string result;

	std::string to_csv_content() const override;

	std::string to_csv_header() const override;

};

#endif //KLONDIKESOLVER_EXPORT_DATA_HPP