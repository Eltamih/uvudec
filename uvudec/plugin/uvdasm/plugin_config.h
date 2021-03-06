/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVDASM_PLUGIN_CONFIG_H
#define UVDASM_PLUGIN_CONFIG_H

#include "uvd/config/config.h"

//Architecture
#define UVD_PROP_ARCH_FILE						"arch.file"
#define UVD_PROP_ARCH_PATHS						"arch.paths"

class UVDAsmPlugin;
class UVDAsmConfig
{
public:
	UVDAsmConfig();
	~UVDAsmConfig();
	
	uv_err_t init(UVDConfig *config);
	uv_err_t deinit();

	uv_err_t setConfigInterpreterLanguageInterface(const std::string &in);
	uv_err_t setConfigInterpreterLanguage(const std::string &in);
	uv_err_t getDefaultArchitectureFile(std::string &ret);

public:
	//UVDAsmPlugin *m_plugin;
	//Default interpreter to use for script files
	uint32_t m_configInterpreterLanguage;
	uint32_t m_configInterpreterLanguageInterface;

	//std::string g_mcu_name;
	std::string m_mcu_name;
	//std::string g_mcu_desc;
	std::string m_mcu_desc;
	//std::string g_asm_imm_prefix;
	std::string m_asm_imm_prefix;
	//std::string g_asm_imm_prefix_hex;
	std::string m_asm_imm_prefix_hex;
	//std::string g_asm_imm_postfix_hex;
	std::string m_asm_imm_postfix_hex;
	//std::string g_asm_imm_suffix;
	std::string m_asm_imm_suffix;
	//The file that will be used to load the CPU module and such
	std::string m_architectureFileName;
};

extern UVDAsmConfig *g_asmConfig;

#endif

