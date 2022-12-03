#pragma once

struct stRGB {
	stRGB(unsigned char R, unsigned char G, unsigned char B);
	stRGB();
	stRGB(int data);

	unsigned char R = 0, G = 0, B = 0;

	void operator=(const stRGB& other);
	void operator+=(int data);
	void Set(unsigned char R, unsigned char G, unsigned char B);
	void Set(const stRGB& other);
};