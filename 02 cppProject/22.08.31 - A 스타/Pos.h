#pragma once
struct Pos {
public:
	Pos();
	Pos(short x, short y);

public:
	short x;
	short y;

public:
	bool operator==(const Pos& other) const;
	bool operator!=(const Pos& other) const;
};
