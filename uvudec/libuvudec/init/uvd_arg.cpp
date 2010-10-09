/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd_arg.h"
#include "uvd_arg_property.h"
#include "uvd_arg_util.h"
#include "uvd_ascii_art.h"
#include "uvd_config.h"
#include "uvd_language.h"
#include "uvd_types.h"
#include "uvd_util.h"
#include "uvd_version.h"
#include "core/uvd_analysis.h"
#include "plugin/engine.h"
#include <vector>

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments);
static void UVDPrintVersion(void);

UVDArgConfig::UVDArgConfig()
{
	m_hasDefault = false;
}

UVDArgConfig::UVDArgConfig(const std::string &propertyForm,
		char shortForm, std::string longForm, 
		std::string helpMessage,
		uint32_t numberExpectedValues,
		UVDArgConfigHandler handler,
		bool hasDefault)
{
	m_propertyForm = propertyForm;
	m_shortForm = shortForm;
	m_longForm = longForm;
	m_helpMessage = helpMessage;
	m_numberExpectedValues = numberExpectedValues;
	m_handler = handler;
	m_hasDefault = hasDefault;
}
		
UVDArgConfig::UVDArgConfig(const std::string &propertyForm,
		char shortForm, std::string longForm, 
		std::string helpMessage,
		std::string helpMessageExtra,
		uint32_t numberExpectedValues,
		UVDArgConfigHandler handler,
		bool hasDefault)
{
	m_propertyForm = propertyForm;
	m_shortForm = shortForm;
	m_longForm = longForm;
	m_helpMessage = helpMessage;
	m_helpMessageExtra = helpMessageExtra;
	m_numberExpectedValues = numberExpectedValues;
	m_handler = handler;
	m_hasDefault = hasDefault;
}

UVDArgConfig::~UVDArgConfig()
{
}

bool UVDArgConfig::isNakedHandler() const
{
	return m_propertyForm == "";
}

/*
bool UVDArgConfig::operator==(const std::string &r) const
{
	return m_propertyForm == r;
}
*/

uv_err_t setupInstallDir()
{
	std::string programName;

	uv_assert_err_ret(getProgramName(programName));
	uv_assert_ret(g_config);
	//Like /opt/uvudec/3.0.0/bin/uvudec, need to remove two dirs
	g_config->m_installDir = uv_dirname(uv_dirname(programName));
	g_config->m_archDir = g_config->m_installDir + "/arch";

	return UV_ERR_OK;
}

uv_err_t UVDArgConfig::process(const std::vector<UVDArgConfig *> &argConfigs, std::vector<std::string> &args)
{	
	uv_assert_err_ret(setupInstallDir());

	/*
	Core high level argument dispatching
	We could store args as property map, but we'd still have to iterate over to match each part
	*/
	//Iterate for each actual command line argument
	//skip first arg which is prog name
	for( std::vector<std::string>::size_type argsIndex = 1; argsIndex < args.size(); ++argsIndex )
	{
		std::string arg = args[argsIndex];
		std::vector<UVDParsedArg> parsedArgs;

		uv_assert_err_ret(processArg(arg, parsedArgs));

		//Now iterate for each logical command line argument (such as from -abc)
		for( std::vector<UVDParsedArg>::iterator iter = parsedArgs.begin(); iter != parsedArgs.end(); ++iter )
		{
			UVDParsedArg &parsedArg = *iter;
			const UVDArgConfig *matchedConfig = NULL;
			
			//Extract the argument info for who should handle it
			uv_assert_err_ret(matchArgConfig(argConfigs, parsedArg, &matchedConfig));
			uv_assert_ret(matchedConfig);
			std::vector<std::string> argumentArguments;
			//Do we need to consume an arg for this?
			if( matchedConfig->m_numberExpectedValues > 0 )
			{
				uv_assert_ret(matchedConfig->m_numberExpectedValues == 1);
				if( parsedArg.m_embeddedValPresent )
				{
					argumentArguments.push_back(parsedArg.m_embeddedVal);
				}
				else
				{
					//Only grab the next argument if we don't have a default value
					if( !matchedConfig->m_hasDefault )
					{
						//we need to grab next then
						++argsIndex;
						uv_assert_ret(argsIndex < args.size());
						argumentArguments.push_back(args[argsIndex]);
					}
				}
			}
			//And call their handler
			uv_err_t handlerRc = matchedConfig->m_handler(matchedConfig, argumentArguments);
			uv_assert_err_ret(handlerRc);
			//Some option like help() has been called that means we should abort program
			if( handlerRc == UV_ERR_DONE )
			{
				return UV_ERR_DONE;
			}
		}
	}
	
	return UV_ERR_OK;
}

