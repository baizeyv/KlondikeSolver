//
// Created by baizeyv on 1/14/2026.
//

#ifndef KLONDIKESOLVER_ACTION_TYPE_H
#define KLONDIKESOLVER_ACTION_TYPE_H

enum class action_type {
	waste_to_foundation,
	waste_to_tableau,
	tableau_to_foundation,
	foundation_to_tableau,
	tableau_to_tableau,
	draw,
	redeal
};

#endif //KLONDIKESOLVER_ACTION_TYPE_H