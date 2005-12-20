#include <windows.h>
#include <tchar.h>

#include "Tombo.h"
#include "TString.h"
#include "Uniconv.h"
#include "TomboURI.h"
#include "Repository.h"

#include "MemoNote.h"

#include "RepositoryImpl.h"

// At this time, Repository is only proxy to RepositoryImpl.
// This class will choose repository implementations in future.

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


URIList *Repository::GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt)
{
	return pDefaultImpl->GetChild(pFolder, bSkipEncrypt);
}

BOOL Repository::RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI)
{
	return pDefaultImpl->RequestAllocateURI(pBaseURI, pText, pHeadLine, pURI, pTemplateURI);
}

BOOL Repository::GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute)
{
	return pDefaultImpl->GetAttribute(pURI, pAttribute);
}

BOOL Repository::GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize)
{
	return pDefaultImpl->GetNoteAttribute(pURI, pLastUpdate, pCreateDate, pFileSize);
}

BOOL Repository::SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute)
{
	return pDefaultImpl->SetAttribute(pURI, pAttribute);
}

LPTSTR Repository::GetNoteData(const TomboURI *pURI)
{
	return pDefaultImpl->GetNoteData(pURI);
}

char* Repository::GetNoteDataA(const TomboURI *pURI)
{
	return pDefaultImpl->GetNoteDataNative(pURI);
}

char* Repository::GetNoteDataUTF8(const TomboURI *pURI)
{
#if defined(PLATFORM_WIN32)
	return GetNoteData(pURI);
#else
	LPTSTR pT = GetNoteData(pURI);
	char *pA = ConvUCS2ToUTF8(pT);
	WipeOutAndDelete(pT);
	return pA;
#endif
}

BOOL Repository::ExecuteAssoc(const TomboURI *pURI, ExeAppType nType)
{
	return pDefaultImpl->ExecuteAssoc(pURI, nType);
}

BOOL Repository::MakeFolder(const TomboURI *pParent, LPCTSTR pFolderName)
{
	return pDefaultImpl->MakeFolder(pParent, pFolderName);
}

BOOL Repository::GetFileName(const TomboURI *pURI, TString *pName)
{
	return pURI->GetBaseName(pName);
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

