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

#endif
