#ifndef TOMBO_AUTOPTR_H
#define TOMBO_AUTOPTR_H

template <class T>
class AutoPointer {
	T* pPtr;
public:
	AutoPointer(T* p = NULL) : pPtr(p) {}
	~AutoPointer() { delete pPtr; }

	void set(T* p) { pPtr = p; }
	void releaseAndSet(T* p) { delete pPtr; pPtr = p; }
};

template <class T>
class ArrayAutoPointer {
	T* pPtr;
public:
	ArrayAutoPointer(T* p = NULL) : pPtr(p) {}
	~ArrayAutoPointer() { delete [] pPtr; }

	void set(T* p) { pPtr = p; }
	void releaseAndSet(T* p) { delete [] pPtr; pPtr = p; }
};

class SecureBufferAutoPointerT {
	LPTSTR pPtr;
public:
	SecureBufferAutoPointerT(LPTSTR p) : pPtr(p) {}
	~SecureBufferAutoPointerT() {
		if (pPtr == NULL) return;
		LPTSTR q = pPtr;
		while(*q) *q++ = TEXT('\0');
		delete [] pPtr;
	}
};

class SecureBufferAutoPointerA {
	char *pPtr;
public:
	SecureBufferAutoPointerA(char *p) : pPtr(p) {}
	~SecureBufferAutoPointerA() {
		if (pPtr == NULL) return;
		char *q = pPtr;
		while(*q) *q++ = TEXT('\0');
		delete [] pPtr;
	}
};
#endif
