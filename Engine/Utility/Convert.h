#pragma once
#ifndef LEVIATHAN_CONVERT
#define LEVIATHAN_CONVERT
// ------------------------------------ //

namespace Leviathan{

#define STRINGTOSOMETHINGTEMPLATEALTERNATIVE(StringyType, strstreamt, funcname, totype) DLLEXPORT static inline totype funcname(const StringyType &str){totype tempval; strstreamt stream;stream.str(str.c_str());stream >> tempval;return tempval;}
	
	//! \brief Holds common conversion functions
	class Convert{
	public:

		DLLEXPORT static double DegreesToRadians(float degrees);
		DLLEXPORT static double RadiansToDegrees(float radians);


		DLLEXPORT static std::wstring IntToWstring(const int &i);
		DLLEXPORT static std::wstring FloatToWstring(const float &i);
		DLLEXPORT static std::wstring CharToWstring(const char &i);

		//! \brief Function That converts a string to wide string preserving character codes (not utf8)
		//! \see Utf8ToUtf16
		DLLEXPORT static std::wstring StringToWstring(const std::string &str);

		//! \brief Function That converts a string to wide string preserving character codes (not utf8)
		//! \see Utf8ToUtf16
		DLLEXPORT static std::wstring StringToWstringNonRef(const std::string str);

		//! \brief Function that converts a wstring to a string (potentially messing up characters)
		//! \see Utf16ToWstring
		DLLEXPORT static std::string WstringToString(const std::wstring &str);


		template<class StringStreamType, class ReturnType>
		DLLEXPORT static ReturnType Float3ToSWstring(const Float3 &data){

			StringStreamType stream;

			stream << "[" << data.X << ", " << data.Y << ", " << data.Z << "]";
			return stream.str();
		}

		template<class StringStreamType, class ReturnType>
		DLLEXPORT static ReturnType Float4ToString(const Float4 &data){

			StringStreamType stream;

			stream << "[" << data.X << ", " << data.Y << ", " << data.Z << ", " << data.W << "]";
			return stream.str();
		}

		DLLEXPORT static int WstringFromBoolToInt(const std::wstring &i);
		DLLEXPORT static int StringFromBoolToInt(const std::string &i);

		DLLEXPORT static bool IsWstringBool(const std::wstring &val, bool* valreceiver = NULL);

		template<class T>
		DLLEXPORT static inline T WstringTo(const std::wstring &str){
			T tempval;
			wstringstream stream;
			stream.str(str.c_str());
			stream >> tempval;
			return tempval;
		}
		template<class T>
		DLLEXPORT static inline T StringTo(const std::string &str){
			T tempval;
			stringstream stream;
			stream.str(str.c_str());
			stream >> tempval;
			return tempval;
		}


		// macro conversions //
		STRINGTOSOMETHINGTEMPLATEALTERNATIVE(std::wstring, std::wstringstream, WstringToInt, int);
		STRINGTOSOMETHINGTEMPLATEALTERNATIVE(std::wstring, std::wstringstream, WstringToFloat, float);


		DLLEXPORT static wchar_t ToLower(const wchar_t &chara);
		DLLEXPORT static void ToLower(const std::wstring &source, std::wstring &target);
		DLLEXPORT static void ToCapital(const std::wstring &source, std::wstring &target);

		// type checks //
		DLLEXPORT static int WstringTypeCheck(const std::wstring& data, int typecheckfor);
		DLLEXPORT static int WstringTypeNameCheck(const std::wstring &data);

		// template functions //

		template<typename T>
		DLLEXPORT static std::wstring ToWstring(const T& val){
			std::wstringstream stream;
			if(!(stream << val)){
				return L"";
			}
			return stream.str();
		}
		template<typename T>
		DLLEXPORT static std::string ToString(const T& val){
			std::stringstream stream;
			if(!(stream << val)){
				return "";
			}
			return stream.str();
		}

		template<class T>
		DLLEXPORT static std::wstring ToHexadecimalWstring(const T& val){
			std::wstringstream stream;
			if(!(stream << std::hex << val)){
				return L"";
			}
			return stream.str();
		}


		//! \brief Decodes an UTF8 string to an UTF16 string (wide string/wstring)
		//! \return The converted string or an empty string in case the input string is invalid/has invalid format
		DLLEXPORT static std::wstring Utf8ToUtf16(const string &utf8str);

		//! \brief Encodes an UTF8 string from a wide string (wstring/utf16)
		//! \return The converted string or an empty string in case the input string is invalid/has invalid format
		DLLEXPORT static std::string Utf16ToUtf8(const wstring &utf16str);



	};

}

#endif