// (C) Copyright 2002-2012 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

//-----------------------------------------------------------------------------
//----- acrxEntryPoint.cpp
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"
#include "Utilities.h"
#include "EmployeeDetails\ADSKEmployeeDetails.h"

//-----------------------------------------------------------------------------
#define szRDS _RXST("ADSK")

//-----------------------------------------------------------------------------
//----- ObjectARX EntryPoint
class CStep05App : public AcRxArxApp {

public:
	CStep05App () : AcRxArxApp () {}

	virtual AcRx::AppRetCode On_kInitAppMsg (void *pkt) {
		// TODO: Load dependencies here
		AcRxObject* pSvc;
		if (!(pSvc = acrxServiceDictionary->at(ADSKEMPLOYEEDETAILS_DBXSERVICE)))
		{
			// Try to load the module, if it is not yet present 
			if (!acrxDynamicLinker->loadModule(_T("ASDKEmployeeDetails.dbx"), 0))
			{

				acutPrintf(_T("Unable to load AsdkEmployeeDetails.dbx. Unloading this application...\n"));
				return (AcRx::kRetError);

			}

		}
		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kInitAppMsg (pkt) ;
		
		// TODO: Add your initialization code here

		return (retCode) ;
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg (void *pkt) {
		// TODO: Add your code here

		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kUnloadAppMsg (pkt) ;

		// TODO: Unload dependencies here

		return (retCode) ;
	}

	virtual void RegisterServerComponents () {
	}

	static void ADSKStep05CREATE() {
		try {
			AcDbObjectId layerId;
			if (createLayer(L"USER", layerId) != Acad::eOk) {
				acutPrintf(L"\nCannot create new layer record");
				return;
			}

			applyCurDwgLayerTableChanges();
			acutPrintf(L"\nNew Layer created successfully");

			if (createBlockRecord(L"EMPLOYEE") != Acad::eOk) {
				acutPrintf(L"\nCannot create block record");
			}
			else
			{
				acutPrintf(L"\nBlock record EMPLOYEE created successfully");
			}
		}
		catch (...)
		{
		}
	}

	static void ADSKStep05SETLAYER() {
		try {
			Acad::ErrorStatus errorStatus;
			AcDbBlockTable* pBlockTable;
			AcDbBlockTableRecord* pModelSpace;

			if ((errorStatus = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable, AcDb::kForRead)) != Acad::eOk) {
				acutPrintf(L"\nCannot open block table");
				return;
			}

			if ((errorStatus = pBlockTable->getAt(ACDB_MODEL_SPACE, pModelSpace, AcDb::kForWrite)) != Acad::eOk) {
				acutPrintf(L"\nCannot get Model Space. Check the drawing");
				return;
			}

			pBlockTable->close();

			AcDbBlockTableRecordIterator* pBTRIter;
			if ((errorStatus = pModelSpace->newIterator(pBTRIter)) != Acad::eOk) {
				acutPrintf(L"\nCannot create Model Space iterator");
				pModelSpace->close();
				return;
			}

			ACHAR* blockName;
			AcDbEntity* pEntity;
			AcDbBlockTableRecord* pCurrEntityBlock;
			AcDbObjectId blockId;

			for (pBTRIter->start(); !pBTRIter->done(); pBTRIter->step()) {
				if ((errorStatus = pBTRIter->getEntity(pEntity, AcDb::kForRead)) != Acad::eOk) {
					acutPrintf(L"\nCannot open Entity");
					continue;
				}

				if (pEntity->isA() != AcDbBlockReference::desc()) {
					pEntity->close();
					continue;
				}

				blockId = (AcDbBlockReference::cast(pEntity))->blockTableRecord();

				if (acdbOpenObject((AcDbObject*&)pCurrEntityBlock, blockId, AcDb::kForRead) == Acad::eOk) {
					pCurrEntityBlock->getName(blockName);
					if (wcscmp(blockName, L"EMPLOYEE") == 0) {
						if (pEntity->upgradeOpen() == Acad::eOk) {
							pEntity->setLayer(L"USER");
						}
					}
					pCurrEntityBlock->close();
					acdbFree(blockName);
				}
				pEntity->close();
			}
			pModelSpace->close();
		}
		catch (...) {
		}
	}

