#pragma once
#include "Pos.h"
#include "enums.h"

struct Node {
	Pos pos;

	float g; // 출발점에서의 이동거리 (직전 부모와의 거리)
	float h; // 목적지와의 거리
	float f; // g + h, f가 가장 작은 노드를 찾으며 퍼저나간다.
	Node* parent = nullptr;

	eDIR dir;
public:
	void Set(Pos pos, float g, float h, Node* const parent, eDIR dir);
};