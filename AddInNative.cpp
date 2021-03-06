﻿
#include "stdafx.h"

#include <stdio.h>
#include <wchar.h>
#include "AddInNative.h"
#include <string>

static std::map<std::u16string, long> mMethods;
static std::vector<std::u16string> vMethods;
static std::map<std::u16string, long> mMethods_ru;
static std::vector<std::u16string> vMethods_ru;

static std::map<std::u16string, long> mProps;
static std::vector<std::u16string> vProps;
static std::map<std::u16string, long> mProps_ru;
static std::vector<std::u16string> vProps_ru;

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
void convertUTF16ToUTF32(const char16_t *input, const size_t input_size, std::basic_string<wchar_t> &output);
unsigned int convertUTF32ToUTF16(const wchar_t *input, size_t input_size, char16_t *output);
#endif

static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
	if (!*pInterface)
	{
		*pInterface = new CAddInNative;
		return (long)*pInterface;
	}
	return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
	g_capabilities = capabilities;
	return eAppCapabilitiesLast;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	if (!*pIntf)
		return -1;

	delete *pIntf;
	*pIntf = 0;
	return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	static char16_t cls_names[] = u"CAddInNative";
	return reinterpret_cast<WCHAR_T *>(cls_names);
}
//---------------------------------------------------------------------------//

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
	m_iMemory = 0;
	m_iConnect = 0;

	iCurrentPosition = 0;
	m_PropCountOfItemsInSearchResult = 0;
	sErrorDescription = "";
	bThrowExceptions = false;
	bIgnoreCase = false;
	isPattern = false;
	bGlobal = false;

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
	uPattern.clear();