	static void ADSKStep05ADDDETAIL() {
		ads_name eName;
		ads_point ePoint;
		try
		{

			if (acedEntSel(L"\nSelect employee:", eName, ePoint) != RTNORM) {
				return;
			}

			AcDbObjectId idO;
			if (acdbGetObjectId(idO, eName) != Acad::eOk) {
				return;
			}

			AcDbObject* pO;
			if (acdbOpenAcDbObject(pO, idO, AcDb::kForWrite) != Acad::eOk) {
				return;
			}

			if (!pO->isKindOf(AcDbBlockReference::desc())) {
				acutPrintf(L"\nThis is not a block reference");
				pO->close();
				return;
			}

			int id, cubeNumder;
			TCHAR firstName[132];
			TCHAR lastName[132];
			if (acedGetInt(L"\nEnter employee ID:", &id) != RTNORM || acedGetInt(L"\nEnter cube number:", &cubeNumder) != RTNORM || acedGetString(0, L"\nEnter employee first name:", firstName) != RTNORM || acedGetString(0, L"\nEnter employee last name:", lastName) != RTNORM) {
				pO->close();
				return;
			}

			if ((idO = pO->extensionDictionary()) == AcDbObjectId::kNull) {
				if (pO->createExtensionDictionary() != Acad::eOk) {
					acutPrintf(L"\nCannot create ext dict");
					pO->close();
					return;
				}
				idO = pO->extensionDictionary();
			}
			pO->close();

			AcDbDictionary* pExtDict;
			if (acdbOpenAcDbObject((AcDbObject*&)pExtDict, idO, AcDb::kForWrite, Adesk::kTrue) != Acad::eOk) {
				acutPrintf(L"\nCannot open the ext dict");
				return;
			}

			if (pExtDict->isErased()) {
				pExtDict->erase(Adesk::kFalse);
			}

			AcDbDictionary* pEmployeeDict;
			if (pExtDict->getAt(L"ADSK_EMPLOYEE_DICTIONARY", idO) == Acad::eKeyNotFound) {
				pEmployeeDict = new AcDbDictionary;
				if (Acad::ErrorStatus es; (es = pExtDict->setAt(L"ADSK_EMPLOYEE_DICTIONARY", pEmployeeDict, idO)) != Acad::eOk) {
					acutPrintf(L"\nCannot create Employee dict");
					pExtDict->close();
					return;
				}
			}
			else
			{
				if (acdbOpenAcDbObject(pO, idO, AcDb::kForWrite) != Acad::eOk) {
					acutPrintf(L"\nCannot open the Employee dict");
					pExtDict->close();
					return;
				}

				if ((pEmployeeDict = AcDbDictionary::cast(pO)) == NULL) {
					acutPrintf(L"\nThis entry is not a dict");
					pO->close();
					pExtDict->close();
					return;
				}
			}
			pExtDict->close();

			if (pEmployeeDict->getAt(L"DETAILS", idO) == Acad::eOk) {
				acutPrintf(L"\nDetails already assign to the Emlployee object");
				pEmployeeDict->close();
				return;
			}

			ADSKEmployeeDetails* pEmployeeDetails = new ADSKEmployeeDetails;
			pEmployeeDetails->setID(id);
			pEmployeeDetails->setCube(cubeNumder);
			pEmployeeDetails->setFirstName(firstName);
			pEmployeeDetails->setLastName(lastName);

			if ((pEmployeeDict->setAt(L"DETAILS", pEmployeeDetails, idO)) != Acad::eOk) {
				acutPrintf(L"\nCannot add the details to that object");
				delete pEmployeeDetails;
				pEmployeeDict->close();
				return;
			}
			acutPrintf(L"\nDetails added successfully.");
			pEmployeeDict->close();
			pEmployeeDetails->close();
		}
		catch (...)
		{
		}
	}

	static void ADSKStep05LISTDETAILS() {
		ads_name eName;
		ads_point ePoint;
		try
		{
			if (acedEntSel(L"\nSelect employee:", eName, ePoint) != RTNORM) {
				return;
			}

			AcDbObjectId idO;
			if (acdbGetObjectId(idO, eName) != Acad::eOk) {
				return;
			}

			AcDbObject* pO;
			if (acdbOpenAcDbObject(pO, idO, AcDb::kForRead) != Acad::eOk) {
				return;
			}

			if (!pO->isKindOf(AcDbBlockReference::desc())) {
				acutPrintf(L"\nThis is not a block ref");
				pO->close();
				return;
			}

			if ((idO = pO->extensionDictionary()) == AcDbObjectId::kNull) {
				pO->close();
				return;
			}
			pO->close();

			AcDbDictionary* pExtDict;
			if ((acdbOpenAcDbObject((AcDbObject*&)pExtDict, idO, AcDb::kForRead)) != Acad::eOk) {
				acutPrintf(L"\nCannot open ext ict");
				return;
			}

			AcDbDictionary* pEmployeeDict;
			if (pExtDict->getAt(L"ADSK_EMPLOYEE_DICTIONARY", idO) == Acad::eKeyNotFound) {
				pExtDict->close();
				return;
			}
			else
			{
				if ((acdbOpenAcDbObject(pO, idO, AcDb::kForRead)) != Acad::eOk) {
					acutPrintf(L"\nCannot open the Employee dict");
					pExtDict->close();
					return;
				}

				if ((pEmployeeDict = AcDbDictionary::cast(pO)) == NULL) {
					acutPrintf(L"\nThe entry is not dictionary");
					pO->close();
					pExtDict->close();
					return;
				}
			}

			if (pEmployeeDict->getAt(L"DETAILS", idO) != Acad::eOk) {
				pEmployeeDict->close();
				pExtDict->close();
				return;
			}

			if (acdbOpenAcDbObject(pO, idO, AcDb::kForRead) != Acad::eOk) {
				acutPrintf(L"\nCannot open Details dict");
				pEmployeeDict->close();
				pExtDict->close();
				return;
			}

			ADSKEmployeeDetails* pEmployeeDetails = ADSKEmployeeDetails::cast(pO);
			if (pEmployeeDetails == NULL) {
				acutPrintf(L"\nNo details found");
				pO->close();
				pEmployeeDict->close();
				pExtDict->close();
				return;
			}

			Adesk::Int32 index = 0;
			acutPrintf(L"Employee's cube number:");

			TCHAR* str = nullptr;

			pEmployeeDetails->iD(index);
			acutPrintf(L"Employee's ID: %d\n", index);

			pEmployeeDetails->firstName(str);
			acutPrintf(L"Emloyee's first name: %s\n", str);
			delete[] str;

			pEmployeeDetails->iD(index);
			acutPrintf(L"Employee's ID: %d\n", index);
			pEmployeeDetails->cube(index);
			acutPrintf(L"Employee's cube number: %d\n", index);
			pEmployeeDetails->lastName(str);
			acutPrintf(L"Emloyee's last name: %s\n", str);
			delete[] str;

			pO->close();
			pExtDict->close();
			pEmployeeDict->close();
		}
		catch (...)
		{
		}
	}

