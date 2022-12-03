#include "stdafx.h"
#include "Node.h"
#include "WndProc.h"

void Node::Set(Pos pos, float g, float h, Node* const parent, eDIR dir) {
	this->pos = pos;
	this->g = g;
	this->h = h;
	f = g + h;
	this->parent = parent;
	this->dir = dir;
}