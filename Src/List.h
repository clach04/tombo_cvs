#ifndef LIST_H
#define LIST_H

////////////////////////////////
// ���X�g
////////////////////////////////

// �P�����̔ėp���X�g

// �ێ�����v�f�ɂ��Ă̓I�u�W�F�N�g�������ɃT�C�Y�w��ł���B
//
//
//  struct Val { DWORD a, b; }
//
//  List lst(sizeof(Val));
//  h = lst.Add();
//
//  Val *p = (Val*)lst.Value(h);
//  p->a = ...
//

class List {
	struct Item {
		Item *pNext;
		Item *pPrev;
		LPVOID pVal;
	};

	Item *pHead, *pTail;
public:
	
	List() : pHead(NULL), pTail(NULL){}
	~List() { Clear(); }

	void Clear();

	HANDLE Add(LPVOID val);

	// �w�肵���v�f���폜����
	LPVOID Remove(HANDLE h);

	HANDLE First();
	HANDLE Next(HANDLE h);
	HANDLE Prev(HANDLE h);

	LPVOID Value(HANDLE h);
};

HANDLE SearchList(List *l, const char *str);

#endif