	static void ADSKStep05REMOVEDETAIL() {
		ads_name eName;
		ads_point ePoint;
		try
		{

			if (acedEntSel(L"Select employee:", eName, ePoint) != RTNORM) {
				return;
			}

			AcDbObjectId idO;
			if (acdbGetObjectId(idO, eName) != Acad::eOk) {
				return;
			}

			AcDbObject* pO;
			if (acdbOpenAcDbObject(pO, idO, AcDb::kForRead) != Acad::eOk) {
				return;
			}

			if (!pO->isKindOf(AcDbBlockReference::desc())) {
				acutPrintf(L"\nThis is not block ref");
				pO->close();
				return;
			}

			if ((idO = pO->extensionDictionary()) == AcDbObjectId::kNull) {
				pO->close();
				return;
			}
			pO->close();

			AcDbDictionary* pExtDict;
			if (acdbOpenAcDbObject((AcDbObject*&)pExtDict, idO, AcDb::kForWrite, Adesk::kFalse) != Acad::eOk) {
				acutPrintf(L"\nCannot open ext dict");
				return;
			}

			AcDbDictionary* pEmployeeDict;
			if (pExtDict->getAt(L"ADSK_EMPLOYEE_DICTIONARY", idO) == Acad::eKeyNotFound) {
				pExtDict->close();
				return;
			}
			else
			{
				if (acdbOpenAcDbObject(pO, idO, AcDb::kForWrite) != Acad::eOk) {
					acutPrintf(L"\nCannot open Employee dict");
					pExtDict->close();
					return;
				}

				if ((pEmployeeDict = AcDbDictionary::cast(pO)) == NULL) {
					acutPrintf(L"\nEntry is not dict");
					pO->close();
					pExtDict->close();
					return;
				}
			}

			if (pEmployeeDict->getAt(L"DETAILS", idO) != Acad::eOk) {
				acutPrintf(L"\nNo details assigned with this Employee dict");
				pExtDict->close();
				pEmployeeDict->close();
				return;
			}

			if (acdbOpenAcDbObject(pO, idO, AcDb::kForWrite) != Acad::eOk) {
				acutPrintf(L"\nCannot open the details of object");
				pEmployeeDict->close();
				pExtDict->close();
				return;
			}

			pO->erase();
			pO->close();
			if (pEmployeeDict->numEntries() == 0) {
				pEmployeeDict->close();
			}
			pEmployeeDict->close();

			if (pExtDict->numEntries() == 0) {
				pExtDict->close();
			}
			pExtDict->close();

		}
		catch (...)
		{
		}
	}
} ;

//-----------------------------------------------------------------------------
IMPLEMENT_ARX_ENTRYPOINT(CStep05App)

ACED_ARXCOMMAND_ENTRY_AUTO(CStep05App, ADSKStep05, CREATE, CREATE, ACRX_CMD_TRANSPARENT, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CStep05App, ADSKStep05, SETLAYER, SETLAYER, ACRX_CMD_TRANSPARENT, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CStep05App, ADSKStep05, ADDDETAIL, ADDDETAIL, ACRX_CMD_TRANSPARENT, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CStep05App, ADSKStep05, LISTDETAILS, LISTDETAILS, ACRX_CMD_TRANSPARENT, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CStep05App, ADSKStep05, REMOVEDETAIL, REMOVEDETAIL, ACRX_CMD_TRANSPARENT, NULL)