#endif

	if (mMethods.size() == 0) {
		mMethods = {
		{u"matches", eMethMatches},
		{u"ismatch", eMethIsMatch},
		{u"next", eMethNext},
		{u"replace", eMethReplace},
		{u"count", eMethCount},
		{u"version", eMethVersion},
		};
		vMethods = { u"Matches", u"IsMatch", u"Next", u"Replace", u"Count", u"Version" };
	}

	if (mMethods_ru.size() == 0) {
		mMethods_ru = {
		{u"найтисовпадения", eMethMatches},
		{u"совпадает", eMethIsMatch},
		{u"следующий", eMethNext},
		{u"заменить", eMethReplace},
		{u"количество", eMethCount},
		{u"версия", eMethVersion},
		};
		vMethods_ru = { u"НайтиСовпадения", u"Совпадает", u"Следующий", u"Заменить", u"Количество", u"Версия" };
	}

	if (mProps.size() == 0) {
		mProps = {
		{u"currentvalue", ePropCurrentValue},
		{u"ignorecase", ePropIgnoreCase},
		{u"errordescription", ePropErrorDescription},
		{u"throwexceptions", ePropThrowExceptions},
		{u"pattern", ePropPattern},
		{u"global", ePropGlobal},
		};
		vProps = { u"CurrentValue", u"IgnoreCase", u"ErrorDescription", u"ThrowExceptions", u"Pattern", u"Global" };
	}

	if (mProps_ru.size() == 0) {
		mProps_ru = {
		{u"текущеезначение", ePropCurrentValue},
		{u"игнорироватьрегистр", ePropIgnoreCase},
		{u"описаниеошибки", ePropErrorDescription},
		{u"вызыватьисключения", ePropThrowExceptions},
		{u"шаблон", ePropPattern},
		{u"всесовпадения", ePropGlobal},
		};
		vProps_ru = { u"ТекущееЗначение", u"ИгнорироватьРегистр", u"ОписаниеОшибки", u"ВызыватьИсключения", u"Шаблон", u"ВсеСовпадения" };
	}
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{
	m_iConnect = (IAddInDefBase*)pConnection;
	return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{
	// Component should put supported component technology version 
	// This component supports 2.0 version
	return 2000;
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{

}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
	char16_t name[] = u"RegEx";

	if (!m_iMemory || !m_iMemory->AllocMemory(reinterpret_cast<void **>(wsExtensionName), sizeof(name))) {
		return false;
	};

	memcpy(*wsExtensionName, name, sizeof(name));

	return true;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{
	return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{
	std::basic_string<char16_t> usPropName = (char16_t*)(wsPropName);
	std::transform(usPropName.begin(), usPropName.end(), usPropName.begin(), ::towlower);

	auto it = mProps.find(usPropName);
	if (it != mProps.end())
		return it->second;

	it = mProps_ru.find(usPropName);
	if (it != mProps_ru.end())
		return it->second;

	return -1;
}

//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{
	if (lPropNum >= ePropLast)
		return NULL;

	std::basic_string<char16_t> *usCurrentName;

	switch (lPropAlias)
	{
	case 0: // First language
		usCurrentName = &vProps[lPropNum];
		break;
	case 1: // Second language
		usCurrentName = &vProps_ru[lPropNum];
		break;
	default:
		return 0;
	}

	if (usCurrentName->length() == 0) {
		return nullptr;
	}

	WCHAR_T *result = nullptr;

	size_t bytes = (usCurrentName->length() + 1) * sizeof(char16_t);

	if (!m_iMemory || !m_iMemory->AllocMemory(reinterpret_cast<void **>(&result), bytes)) {
		return nullptr;
	};

	memcpy(result, usCurrentName->c_str(), bytes);

	return result;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	switch (lPropNum)
	{
	case ePropCurrentValue:
	{
		TV_VT(pvarPropVal) = VTYPE_PWSTR;
		std::wstring* wsCurrentValue = &vResults[iCurrentPosition];

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pwstrVal, (wsCurrentValue->length() + 1) * sizeof(char32_t)))
		{
			memcpy(pvarPropVal->pwstrVal, wsCurrentValue->c_str(), (wsCurrentValue->length() + 1) * sizeof(wchar_t));
			pvarPropVal->wstrLen = wsCurrentValue->length();

			convertUTF32ToUTF16(wsCurrentValue->c_str(), wsCurrentValue->length(), pvarPropVal->pwstrVal);
			pvarPropVal->wstrLen = wsCurrentValue->length();
			return true;
		}
#else

		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pwstrVal, (wsCurrentValue->length() + 1) * sizeof(wchar_t)))
		{
			memcpy(pvarPropVal->pwstrVal, wsCurrentValue->c_str(), (wsCurrentValue->length() + 1) * sizeof(wchar_t));
			pvarPropVal->wstrLen = wsCurrentValue->length();
			return true;
		}
#endif
		return false;
	};
	case ePropErrorDescription:
	{
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pstrVal, (sErrorDescription.length() + 1) * sizeof(char)))
		{
			strcpy(pvarPropVal->pstrVal, sErrorDescription.c_str());
			TV_VT(pvarPropVal) = VTYPE_PSTR;
			pvarPropVal->strLen = sErrorDescription.length();
			return true;
		}
		else
		{
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->pwstrVal = NULL;
			pvarPropVal->wstrLen = 0;
			return true;
		}
	}
	case ePropThrowExceptions:
	{
		TV_VT(pvarPropVal) = VTYPE_BOOL;
		pvarPropVal->bVal = bThrowExceptions;
		return true;
	}
	case ePropPattern:
	{
#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pwstrVal, (uPattern.length() + 1) * sizeof(WCHAR)))
		{
			memcpy(pvarPropVal->pwstrVal, uPattern.c_str(), uPattern.length() * sizeof(WCHAR));
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->wstrLen = uPattern.length();
			return true;
	}
#else
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pwstrVal, (rePattern.str().length() + 1) * sizeof(WCHAR)))
		{
			memcpy(pvarPropVal->pwstrVal, rePattern.str().c_str(), rePattern.str().length() * sizeof(WCHAR));
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->wstrLen = rePattern.str().length();
			return true;
		}