uv_err_t UVDInitArgConfig()
{
	//Now add our arguments
	
	//Actions
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_ACTION_HELP, 'h', "help", "print this message and exit", 0, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_ACTION_VERSION, 0, "version", "print version and exit", 0, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_ACTION_USELESS_ASCII_ART, 0, "print-useless-ascii-art", "print nifty ASCII art", 1, argParser, true));
	
	//Debug
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_LEVEL, 0, "verbose", "debug verbosity level", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_ARGS, 0, "verbose-args", "selectivly debug argument parsing", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_INIT, 0, "verbose-init", "selectivly debug initialization", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_PROCESSING, 0, "verbose-analysis", "selectivly debugging code analysis", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_ANALYSIS, 0, "verbose-processing", "selectivly debugging code post-analysis", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_PRINTING, 0, "verbose-printing", "selectivly debugging print routine", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_DEBUG_FILE, 0, "debug-file", "debug output (default: stdout)", 1, argParser, true));
	
	
	//Analysis target related
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_TARGET_ADDRESS_INCLUDE, 0, "addr-include", "inclusion address range (, or - separated)", 1, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_TARGET_ADDRESS_EXCLUDE, 0, "addr-exclude", "exclusion address range (, or - separated)", 1, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_TARGET_ADDRESS, 0, "analysis-address", "only output analysis data for specified address", 1, argParser, false));

	//Analysis
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_ANALYSIS_ONLY, 0, "analysis-only", "only do analysis, don't print data", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_ANALYSIS_DIR, 0, "analysis-dir", "create data suitible for stored analysis", 1, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_ANALYSIS_FLOW_TECHNIQUE, 0, "flow-analysis",
			"how to determine next instruction to analyze",
				"\tlinear (linear sweep): start at beginning, read all instructions linearly, then find jump/calls (default)\n"
				"\ttrace (recursive descent): start at all vectors, analyze all segments called/branched recursivly\n"
				,	
			1, argParser, false));

	//Output
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_OPCODE_USAGE, 0, "opcode-usage", "opcode usage count table", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_JUMPED_ADDRESSES, 0, "print-jumped-addresses", "whether to print information about jumped to addresses (*1)", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_CALLED_ADDRESSES, 0, "print-called-addresses", "whether to print information about called to addresses (*1)", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_USELESS_ASCII_ART, 0, "useless-ascii-art", "append nifty ascii art headers to output files", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_ADDRESS_COMMENT, 0, "addr-comment", "put comments on addresses", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_OUTPUT_ADDRESS_LABEL, 0, "addr-label", "label addresses for jumping", 1, argParser, true));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_PLUGIN_NAME, 0, "plugin", "load given library name as plugin", 1, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_PLUGIN_APPEND_PATH, 0, "plugin-path", "append dir to plugin search path", 1, argParser, false));
	g_config->m_configArgs.push_back(new UVDArgConfig(UVD_PROP_PLUGIN_PREPEND_PATH, 0, "plugin-path-prepend", "prepend dir to plugin search path", 1, argParser, false));

	return UV_ERR_OK;	
}

