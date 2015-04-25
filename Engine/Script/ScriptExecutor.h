#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <string>
#include <memory>

// angelscript //
//#define ANGELSCRIPT_DLL_LIBRARY_IMPORT
#include "angelscript.h"


#include "Script/ScriptScript.h"
#include "Common/DataStoring/DataBlock.h"
#include "Script/ScriptRunningSetup.h"
#include "Handlers/IDFactory.h"


#define ANGELSCRIPT_REGISTERFAIL	Logger::Get()->Error("ScriptExecutor: Init: AngelScript: "\
    "register global failed in file " __FILE__ " on line "+Convert::ToString(__LINE__)); \
    return false;

namespace Leviathan{

    //! \todo Make the module finding more efficient, store module IDs in all call sites
	class ScriptExecutor{
		friend ScriptModule;
	public:
		DLLEXPORT ScriptExecutor();
		DLLEXPORT ~ScriptExecutor();

		DLLEXPORT void ScanAngelScriptTypes();

		// module managing //
		DLLEXPORT std::weak_ptr<ScriptModule> CreateNewModule(const std::string &name,
            const std::string &source, const int &modulesid = IDFactory::GetID());
        
		DLLEXPORT void DeleteModule(ScriptModule* ptrtomatch);
		DLLEXPORT bool DeleteModuleIfNoExternalReferences(int ID);
		DLLEXPORT std::weak_ptr<ScriptModule> GetModule(const int &ID);
		DLLEXPORT std::weak_ptr<ScriptModule> GetModuleByAngelScriptName(const char* nameofmodule);

		DLLEXPORT inline asIScriptEngine* GetASEngine(){
			return engine;
		}

		DLLEXPORT int GetAngelScriptTypeID(const std::string &typesname);


		//! \brief Runs a script
		DLLEXPORT std::shared_ptr<VariableBlock> RunSetUp(ScriptScript* scriptobject,
            ScriptRunningSetup* parameters);

		//! \brief Runs a script
		DLLEXPORT std::shared_ptr<VariableBlock> RunSetUp(ScriptModule* scrptmodule,
            ScriptRunningSetup* parameters);

		//! \brief Runs a script function whose pointer is passed in
		DLLEXPORT std::shared_ptr<VariableBlock> RunSetUp(asIScriptFunction* function,
            ScriptRunningSetup* parameters);


		DLLEXPORT static ScriptExecutor* Get();
        
	private:

		void PrintAdditionalExcept(asIScriptContext *ctx);

		//! \brief Handles the return type and return value of a function
        std::shared_ptr<VariableBlock> _GetScriptReturnedVariable(int retcode,
            asIScriptContext* ScriptContext,
            ScriptRunningSetup* parameters, asIScriptFunction* func, ScriptModule* scrptmodule,
            FunctionParameterInfo* paraminfo);

		//! \brief Handles passing parameters to a context
		bool _SetScriptParameters(asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
            ScriptModule* scrptmodule, FunctionParameterInfo* paraminfo);

		//! \brief Checks whether a function is a valid pointer
		bool _CheckScriptFunctionPtr(asIScriptFunction* func, ScriptRunningSetup* parameters,
            ScriptModule* scrptmodule);

		//! \brief Prepares a context for usage
		bool _PrepareContextForPassingParameters(asIScriptFunction* func,
            asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
            ScriptModule* scrptmodule);

		//! \brief Called when a context is required for script execution
		//! \todo Add a pool from which these are retrieved
		asIScriptContext* _GetContextForExecution();

		//! \brief Called after a script has been executed and the context is no longer needed
		void _DoneWithContext(asIScriptContext* context);

		// ------------------------------ //
		// AngelScript engine script executing part //
		asIScriptEngine* engine;
        
		// list of modules that have been created, some might only have this as reference, and
        // could potentially be released
        std::vector<std::shared_ptr<ScriptModule>> AllocatedScriptModules;


		// map of type name and engine type id //
		static std::map<int, std::string> EngineTypeIDS;
        
		// inverted of the former for better performance //
		static std::map<std::string, int> EngineTypeIDSInverted;
        
		static ScriptExecutor* instance;
	};

}