#endif
		else
		{
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->pwstrVal = NULL;
			pvarPropVal->wstrLen = 0;
			return true;
		}
	}
	case ePropGlobal:
	{
		TV_VT(pvarPropVal) = VTYPE_BOOL;
		pvarPropVal->bVal = bGlobal;
		return true;
	}
	default:
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{
	SetLastError("");

	switch (lPropNum)
	{
	case ePropThrowExceptions:
	{
		if (TV_VT(varPropVal) != VTYPE_BOOL)
			return false;
		bThrowExceptions = TV_BOOL(varPropVal);
		return true;
	}
	case ePropIgnoreCase: {
		bIgnoreCase = TV_BOOL(varPropVal);
		return true;
	}
	case ePropPattern: {
#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
		
		uPattern.assign(varPropVal->pwstrVal, varPropVal->wstrLen);

		// Сконвертируем в строку с wchar_t символами
		std::wstring wcsPattern;
		wcsPattern.resize(varPropVal->wstrLen);
		convertUTF16ToUTF32(varPropVal->pwstrVal, varPropVal->wstrLen, wcsPattern);
		try {
			rePattern.assign(wcsPattern, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		catch (const std::exception& e)
		{
			vResults.clear();
			uPattern.clear();
			isPattern = false;
			iCurrentPosition = -1;
			m_PropCountOfItemsInSearchResult = 0;
			SetLastError(e.what());
			if (bThrowExceptions)
				return false;
			else
				return true;
		}
#else
		try
		{
			rePattern.assign(varPropVal->pwstrVal, varPropVal->wstrLen, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		catch (const std::exception& e)
		{
			vResults.clear();
			isPattern = false;
			iCurrentPosition = -1;
			m_PropCountOfItemsInSearchResult = 0;
			SetLastError(e.what());
			if (bThrowExceptions)
				return false;
			else
				return true;
		}
#endif
		isPattern = true;
		return true;
	}
	case ePropGlobal:
	{
		bGlobal = TV_BOOL(varPropVal);
		return true;
	}
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{
	switch (lPropNum)
	{
	case ePropCurrentValue:
		return true;
	case ePropErrorDescription:
		return true;
	case ePropIgnoreCase:
		return true;
	case ePropThrowExceptions:
		return true;
	case ePropPattern:
		return true;
	case ePropGlobal:
		return true;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
	switch (lPropNum)
	{
	case ePropIgnoreCase:
		return true;
	case ePropThrowExceptions:
		return true;
	case ePropPattern:
		return true;
	case ePropGlobal:
		return true;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{
	return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{

	std::basic_string<char16_t> usMethodName = (char16_t*)(wsMethodName);
	std::transform(usMethodName.begin(), usMethodName.end(), usMethodName.begin(), ::towlower);

	auto it = mMethods.find(usMethodName);
	if (it != mMethods.end())
		return it->second;

	it = mMethods_ru.find(usMethodName);
	if (it != mMethods_ru.end())
		return it->second;

	return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{

	if (lMethodNum >= eMethLast)
		return NULL;

	std::basic_string<char16_t> *usCurrentName;

	switch (lMethodAlias)
	{
	case 0: // First language
		usCurrentName = &vMethods[lMethodNum];
		break;
	case 1: // Second language
		usCurrentName = &vMethods_ru[lMethodNum];
		break;
	default:
		return 0;
	}

	WCHAR_T *result = nullptr;

	size_t bytes = (usCurrentName->length() + 1) * sizeof(char16_t);

	if (!m_iMemory || !m_iMemory->AllocMemory(reinterpret_cast<void **>(&result), bytes)) {
		return nullptr;
	};

	memcpy(result, usCurrentName->c_str(), bytes);

	return result;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethMatches:
		return 2;
	case eMethIsMatch:
		return 2;
	case eMethReplace:
		return 3;
	default:
		return 0;
	}

	return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
	tVariant *pvarParamDefValue)
{
	switch (lMethodNum)
	{
	case eMethMatches:
	{
		if (lParamNum == 1)
		{
			TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
			pvarParamDefValue->wstrLen = 0;
			return true;
		}
	}
	case eMethIsMatch:
		if (lParamNum == 1)
		{
			TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
			pvarParamDefValue->wstrLen = 0;
			return true;
		}
	case eMethNext:
		break;
	case eMethReplace:
		if (lParamNum == 1)
		{
			TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
			pvarParamDefValue->wstrLen = 0;
			return true;
		}
	default:
	{
		TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
		return false;
	}
	}

	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethNext:
		return true;
	case eMethReplace:
		return true;
	case eMethIsMatch:
		return true;
	case eMethCount:
		return true;
	case eMethVersion:
		return true;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
	tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethMatches:
		return search(paParams);
	case eMethIsMatch:
		break;
	case eMethReplace:
		break;
	case eMethCount:
		break;
	case eMethVersion:
		break;
	case eMethNext:
	{
		if (iCurrentPosition >= m_PropCountOfItemsInSearchResult) {
			return true;
		}
		else {
			iCurrentPosition++;
			return true;
		}
	}
	break;
	default:
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
	tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
		// Method acceps one argument of type BinaryData ant returns its copy
	case eMethMatches:
		break;
	case eMethIsMatch:
	{
		match(pvarRetValue, paParams);
		return true;
	}
	break;
	case eMethReplace:
		return replace(pvarRetValue, paParams);
	case eMethNext:
	{
		iCurrentPosition++;
		if (iCurrentPosition >= m_PropCountOfItemsInSearchResult) {
			TV_VT(pvarRetValue) = VTYPE_BOOL;
			pvarRetValue->bVal = false;
			return true;
		}
		else {
			TV_VT(pvarRetValue) = VTYPE_BOOL;
			pvarRetValue->bVal = true;
			return true;
		}
	}
	break;
	case eMethCount: {
		TV_VT(pvarRetValue) = VTYPE_I4;
		pvarRetValue->lVal = m_PropCountOfItemsInSearchResult;
		return true;
	}
	case eMethVersion: {
		version(pvarRetValue);
		return true;
	}
	default:
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------//
// Платформа передает в компоненту текущее название локали.
// Название локали зависит от системы и возможно от настроек платформы,
// может принимать самые различные значения: "ru_RU", "rus", "ru" и т.д.
// Название локали можно использовать для инициализации локали в стандартной библиотеке шаблонов (STL)
// языка C++. От установленной локали в STL зависит: 
// - правила конвертации из строки в число (напр. символ разделения дробной части)
// - мультибайтовая кодировка (UTF-8, CP1521 и т.д.) в которую будет преобразовываться строка из широких символов функцией wcstombs.
// - мультибайтовая кодировка, которая будет принята за основу при преобразовании строки из мультибайтовых символов в широкие функцией mbstowcs.
// - множество прочих национальных особенностей 
// На текущий момент (v1С = 8.3.6.1760), в функцию могут передаваться названия локалей которые не подходят для передачи
// в функцию setlocale и она возвращает NULL, с этим надо бы разобраться. Если функция setlocale возвращает NULL,
// то в неё передается пустая строка, это приводит к установке текущих языковых настроек операционной системы.
// 
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
#if !defined( __linux__ ) && !defined(__APPLE__) && !defined(__ANDROID__)
	_wsetlocale(LC_ALL, L"");
#else
	setlocale(LC_ALL, "");
#endif
}

/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
	m_iMemory = (IMemoryManager*)mem;
	return m_iMemory != 0;
}

bool CAddInNative::search(tVariant * paParams)
{
	SetLastError("");
	vResults.clear();

	boost::wsmatch wsmMatch;
	boost::wregex* pattern = NULL;
	bool bClearPattern = false;

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
	// Сконвертируем в строку с wchar_t символами
	std::wstring str;
	str.resize(paParams[0].wstrLen);
	convertUTF16ToUTF32(paParams[0].pwstrVal, paParams[0].wstrLen, str);
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (isPattern)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPattern = true;
			std::wstring regex_str;
			regex_str.resize(paParams[1].wstrLen);
			convertUTF16ToUTF32(paParams[1].pwstrVal, paParams[1].wstrLen, regex_str);
			pattern = new boost::wregex(regex_str, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}

		if (bGlobal)
		{
			std::wstring::const_iterator start = str.begin();
			std::wstring::const_iterator end = str.end();

			while (start < end &&
				boost::regex_search(start, end, wsmMatch, *pattern))
			{
				for (auto r : wsmMatch) {
					if (r.length() == 0)
						continue;
					vResults.push_back(r);
				}
				start = wsmMatch[0].second;
			}
		}
		else
		{
			boost::regex_search(str, wsmMatch, *pattern);
			for (auto r : wsmMatch) {
				if (r.length() == 0)
					continue;
				vResults.push_back(r);
			}
		}

	}
	catch (const std::exception& e)
	{
		SetLastError(e.what());
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPattern && pattern != NULL)
			delete pattern;

		if (bThrowExceptions)
			return false;
		else
			return true;
	}

#else
	std::wstring str;
	str.assign(paParams[0].pwstrVal, paParams[0].wstrLen);
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (isPattern)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPattern = true;
			pattern = new boost::wregex(paParams[1].pwstrVal, paParams[1].wstrLen, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}

		if (bGlobal)
		{
			std::wstring::const_iterator start = str.begin();
			std::wstring::const_iterator end = str.end();

			while (start < end &&
				boost::regex_search(start, end, wsmMatch, *pattern))
			{
				for (auto r : wsmMatch) {
					if (r.length() == 0)
						continue;
					vResults.push_back(r);
				}
				start = wsmMatch[0].second;
			}
		}
		else
		{
			boost::regex_search(str, wsmMatch, *pattern);
			for (auto r : wsmMatch) {
				if (r.length() == 0)
					continue;
				vResults.push_back(r);
			}
		}
	}
	catch (const std::exception& e)
	{
		SetLastError(e.what());
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPattern && pattern != NULL)
			delete pattern;
		if (bThrowExceptions)
			return false;
		else
			return true;
	}
#endif
	iCurrentPosition = -1;
	m_PropCountOfItemsInSearchResult = vResults.size();
	if (bClearPattern && pattern != NULL)
		delete pattern;
	return true;
}
bool CAddInNative::replace(tVariant * pvarRetValue, tVariant * paParams)
{
	SetLastError("");
	TV_VT(pvarRetValue) = VTYPE_PWSTR;
	iCurrentPosition = 0;
	m_PropCountOfItemsInSearchResult = 0;
	vResults.clear();

	boost::wregex* pattern = NULL;
	bool bClearPattern = false;

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
	// Сконвертируем в строку с wchar_t символами
	std::wstring str;
	str.resize(paParams[0].wstrLen);
	convertUTF16ToUTF32(paParams[0].pwstrVal, paParams[0].wstrLen, str);

	std::wstring replacement;
	replacement.resize(paParams[2].wstrLen);
	convertUTF16ToUTF32(paParams[2].pwstrVal, paParams[2].wstrLen, replacement);
	std::wstring res;
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (isPattern)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPattern = true;
			std::wstring pattern_str;
			pattern_str.resize(paParams[1].wstrLen);
			convertUTF16ToUTF32(paParams[1].pwstrVal, paParams[1].wstrLen, pattern_str);
			pattern = new boost::wregex(pattern_str, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		if (bGlobal)
			res = boost::regex_replace(str, *pattern, replacement);
		else
			res = boost::regex_replace(str, *pattern, replacement, boost::regex_constants::format_first_only);

	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPattern && pattern != NULL)
			delete pattern;
		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}

	if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (res.length() + 1) * sizeof(char32_t)))
	{
		convertUTF32ToUTF16(res.c_str(), res.length(), pvarRetValue->pwstrVal);
		pvarRetValue->wstrLen = res.length();
		return true;
	}
#else

	std::wstring str;
	str.assign(paParams[0].pwstrVal, paParams[0].wstrLen);
	std::wstring res;
	try
	{
		boost::wregex* pattern;
		if (paParams[1].wstrLen == 0)
		{
			if (isPattern)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPattern = true;
			pattern = new boost::wregex(paParams[1].pwstrVal, paParams[1].wstrLen, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		if (bGlobal)
			res = boost::regex_replace(str, *pattern, paParams[2].pwstrVal);
		else
			res = boost::regex_replace(str, *pattern, paParams[2].pwstrVal, boost::regex_constants::format_first_only);
	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPattern && pattern != NULL)
			delete pattern;

		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}

	if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (res.length() + 1) * sizeof(wchar_t)))
	{
		memcpy(pvarRetValue->pwstrVal, res.c_str(), (res.length() + 1) * sizeof(wchar_t));
		pvarRetValue->wstrLen = res.length();
		return true;
	}
#endif

	if (bClearPattern && pattern != NULL)
		delete pattern;

	return false;
}

bool CAddInNative::match(tVariant * pvarRetValue, tVariant * paParams)
{
	SetLastError("");
	TV_VT(pvarRetValue) = VTYPE_BOOL;

	boost::wregex* pattern = NULL;
	bool bClearPattern = false;

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)
	std::wstring str;
	str.resize(paParams[0].wstrLen);
	convertUTF16ToUTF32(paParams[0].pwstrVal, paParams[0].wstrLen, str);
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (isPattern)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPattern = true;
			std::wstring pattern_str;
			pattern_str.resize(paParams[1].wstrLen);
			convertUTF16ToUTF32(paParams[1].pwstrVal, paParams[1].wstrLen, pattern_str);
			pattern = new boost::wregex(pattern_str, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		pvarRetValue->bVal = boost::regex_match(str, *pattern);

	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPattern && pattern != NULL)
			delete pattern;
		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}