static uv_err_t argParser(const UVDArgConfig *argConfig, std::vector<std::string> argumentArguments)
{
	UVDConfig *config = NULL;
	//If present
	std::string firstArg;
	uint32_t firstArgNum = 0;
	
	config = g_config;
	uv_assert_ret(config);
	uv_assert_ret(config->m_argv);

	uv_assert_ret(argConfig);

	if( !argumentArguments.empty() )
	{
		firstArg = argumentArguments[0];
		firstArgNum = strtol(firstArg.c_str(), NULL, 0);
	}

	/*
	Actions
	*/
	if( argConfig->m_propertyForm == UVD_PROP_ACTION_HELP )
	{
		UVDHelp();
		return UV_ERR_DONE;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ACTION_VERSION )
	{
		UVDPrintVersion();
		return UV_ERR_DONE;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ACTION_USELESS_ASCII_ART )
	{
		printf("Have too much time on our hands do we?\n%s\n\n", getRandomUVNetASCIIArt().c_str());
	}
	/*
	Debug
	*/
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_LEVEL )
	{
		//If they didn't set any flags, assume its a general state across the program
		if( !config->anyVerboseActive() )
		{
			config->setVerboseAll();
		}
	
		//Did we specify or want default?
		if( argumentArguments.empty() )
		{
			config->m_verbose_level = UVD_DEBUG_VERBOSE;
		}
		else
		{
			config->m_verbose_level = firstArgNum;
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_ARGS )
	{
		if( argumentArguments.empty() )
		{
			config->m_verbose_args = true;
		}
		else
		{
			config->m_verbose_args = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_INIT )
	{
		if( argumentArguments.empty() )
		{
			config->m_verbose_init = true;
		}
		else
		{
			config->m_verbose_init = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_PROCESSING )
	{
		if( argumentArguments.empty() )
		{
			config->m_verbose_processing = true;
		}
		else
		{
			config->m_verbose_processing = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_ANALYSIS )
	{
		if( argumentArguments.empty() )
		{
			config->m_verbose_analysis = true;
		}
		else
		{
			config->m_verbose_analysis = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_PRINTING )
	{
		if( argumentArguments.empty() )
		{
			config->m_verbose_printing = true;
		}
		else
		{
			config->m_verbose_printing = UVDArgToBool(firstArg);
		}
	}
	/*
	This looks unused, was used for printing out the binary as we go along during disassembly
	else if( argConfig->m_propertyForm == "debug.print_binary" )
	{
		g_binary = TRUE;
	}
	*/
	else if( argConfig->m_propertyForm == UVD_PROP_DEBUG_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_sDebugFile = firstArg;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_TARGET_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_targetFileName = firstArg;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ARCH_FILE )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_architectureFileName = firstArg;
	}
	/*
	Analysis target specific
	As in, could be invalid depending on what our actual data was
	*/
	/*
	we need to parse two args at once, otherwise this is messy
	think was doing this before as comma seperate list?
	*/
	//Positive strategy: specify the address we want
	else if( argConfig->m_propertyForm == UVD_PROP_TARGET_ADDRESS_INCLUDE )
	{
		uint32_t low = 0;
		uint32_t high = 0;
		
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(parseNumericRangeString(firstArg, &low, &high));
		uv_assert_err_ret(config->addAddressInclusion(low, high));
	}
	//Negative strategy: specify the addresses we don't want
	else if( argConfig->m_propertyForm == UVD_PROP_TARGET_ADDRESS_EXCLUDE )
	{
		uint32_t low = 0;
		uint32_t high = 0;
		
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(parseNumericRangeString(firstArg, &low, &high));
		uv_assert_err_ret(config->addAddressExclusion(low, high));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_TARGET_ADDRESS )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_analysisOutputAddresses.insert(firstArgNum);
	}
	/*
	General analysis
	*/
	else if( argConfig->m_propertyForm == UVD_PROP_ANALYSIS_DIR )
	{
		uv_assert_ret(!argumentArguments.empty());
		config->m_analysisDir = firstArg;
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ANALYSIS_ONLY )
	{
		if( argumentArguments.empty() )
		{
			config->m_analysisOnly = true;
		}
		else
		{
			config->m_analysisOnly = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_ANALYSIS_FLOW_TECHNIQUE )
	{
		std::string arg = firstArg;
		if( arg == "linear" )
		{
			config->m_flowAnalysisTechnique = UVD__FLOW_ANALYSIS__LINEAR;
		}
		else if( arg == "trace" )
		{
			config->m_flowAnalysisTechnique = UVD__FLOW_ANALYSIS__TRACE;
		}
		else
		{
			printf_error("unknown flow analysis type: %s\n", arg.c_str());
			UVDHelp();
			return UV_DEBUG(UV_ERR_GENERAL);
		}
	}
	/*
	Output
	*/
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_OPCODE_USAGE )
	{
		if( argumentArguments.empty() )
		{
			config->m_printUsed = true;
		}
		else
		{
			config->m_printUsed = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_JUMPED_ADDRESSES )
	{
		if( argumentArguments.empty() )
		{
			config->m_jumpedSources = true;
		}
		else
		{
			config->m_jumpedSources = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_CALLED_ADDRESSES )
	{
		if( argumentArguments.empty() )
		{
			config->m_calledSources = true;
		}
		else
		{
			config->m_calledSources = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_USELESS_ASCII_ART )
	{
		if( argumentArguments.empty() )
		{
			config->m_uselessASCIIArt = true;
		}
		else
		{
			config->m_uselessASCIIArt = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_ADDRESS_COMMENT )
	{
		if( argumentArguments.empty() )
		{
			config->m_addressComment = true;
		}
		else
		{
			config->m_addressComment = UVDArgToBool(firstArg);
		}
	}
	else if( argConfig->m_propertyForm == UVD_PROP_OUTPUT_ADDRESS_LABEL )
	{
		if( argumentArguments.empty() )
		{
			config->m_addressLabel = true;
		}
		else
		{
			config->m_addressLabel = UVDArgToBool(firstArg);
		}
	}
	//Plugins
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_NAME )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.addPlugin(firstArg));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_APPEND_PATH )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.appendPluginPath(firstArg));
	}
	else if( argConfig->m_propertyForm == UVD_PROP_PLUGIN_PREPEND_PATH )
	{
		uv_assert_ret(!argumentArguments.empty());
		uv_assert_err_ret(config->m_plugin.prependPluginPath(firstArg));
	}
	else
	{
		printf_error("Property not recognized in callback: %s\n", argConfig->m_propertyForm.c_str());
		return UV_DEBUG(UV_ERR_GENERAL);
	}

	return UV_ERR_OK;
}

static void UVDPrintVersion()
{
	if( g_config && g_config->versionPrintPrefixThunk )
	{
		g_config->versionPrintPrefixThunk();
	}
	printf_help("libuvudec version %s\n", UVDGetVersion());
	printf_help("Copyright 2009-2010 John McMaster <JohnDMcMaster@gmail.com>\n");
	printf_help("Portions copyright GNU (MD5 implementation)\n");
}

static uv_err_t UVDPrintLoadedPlugins()
{
	UVDPluginEngine *pluginEngine = &g_config->m_plugin.m_pluginEngine;

	printf_help("Loaded plugins (%d / %d):\n", pluginEngine->m_loadedPlugins.size(), pluginEngine->m_plugins.size());

	/*
	//We print args, so this list is somewhat useless, maybe as a single liner?
	for( std::map<std::string, UVDPlugin *>::iterator iter = pluginEngine->m_loadedPlugins.begin(); iter != pluginEngine->m_loadedPlugins.end(); ++iter )
	{
		UVDPlugin *plugin = (*iter).second;
		std::string name;
	
		uv_assert_ret(plugin);
		uv_assert_err_ret(plugin->getName(name));
		printf("\t%s\n", name.c_str());
	}
	*/

	return UV_ERR_OK;
}

uv_err_t printArg(UVDArgConfig *argConfig, const std::string &indent)
{
	printf_help("%s--%s (%s): %s\n",
			indent.c_str(), argConfig->m_longForm.c_str(), argConfig->m_propertyForm.c_str(),
			argConfig->m_helpMessage.c_str());
	if( !argConfig->m_helpMessageExtra.empty() )
	{
		printf_help("%s%s", indent.c_str(), argConfig->m_helpMessageExtra.c_str());
	}
	return UV_ERR_OK;
}

static uv_err_t UVDPrintUsage()
{
	const char *program_name = "";
	UVDPluginEngine *pluginEngine = NULL;
	
	uv_assert_ret(g_config);
	pluginEngine = &g_config->m_plugin.m_pluginEngine;
	
	if( g_config->m_argv )
	{
		program_name = g_config->m_argv[0];
	}

	printf_help("\n");
	printf_help("Usage: %s <args>\n", program_name);
	printf_help("Args:\n");
	for( std::vector<UVDArgConfig>::size_type i = 0; i < g_config->m_configArgs.size(); ++i )
	{
		UVDArgConfig *argConfig = g_config->m_configArgs[i];
		
		uv_assert_ret(argConfig);
		
		//Print main config first
		if( pluginEngine->m_pluginArgMap.find(argConfig) == pluginEngine->m_pluginArgMap.end() )
		{
			uv_assert_err_ret(printArg(argConfig, ""));
		}
	}
	
	printf_help("\n");
	UVDPrintLoadedPlugins();

	//Now do it by plugin type
	//this isn't terribly efficient, but who cares we should only have a handful of plugins
	for( std::map<std::string, UVDPlugin *>::iterator iter = pluginEngine->m_loadedPlugins.begin();
			iter != pluginEngine->m_loadedPlugins.end(); ++iter )
	{	
		UVDPlugin *plugin = (*iter).second;
		std::string pluginName = (*iter).first;
		
		uv_assert_ret(plugin);
		printf_help("\n");
		printf_help("Plugin %s:\n", pluginName.c_str());
		
		for( std::map<UVDArgConfig *, std::string>::iterator iter = pluginEngine->m_pluginArgMap.begin();
				iter != pluginEngine->m_pluginArgMap.end(); ++iter )
		{
			UVDArgConfig *argConfig = (*iter).first;
			std::string currentPluginName = (*iter).second;
			
			uv_assert_ret(argConfig);
			
			if( pluginName == currentPluginName )
			{
				uv_assert_err_ret(printArg(argConfig, ""));
			}
		}
	}
	
	printf_help("\n");
	
	return UV_ERR_OK;
}

void UVDHelp()
{
	UVDPrintVersion();
	UVDPrintUsage();
}
