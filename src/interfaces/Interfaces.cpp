#include "../../include/interfaces/Interfaces.h"
#include <string.h>

InterfaceReg* s_pInterfaceRegs;

InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char* pName)
	: m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}

void* CreateInterface(const char* pName, InterfaceStatus* pStatus)
{
	for (InterfaceReg* pCur = s_pInterfaceRegs; pCur; pCur = pCur->m_pNext)
	{
		if (strcmp(pCur->m_pName, pName) == 0)
		{
			if (pStatus)
			{
				*pStatus = InterfaceStatus::OK;
			}

			return pCur->m_CreateFn();
		}
	}

	if (pStatus)
	{
		*pStatus = InterfaceStatus::FAILED;
	}

	return 0;
}
