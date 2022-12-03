#pragma once
#include "Pos.h"
#include "enums.h"

struct Node {
	Pos pos;

	float g; // ����������� �̵��Ÿ� (���� �θ���� �Ÿ�)
	float h; // ���������� �Ÿ�
	float f; // g + h, f�� ���� ���� ��带 ã���� ����������.
	Node* parent = nullptr;

	eDIR dir;
public:
	void Set(Pos pos, float g, float h, Node* const parent, eDIR dir);
};