/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDOBJBIN_PLUGIN_H
#define UVDOBJBIN_PLUGIN_H

#include "plugin/plugin.h"
#include "uvd_types.h"
#include "uvdasm/config.h"

class UVDObjbinPlugin : public UVDPlugin
{
public:
	UVDObjbinPlugin();
	~UVDObjbinPlugin();
	virtual uv_err_t init(UVDConfig *config);

	virtual uv_err_t getName(std::string &out);
	virtual uv_err_t getDescription(std::string &out);	
	virtual uv_err_t getVersion(UVDVersion &out);
	virtual uv_err_t getAuthor(std::string &out);
	virtual uv_err_t getObject(UVDData *data, const std::string &object, const std::string &architecture, UVDObject **out);

public:
};

extern UVDObjbinPlugin *g_objbinPlugin;

#endif
