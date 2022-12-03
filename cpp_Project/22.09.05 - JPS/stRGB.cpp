#include "stdafx.h"
#include "stRGB.h"

stRGB::stRGB() {}

stRGB::stRGB(unsigned char R, unsigned char G, unsigned char B) : R(R), G(G), B(B) {}

stRGB::stRGB(int data) {
	B += (unsigned char)data;
	data = data >> 8;
	G += (unsigned char)data;
	data = data >> 8;
	R += (unsigned char)data;
}

void stRGB::Set(unsigned char R, unsigned char G, unsigned char B) {
	this->R = R;
	this->G = G;
	this->B = B;
}

void stRGB::Set(const stRGB& other){
	this->R = other.R;
	this->G = other.G;
	this->B = other.B;
}

void stRGB::operator=(const stRGB& other) {
	R = other.R;
	G = other.G;
	B = other.B;
}

void stRGB::operator+=(int data){
	B += (unsigned char)data;
	data = data >> 8;
	G += (unsigned char)data;
	data = data >> 8;
	R += (unsigned char)data;
}

