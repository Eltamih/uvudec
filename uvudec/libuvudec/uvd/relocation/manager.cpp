/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/relocation/relocation.h"

/*
UVDRelocationManager
*/

UVDRelocationManager::UVDRelocationManager()
{
}

UVDRelocationManager::~UVDRelocationManager()
{
}

/*
uv_err_t UVDRelocationManager::addRelocatableElement(UVDRelocatableElement *element)
{
	uv_assert_ret(element);
	m_relocatableElements.insert(element);
	return UV_ERR_OK;
}
*/

uv_err_t UVDRelocationManager::addRelocatableData(UVDRelocatableData *data)
{
	uv_assert_ret(data);
	m_data.push_back(data);
	return UV_ERR_OK;
}

uv_err_t UVDRelocationManager::applyPatch(UVDData **dataOut)
{
	return UV_DEBUG(applyPatchCore(dataOut, false));
}

uv_err_t UVDRelocationManager::applyPatchCore(UVDData **dataOut, bool useDefaultValue)
{
	std::vector<UVDData *> dataVector;

	for( std::vector<UVDRelocatableData *>::iterator iter = m_data.begin(); iter != m_data.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		UVDData *data = NULL;

		uv_assert_ret(relocatableData);
		//Update the encapsulated data to reflect the relocations
		if( UV_FAILED(relocatableData->applyRelocationsCore(useDefaultValue)) )
		{
			/*
			if( relocatableData->m_data )
			{
				printf_error("could not apply relocation %s\n", relocatableData->m_data->getSource().c_str());
			}
			*/
			return UV_DEBUG(UV_ERR_GENERAL);
		}
		
		//And rack up the raw data so it can be assembled
		uv_assert_err_ret(relocatableData->getRelocatableData(&data));
		if( data )
		{
			//printf("applyPatch hexdump\n");
			//data->hexdump();
			dataVector.push_back(data);
		}
	}
	//printf("\napplyPatchCore(): concat start\n");
	uv_assert_err_ret(UVDData::concatenate(dataVector, dataOut));
	//printf("\napplyPatchCore(): concat end\n\n");
	
	return UV_ERR_OK;
}

uv_err_t UVDRelocationManager::getOffset(const UVDRelocatableData *relocatableDataIn, uint32_t *offsetOut)
{
	uint32_t offset = 0;
	for( std::vector<UVDRelocatableData *>::iterator iter = m_data.begin(); iter != m_data.end(); ++iter )
	{
		UVDRelocatableData *relocatableData = *iter;
		UVDData *defaultData = NULL;

		uv_assert_ret(relocatableData);
		
		if( relocatableData == relocatableDataIn )
		{
			*offsetOut = offset;
			return UV_ERR_OK;
		}
		
		uv_assert_err_ret(relocatableData->getDefaultRelocatableData(&defaultData));
		if( defaultData )
		{
			uint32_t size = 0;
			
			uv_assert_err_ret(defaultData->size(&size));
			offset += size;
		}
	}
	
	//Bad input was supplied: we should have found it in the list
	return UV_DEBUG(UV_ERR_GENERAL);
}

