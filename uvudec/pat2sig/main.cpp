/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details

obj2pat entry point
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include "uvd/config/arg_property.h"
#include "uvd/config/arg_util.h"
#include "uvd/util/error.h"
#include "uvd/core/init.h"
#include "uvd/util/util.h"
#include "uvd/core/uvd.h"
#include "uvdflirt/args.h"
#include "uvdflirt/args_property.h"
#include "uvdflirt/flirt.h"
#include "uvdflirt/config.h"
#include "uvd/data/data.h"
#include "uvd/language/format.h"
#include "uvd/assembly/address.h"
#include "uvd/language/language.h"

static uv_err_t versionPrintPrefixThunk();

static const char *GetVersion()
{
	return UVUDEC_VER_STRING;
}

static uv_err_t doConvert()
{
	uv_err_t rc = UV_ERR_GENERAL;
	std::string output;
	UVDFLIRTConfig *flirtConfig = NULL;
		
	flirtConfig = g_UVDFLIRTConfig;
	uv_assert_ret(flirtConfig);

	//Get string output
	printf_debug_level(UVD_DEBUG_SUMMARY, "main: creating pat file...\n");
	uv_assert_err_ret(g_flirt->patFiles2SigFile(flirtConfig->m_targetFiles, flirtConfig->m_outputFile));
	printf_debug_level(UVD_DEBUG_PASSES, "main: pat done\n");

	rc = UV_ERR_OK;
	
	return rc;
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments, void *user)
{
	UVDConfig *config = NULL;
	UVDFLIRTConfig *flirtConfig = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	
	config = g_config;
	uv_assert_ret(config);
	uv_assert_ret(config->m_argv);
	uv_assert_ret(argConfig);
	flirtConfig = g_UVDFLIRTConfig;
	uv_assert_ret(flirtConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
	}

	if( argConfig->isNakedHandler() )
	{
		/*
		Try to guess type based on file suffixes
		*/
		
		for( std::vector<std::string>::iterator iter = argumentArguments.begin();
				iter != argumentArguments.end(); ++iter )
		{
			const std::string &arg = *iter;
			
			if( arg.find(".pat") != std::string::npos )
			{
				flirtConfig->m_targetFiles.push_back(arg);
			}
			else if( arg.find(".sig") != std::string::npos )
			{
				flirtConfig->m_outputFile = arg;
			}
			//Hmm okay append suffixes and guess
			else
			{
				printf_error("cannot guess argument purpose: %s\n", arg.c_str());
				return UV_DEBUG(UV_ERR_GENERAL);
			}
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_VERSION )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigVersion = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_LIB_NAME )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_libName = firstArg;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_FEATURES )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigFeatures = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_PAD )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigPad = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_PROCESSOR_ID )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigProcessorID = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_OS_TYPES )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigOSTypes = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_APP_TYPES )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigAppTypes = firstArgNum;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_FLIRT_SIG_FILE_TYPES )
	{
		uv_assert_ret(!argumentArguments.empty());
		flirtConfig->m_sigFileTypes = firstArgNum;
	}
	else
	{
		//return UV_DEBUG(argParserDefault(argConfig, argumentArguments));
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

uv_err_t initProgConfig()
{
	UVDConfig *config = g_config;
	
	uv_assert_err_ret(UVDInitFLIRTSharedConfig(config));

	//Callbacks
	g_config->versionPrintPrefixThunk = versionPrintPrefixThunk;

	uv_assert_err_ret(config->registerDefaultArgument(argParser, " [.pat files, .sig file]"));	
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_PAT_FUNCTIONS_AS_MODULES, 0, "functions-as-modules", "functions will not be grouped into object modules", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_VERSION, 0, "sig-version", "signature format version", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_LIB_NAME, 0, "lib-name", "library name string", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_FEATURES, 0, "features", "feature flags", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_PAD, 0, "unknown-pad", "set at your own risk/intuition/boredom", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_PROCESSOR_ID, 0, "processor-ID", "processor-ID", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_OS_TYPES, 0, "OS-types", "OS-types", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_APP_TYPES, 0, "app-types", "app-types", 1, argParser, true));
	uv_assert_err_ret(config->registerArgument(UVD_PROP_FLIRT_SIG_FILE_TYPES, 0, "file-types", "file-types", 1, argParser, true));

	return UV_ERR_OK;	
}

static uv_err_t versionPrintPrefixThunk()
{
	const char *program_name = "uvobj2pat";
	
	/*
	if( g_config && g_config->m_argv )
	{
		program_name = g_config->m_argv[0];
	}
	*/

	printf_help("%s version %s\n", program_name, GetVersion());
	return UV_ERR_OK;
}

uv_err_t uvmain(int argc, char **argv)
{
	uv_err_t rc = UV_ERR_GENERAL;
	UVDConfig *config = NULL;
	UVDFLIRTConfig *flirtConfig = NULL;
	uv_err_t parseMainRc = UV_ERR_GENERAL;
	
	if( strcmp(GetVersion(), UVDGetVersion()) )
	{
		printf_warn("libuvudec version mismatch (exe: %s, libuvudec: %s)\n", GetVersion(), UVDGetVersion());
		fflush(stdout);
	}
	
	//Early library initialization.  Logging and arg parsing structures
	uv_assert_err_ret(UVDInit());
	config = g_config;
	uv_assert_ret(config);
	//Early local initialization
	uv_assert_err_ret(initProgConfig());
	
	//Grab our command line options
	parseMainRc = config->parseMain(argc, argv);
	uv_assert_err_ret(parseMainRc);
	if( parseMainRc == UV_ERR_DONE )
	{
		rc = UV_ERR_OK;
		goto error;
	}


	//Create a doConvertr engine active on that input
	printf_debug_level(UVD_DEBUG_SUMMARY, "doConvert: initializing FLIRT engine...\n");
	if( UV_FAILED(UVDFLIRT::getFLIRT(&g_flirt)) )
	{
		printf_error("Failed to initialize FLIRT engine\n");
		rc = UV_ERR_OK;
		goto error;
	}
	uv_assert_ret(g_flirt);

	flirtConfig = g_UVDFLIRTConfig;
	uv_assert_ret(flirtConfig);

	//Source .pat files
	if( flirtConfig->m_targetFiles.empty() )
	{
		printf_error("Target file(s) not specified\n");
		config->printHelp();
		uv_assert_err(UV_ERR_GENERAL);
	}
	
	if( UV_FAILED(doConvert()) )
	{
		printf_error("Top level doConvert failed\n");
		uv_assert(UV_ERR_GENERAL);
	}	

	rc = UV_ERR_OK;

error:
	uv_assert_err_ret(UVDDeinit());
		
	return UV_DEBUG(rc);
}

int main(int argc, char **argv)
{
	printf_debug("main: enter\n");

	//Simple translation to keep most stuff in the framework
	uv_err_t rc = uvmain(argc, argv);
	printf_debug("main: exit\n");
	if( UV_FAILED(rc) )
	{
		printf_error("failed\n");
		return 1;
	}
	else
	{
		return 0;
	}
}
