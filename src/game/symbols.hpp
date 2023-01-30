#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	// Functions

	WEAK symbol<int(const char* str)> BG_StringHashValue{0x0, 0x0};

	WEAK symbol<void(const char* fmt, ...)> G_LogPrintf{0x69EA30, 0x5CD250};
	WEAK symbol<int(int clientNum)> G_GetClientScore{0x0, 0x4691D0};

	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_InsertText{0x0, 0x0};
	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{0x49B930, 0x56EF70};
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* text)> Cmd_ExecuteSingleCommand{0x619D00, 0x50B470};
	WEAK symbol<void(const char* cmdName, void(), cmd_function_t* allocedCmd)> Cmd_AddCommandInternal{0x661400, 0x6AD580};
	WEAK symbol<const char*(int index)> Cmd_Argv{0x0, 0x0};
	WEAK symbol<void(const char* cmdName)> Cmd_RemoveCommand{0x5F1A90, 0x527EA0};

	WEAK symbol<void(int clientNum)> ClientUserInfoChanged{0x0, 0x0};

	WEAK symbol<void(errorParm_t code, const char* fmt, ...)> Com_Error{0x651D90, 0x627380};
	WEAK symbol<void(int channel, const char* fmt, ...)> Com_Printf{0x43BF30, 0x4126C0};
	WEAK symbol<void(const char* fmt, ...)> Com_Printf_NoFilter{0x566BC0, 0x64C260};
	WEAK symbol<void(int channel, const char* fmt, ...)> Com_PrintWarning{0x51CE50, 0x54B6A0};

	WEAK symbol<const dvar_t*(const char*)> Dvar_FindVar{0x5AE810, 0x512F70};
	WEAK symbol<int(const dvar_t*)> Dvar_GetInt{0x0, 0x0};
	WEAK symbol<dvar_t*(const char* dvarName, int value, int min, int max, 
		unsigned int flags, const char* description)> Dvar_RegisterInt{0x0, 0x0};
	WEAK symbol<char*(int localClientNum, int bit)> Dvar_InfoString{0x613060, 0x54A8B0};

	WEAK symbol<XAssetHeader(XAssetType type, const char* name, bool errorIfMissing, int waitTime)> DB_FindXAssetHeader{0x0, 0x0};

	WEAK symbol<char*(const char*)> I_CleanStr{0x0, 0x0};
	WEAK symbol<void(char* dest, const char* src, int destsize)> I_strncpyz{0x5D4D60, 0x5A7140};
	WEAK symbol<int(const char* str)> I_DrawStrlen{0x0, 0x6A8EF0};

	WEAK symbol<void*(const char** pName, int* min_args, int* max_args)> Player_GetMethod{0x0, 0x0};
	WEAK symbol<void*(const char** pName, int* type, int* min_args, int* max_args)> Scr_GetCommonFunction{0x0, 0x0};
	WEAK symbol<void*(const char** pName, int* type, int* min_args, int* max_args)> Scr_GetMethod{0x0, 0x0};

	WEAK symbol<void(scriptInstance_t inst, gentity_s* ent)> Scr_AddEntity{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, float value)> Scr_AddFloat{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, int value)> Scr_AddInt{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, const char* value)> Scr_AddString{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, float* value)> Scr_AddVector{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int id)> Scr_AddObject{0x0, 0x0};

	WEAK symbol<void(scriptInstance_t inst)> Scr_ClearOutParams{0x654D10, 0x588680};

	WEAK symbol<unsigned int(scriptInstance_t inst)> AllocObject{0x603400, 0x6970B0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int id)> AllocThread{0x69E140, 0x43CA60};
	WEAK symbol<void(scriptInstance_t inst, unsigned int id)> RemoveRefToObject{0x5517B0, 0x698FA0};
	WEAK symbol<void(scriptInstance_t inst, const float* vectorValue)> RemoveRefToVector{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, const int type, VariableUnion value)> AddRefToValue_{0x53FD50, 0x6706B0};

	WEAK symbol<const char*(netadr_t a)> NET_AdrToString{0x40D790, 0x49F970};

	namespace mp
	{
		WEAK symbol<void(scriptInstance_t inst, const int type, VariableUnion value)> RemoveRefToValue{0x0, 0x4249C0};
	}

	namespace sp
	{
		WEAK symbol<void(scriptInstance_t inst, VariableValue* value)> RemoveRefToValue{0x556830, 0x0};
	}

	WEAK symbol<void(scriptInstance_t inst, unsigned int parentId, unsigned int index)> RemoveVariableValue{0x0, 0x0};

	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int id)> FindVariable{0x476720, 0x5DF7E0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int id)> FindArrayVariable{0x62F8E0, 0x509230};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int id)> GetVariable{0x0, 0x0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int name)> GetNewVariable{0x674820, 0x5B3750};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int parentId, unsigned int unsignedValue)> GetNewArrayVariable{0x689C60, 0x468BC0};

	WEAK symbol<void(unsigned int classnum, int entnum, int offset)> Scr_SetObjectField{0x0, 0x0};
	WEAK symbol<VariableValue(scriptInstance_t inst, unsigned int classnum, int entnum, int clientNum, int offset)> GetEntityFieldValue{0x0, 0x0};

	WEAK symbol<void(gentity_s* ent, unsigned int stringValue, unsigned int paramcount)> Scr_Notify{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, int clientNum, int entnum, 
		unsigned int classnum, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyNum_Internal{0x41D270, 0x4162E0};
	WEAK symbol<void(int entnum, unsigned int classnum, unsigned int stringValue, unsigned int paramcount)> Scr_NotifyNum{0x0, 0x0};

	WEAK symbol<unsigned int(const char* str, unsigned int user, scriptInstance_t inst)> SL_GetString{0x463130, 0x4B1770};
	WEAK symbol<const char*(unsigned int stringValue, scriptInstance_t inst)> SL_ConvertToString{0x687530, 0x624C70};
	WEAK symbol<unsigned int(scriptInstance_t inst, const char* str)> SL_GetCanonicalString{0x4FB120, 0x68D860};

	WEAK symbol<int(scriptInstance_t inst)> Scr_GetNumParam{0x0, 0x0};
	WEAK symbol<gentity_s*(scriptInstance_t inst, int index)> Scr_GetEntity{0x0, 0x0};
	WEAK symbol<float(scriptInstance_t inst, int index)> Scr_GetFloat{0x0, 0x0};
	WEAK symbol<int(scriptInstance_t inst, int index)> Scr_GetInt{0x0, 0x0};
	WEAK symbol<const char*(scriptInstance_t inst, int index)> Scr_GetString{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, int index, float* out)> Scr_GetVector{0x0, 0x0};
	WEAK symbol<const float*(scriptInstance_t inst, const float* v)> Scr_AllocVector{0x41B400, 0x4C4440};
	WEAK symbol<int(scriptInstance_t inst, const char* filename, 
		const char* name, unsigned int* checksum, bool errorIfMissing)> Scr_GetFunctionHandle{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int classnum, char const* name, unsigned int offset)> Scr_AddClassField{0x0, 0x0};
	WEAK symbol<unsigned int(scriptInstance_t inst, int entnum, unsigned int classnum, int clientNum)> Scr_GetEntityId{0x4B7050, 0x6A07D0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int localId)> GetStartLocalId{0x0, 0x0};
	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int localId)> Scr_TerminateRunningThread{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, const char* error, bool force_terminal)> Scr_Error{0x4B5940, 0x6245E0};
	WEAK symbol<void(scriptInstance_t inst, unsigned int paramIndex, const char* error)> Scr_ParamError{0x0, 0x0};
	WEAK symbol<void(scriptInstance_t inst, const char* error)> Scr_ObjectError{0x0, 0x0};
	WEAK symbol<scr_entref_t*(scr_entref_t* refref, scriptInstance_t inst, unsigned int entId)> Scr_GetEntityIdRef{0x433B60, 0x6A53C0};
	WEAK symbol<unsigned int(scriptInstance_t inst)> Scr_AllocArray{0x438800, 0x5B8400};

	WEAK symbol<gentity_s*(scr_entref_t entref)> GetPlayerEntity{0x0, 0x0};

	WEAK symbol<unsigned int(scriptInstance_t inst, unsigned int localId, const char* pos, unsigned int paramcount)> VM_Execute{0x8ACE60, 0x8EADE0};

	WEAK symbol<void(int clientNum, const char* reason)> SV_GameDropClient{0x0, 0x0};
	WEAK symbol<void(int clientNum, int type, const char* command)> SV_GameSendServerCommand{0x543CF0, 0x6B8730};
	WEAK symbol<playerState_s*(int num)> SV_GameClientNum{0x0, 0x6A2570};

	WEAK symbol<void*(int valueIndex)> Sys_GetValue{0x67D4F0, 0x529EB0};
	WEAK symbol<int()> Sys_Milliseconds{0x0, 0x0};

	WEAK symbol<int(const char* qpath, int* f, fsMode_t mode)> FS_FOpenFileByMode{0x4DD530, 0x40C790};
	WEAK symbol<void(int h)> FS_FCloseFile{0x46CAA0, 0x533020};

	WEAK symbol<void*(jmp_buf* Buf, int Value)> longjmp{0x96B980, 0x9D05C4};
	WEAK symbol<int(jmp_buf* Buf, int a2)> _setjmp{0x969EAC, 0x9CED5C};

	// Variables

	WEAK symbol<int> g_script_error_level{0x32D1E18, 0x3DD4A18};
	WEAK symbol<jmp_buf> g_script_error{0x32D0D98, 0x3DD3998};

	WEAK symbol<scrVmPub_t> scr_VmPub{0x32C8738, 0x3DCB338};
	WEAK symbol<scrVarGlob_t> scr_VarGlob{0x32C8580, 0x3DCB180};
	WEAK symbol<scrVarPub_t> scr_VarPub{0x32C8680, 0x3DCB280};
	WEAK symbol<int> scr_starttime{0x0, 0x0};
	WEAK symbol<function_stack_t> fs{0x0, 0x0};

	WEAK symbol<unsigned short> sv_configstrings{0x0, 0x0};

	WEAK symbol<scr_classStruct_t*> g_classMap{0x0, 0x0};

	WEAK symbol<gentity_s> g_entities{0x0, 0x0};
	WEAK symbol<unsigned int> levelEntityId{0x32C86A0, 0x3DCB2A0};

	WEAK symbol<client_s> svs_clients{0x0, 0x372D11C};
	WEAK symbol<int> svs_time{0x0, 0x372D10C};

	WEAK symbol<CmdArgs> sv_cmd_args{0x243D208, 0x355BD88};

	WEAK symbol<int> logFile{0x1C0417C, 0x3443838};
}
