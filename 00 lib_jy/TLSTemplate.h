#pragma once
#define MAX_TLS 100

template<typename T>
class TLSTemplate {
protected:
	TLSTemplate();
	virtual ~TLSTemplate();

private:
	short index = -1;
	T* instanceArray[MAX_TLS];

protected:
	const DWORD tlsIndex;

protected:
	T* Get();
};

template<typename T>
TLSTemplate<T>::TLSTemplate() : tlsIndex(TlsAlloc()) {
}

template<typename T>
TLSTemplate<T>::~TLSTemplate() {
	for (int i = 0; i <= index; i++) {
		delete instanceArray[i];
	}
}

template<typename T>
T* TLSTemplate<T>::Get() {
	T* p = (T*)TlsGetValue(tlsIndex);
	if (nullptr == p) {
		p = new T();
		TlsSetValue(tlsIndex, (LPVOID)p);
		instanceArray[++index] = p;
	}

	return p;
}