#else
	try
	{
		boost::wregex* pattern;
		if (paParams[1].wstrLen == 0)
		{
			if (isPattern)
				pattern = &rePattern;
			else
				return true;
		}
		else
			pattern = new boost::wregex(paParams[1].pwstrVal, paParams[1].wstrLen, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);

		pvarRetValue->bVal = boost::regex_match(paParams[0].pwstrVal, *pattern);
	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPattern && pattern != NULL)
			delete pattern;
		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}
#endif
	iCurrentPosition = 0;
	m_PropCountOfItemsInSearchResult = 0;
	vResults.clear();
	if (bClearPattern && pattern != NULL)
		delete pattern;
	return true;
}

void CAddInNative::version(tVariant * pvarRetValue)
{
	TV_VT(pvarRetValue) = VTYPE_PWSTR;
	std::basic_string<char16_t> res = u"9";

	if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (res.length() + 1) * sizeof(char16_t)))
	{
		memcpy(pvarRetValue->pwstrVal, res.c_str(), (res.length() + 1) * sizeof(char16_t));
		pvarRetValue->wstrLen = res.length();
	}
}

//---------------------------------------------------------------------------//

// Устанавливает значение переменной sErrorDescription класса CAddInNative.
// Если переменная sErrorDescription не пустая, то значение добавляется в начало строки,
// а уже существующий текст, присоединяется через двоеточие справа
// такой подход принят для реализации механизма вложенности описания ошибки
// на самом первом этапе, в текст ошибки добавляется её первопричина, а на более 
// высоких уровнях её можно дополнить описанием о выполняемых действиях
//
void CAddInNative::SetLastError(const char* error) {

	if (error == "") sErrorDescription.clear();
	if (sErrorDescription.length() != 0) sErrorDescription = std::string(error) + ": " + sErrorDescription;
	else sErrorDescription = error;

}

