#include <windows.h>
#include <tchar.h>

#include "TString.h"
#include "TomboURI.h"
#include "Repository.h"

#include "MemoNote.h"

#include "RepositoryImpl.h"

/////////////////////////////////////////
// Global definition
/////////////////////////////////////////

Repository g_Repository;

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

Repository::Repository() : pDefaultImpl(NULL)
{
}

Repository::~Repository()
{
}

BOOL Repository::Init(const RepositoryOption *pOpt)
{
	roOption = *pOpt;

	LocalFileRepository *pImpl;
	
	if ((pImpl = new LocalFileRepository()) == NULL || !pImpl->Init(&roOption)) return FALSE;
	pDefaultImpl = pImpl;
	return TRUE;
}

BOOL Repository::SetRepositoryOption(const RepositoryOption *pOpt)
{
	roOption = *pOpt;
	return pDefaultImpl->SetRepositoryOption(&roOption);
}

BOOL Repository::Create(const TomboURI *pTemplate, LPCTSTR pData, 
						TString *pRealHeadLine, TomboURI *pAllocedURI)
{
	return pDefaultImpl->Create(pTemplate, pData, pRealHeadLine, pAllocedURI);
}

BOOL Repository::Update(const TomboURI *pCurrentURI, 
						LPCTSTR pData, 
						TomboURI *pNewURI, TString *pNewHeadLine)
{
	return pDefaultImpl->Update(pCurrentURI, pData, pNewURI, pNewHeadLine);
}

BOOL Repository::GetHeadLine(const TomboURI *pURI, TString *pHeadLine)
{
	return pDefaultImpl->GetHeadLine(pURI, pHeadLine);
}

BOOL Repository::GetOption(const TomboURI *pURI, URIOption *pOption)
{
	return pDefaultImpl->GetOption(pURI, pOption);
}

BOOL Repository::SetOption(const TomboURI *pCurrentURI, URIOption *pOption)
{
	return pDefaultImpl->SetOption(pCurrentURI, pOption);
}

BOOL Repository::IsEncrypted(const TomboURI *pURI)
{
	return pDefaultImpl->IsEncrypted(pURI);
}

BOOL Repository::Delete(const TomboURI *pURI, URIOption *pOption)
{
	return pDefaultImpl->Delete(pURI, pOption);
}

BOOL Repository::Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption)
{
	return pDefaultImpl->Copy(pCopyFrom, pCopyTo, pOption);
}

BOOL Repository::Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption)
{
	return pDefaultImpl->Move(pMoveFrom, pMoveTo, pOption);
}

BOOL Repository::GetPhysicalPath(const TomboURI *pURI, TString *pFullPath)
{
	return pDefaultImpl->GetPhysicalPath(pURI, pFullPath);
}

BOOL Repository::ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption)
{
	return pDefaultImpl->ChangeHeadLine(pURI, pReqNewHeadLine, pOption);
}

BOOL Repository::GetList(const TomboURI *pFolder, DirList *pList, BOOL bSkipEncrypt)
{
	return pDefaultImpl->GetList(pFolder, pList, bSkipEncrypt);
}

BOOL Repository::RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI)
{
	return pDefaultImpl->RequestAllocateURI(pBaseURI, pText, pHeadLine, pURI, pTemplateURI);
}

BOOL Repository::GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute)
{
	return pDefaultImpl->GetAttribute(pURI, pAttribute);
}

BOOL Repository::SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute)
{
	return pDefaultImpl->SetAttribute(pURI, pAttribute);
}

LPTSTR Repository::GetNoteData(const TomboURI *pURI)
{
	return pDefaultImpl->GetNoteData(pURI);
}

/////////////////////////////////////////
// get attach URI
/////////////////////////////////////////

BOOL Repository::GetAttachURI(const TomboURI *pBase, TomboURI *pAttached)
{
	if (pBase->IsLeaf()) {
		if (!pBase->GetParent(pAttached)) return FALSE;
	} else {
		if (!pAttached->Init(pBase->GetFullURI())) return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////
// URIOption implimentation
/////////////////////////////////////////

URIOption::~URIOption()
{
	delete pNewURI;
	delete pNewHeadLine;
}