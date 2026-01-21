//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_INTERFACE_CSV_EXPORT_H
#define KLONDIKESOLVER_INTERFACE_CSV_EXPORT_H
#include <string>

/**
 * * 要导出的数据类型需要继承这个接口
 */
class interface_csv_export {
public:
	virtual ~interface_csv_export() = default;

	virtual std::string to_csv_header() const = 0;
	virtual std::string to_csv_content() const = 0;
};

#endif //KLONDIKESOLVER_INTERFACE_CSV_EXPORT_H