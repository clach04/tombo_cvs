#ifndef MEMONOTE_H
#define MEMONOTE_H

class PasswordManager;
class TString;

#include "TreeViewItem.h"

////////////////////////////////////////
// Note type definition
////////////////////////////////////////

#define NOTE_TYPE_NO      0
#define NOTE_TYPE_PLAIN   1
#define NOTE_TYPE_CRYPTED 2

////////////////////////////////////////
// メモを抽象するクラス
////////////////////////////////////////

class MemoNote {
protected:
	LPTSTR pPath;
public:

	///////////////////////////////////////////
	// 初期化関連

	MemoNote();
	virtual ~MemoNote();

	BOOL Init(LPCTSTR p);

	///////////////////////////////////////////

	virtual MemoNote *GetNewInstance() = 0;
	virtual LPCTSTR GetExtension() = 0;
	virtual DWORD GetMemoIcon() = 0;

	MemoNote *Clone();
	BOOL Equal(MemoNote *pTarget);

	//////////////////////////////////
	// メモ内容操作関連

	// 新規メモの生成
	BOOL InitNewMemo(LPCTSTR pMemoPath, LPCTSTR pText, TString *pHeadLine);

	// メモ内容の取得
	virtual LPTSTR GetMemoBody(PasswordManager *pMgr);
	virtual char *GetMemoBodyA(PasswordManager *pMgr);

	// メモ内容の保存
	BOOL Save(PasswordManager *pMgr, LPCTSTR pMemo, TString *pHeadLine);
	virtual BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	BOOL IsReadOnly(BOOL *pReadOnly);

	//////////////////////////////////
	// 暗号化関連

	virtual BOOL IsEncrypted() { return FALSE; }

	// 暗号化: 暗号化後のMemoNoteインスタンスを返す。
	// pHeadLineにはバッファを設定して呼び出す。
	// ヘッドライン文字列に変更があった場合には　*pIsModifiedがTRUEとなり、
	// 新しいヘッドライン文字列が設定される。
	virtual MemoNote *Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);

	// 復号化: 復号化後のMemoNoteインスタンスを返す。	
	// pHeadLineにはバッファを設定して呼び出す。
	// ヘッドライン文字列に変更があった場合には　*pIsModifiedがTRUEとなり、
	// 新しいヘッドライン文字列が設定される。
	virtual MemoNote *Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);

	//////////////////////////////////
	// ファイル操作

	// メモのデータをを削除する
	virtual BOOL DeleteMemoData();

	// ファイル名の変更
	BOOL Rename(LPCTSTR pNewName);

	static MemoNote *CopyMemo(MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine);

	//////////////////////////////////
	// ビューとの関連情報

	LPCTSTR MemoPath() { return pPath; }


	//////////////////////////////////
	// Utility系


	// GetMemoBody()によって取得したバッファの開放
	// 開放時にバッファの内容をゼロクリアする。
	// セキュリティ上からGetMemoBody()で確保した領域はdeleteではなくこの関数で
	// 開放すること
	static void WipeOutAndDelete(LPTSTR pMemo);
#ifdef _WIN32_WCE
	static void WipeOutAndDelete(char *pMemo);
#endif

	static DWORD IsNote(LPCTSTR pFile);
	static BOOL MemoNoteFactory(LPCTSTR pPrefix, LPCTSTR pFile, MemoNote **ppNote);
};

////////////////////////////////////////
// 暗号化されていない通常のメモ
////////////////////////////////////////

class PlainMemoNote : public MemoNote {
public:
	MemoNote *GetNewInstance();
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(PasswordManager *pMgr);
	char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	MemoNote *Encrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
};

////////////////////////////////////////
// 暗号化されたメモ
////////////////////////////////////////

class CryptedMemoNote : public MemoNote {
protected:
	LPBYTE GetMemoBodySub(PasswordManager *pMgr, LPDWORD pSize);
public:
	MemoNote *GetNewInstance();
	LPCTSTR GetExtension();
	DWORD GetMemoIcon();

	LPTSTR GetMemoBody(PasswordManager *pMgr);
	char *GetMemoBodyA(PasswordManager *pMgr);

	BOOL SaveData(PasswordManager *pMgr, const char *pMemo, LPCTSTR pWriteFile);

	BOOL IsEncrypted() { return TRUE; }

	MemoNote *Decrypt(PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified);
};

#endif