int __stdcall func(int a, int b, int c = 0, int d = 0) {
	return a + b;
}

int __cdecl func2(int a, int b, int c = 0, int d = 0) {
	return a + b;
}

int main() {
	int a = 1;
	int b = 2;

	func(a, b);
	func2(a, b);
}