#if defined( __linux__ ) || defined(__APPLE__) || defined(__ANDROID__)

inline int is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
inline int is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

inline char32_t surrogate_to_utf32(char16_t high, char16_t low) {
	return (high << 10) + low - 0x35fdc00;
}

// The algorithm is based on this answer:
//https://stackoverflow.com/a/23920015/2134488
//
void convertUTF16ToUTF32(const char16_t *input,
	const size_t input_size,
	std::basic_string<wchar_t> &output)
{
	int i = 0;
	const char16_t * const end = input + input_size;
	while (input < end) {
		const char16_t uc = *input++;
		if (!((uc - 0xd800u) < 2048u)) {
			output[i++] = uc;
		}
		else {
			if (is_high_surrogate(uc) && input < end && is_low_surrogate(*input)) {
				output[i++] = (wchar_t)surrogate_to_utf32(uc, *input++);
			}
			else {
				output[i++] = 0; //ERROR
			}
		}
	}
}

// The algorithm is based on this answer:
//https://stackoverflow.com/questions/955484/is-it-possible-to-convert-utf32-text-to-utf16-using-only-windows-api
//
unsigned int convertUTF32ToUTF16(const wchar_t *input, size_t input_size, char16_t *output)
{
	char16_t *start = output;
	const wchar_t * const end = input + input_size;
	while (input < end) {
		const wchar_t cUTF32 = *input++;
		if (cUTF32 < 0x10000)
		{
			*output++ = cUTF32;
		}
		else {
			unsigned int t = cUTF32 - 0x10000;
			wchar_t h = (((t << 12) >> 22) + 0xD800);
			wchar_t l = (((t << 22) >> 22) + 0xDC00);
			*output++ = h;
			*output++ = (l & 0x0000FFFF);
		}
	}
	return (output - start) * 2; // size in bytes
}

#endif