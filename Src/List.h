#ifndef LIST_H
#define LIST_H

////////////////////////////////
// リスト
////////////////////////////////

// 単方向の汎用リスト

// 保持する要素についてはオブジェクト生成時にサイズ指定できる。
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

	// 指定した要素を削除する
	LPVOID Remove(HANDLE h);

	HANDLE First();
	HANDLE Next(HANDLE h);
	HANDLE Prev(HANDLE h);

	LPVOID Value(HANDLE h);
};

HANDLE SearchList(List *l, const char *str);

#endif
