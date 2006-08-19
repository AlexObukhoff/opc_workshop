//	delayimphlp.h: Delay import helper library
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		26.09.2005
//  Version     1.1.3
//  http://gzip.rsdn.ru/article/cpp/delayload.xml

#ifndef	__DELAYIMPHLP_H__
#define __DELAYIMPHLP_H__


#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#ifndef DL_NO_MT
	#ifdef _MT
		#define DL_MT
	#endif
#endif

#pragma warning (disable: 4786)

#ifndef DL_USE_BOOST_PP

#define DL_CAT_(x,y) x##y
#define DL_CAT(x,y) DL_CAT_(x,y)

#define DL_STRINGIZE_(x) #x
#define DL_STRINGIZE(x) DL_STRINGIZE_(x)


#define MAX_DL_REPEAT 16
#define DL_COMMA() ,
#define DL_EMPTY() 


#define DL_BOOL(x)      DL_BOOL_IMPL(x)
#define DL_BOOL_IMPL(x) DL_CAT(DL_BOOL_N, x)

#define DL_BOOL_N0  0
#define DL_BOOL_N1  1
#define DL_BOOL_N2  1
#define DL_BOOL_N3  1
#define DL_BOOL_N4  1
#define DL_BOOL_N5  1
#define DL_BOOL_N6  1
#define DL_BOOL_N7  1
#define DL_BOOL_N8  1
#define DL_BOOL_N9  1
#define DL_BOOL_N10 1
#define DL_BOOL_N11 1
#define DL_BOOL_N12 1
#define DL_BOOL_N13 1
#define DL_BOOL_N14 1
#define DL_BOOL_N15 1
#define DL_BOOL_N16 1


#define DL_IF(c, t, f)       DL_IF_IMPL1(c, t, f)
#define DL_IF_IMPL1(c, t, f) DL_IF_IMPL2(DL_BOOL(c), t ,f)
#define DL_IF_IMPL2(c, t, f) DL_CAT(DL_IF_IMPL_N, c)(t, f)
#define DL_IF_IMPL_N0(_, f) f
#define DL_IF_IMPL_N1(t, _) t


#define DL_COMMA_IF(c) DL_COMMA_IF_IMPL(c)
#define DL_COMMA_IF_IMPL(c) DL_IF(c, DL_COMMA, DL_EMPTY)()


// DL_REPEAT_N
#define DL_REPEAT_IMPL_N_0(x, d) 
#define DL_REPEAT_IMPL_N_1(x, d)	DL_CAT(x,0)
#define DL_REPEAT_IMPL_N_2(x, d)	DL_REPEAT_IMPL_N_1(x, d)d()  DL_CAT(x,1)
#define DL_REPEAT_IMPL_N_3(x, d)	DL_REPEAT_IMPL_N_2(x, d)d()  DL_CAT(x,2)
#define DL_REPEAT_IMPL_N_4(x, d)	DL_REPEAT_IMPL_N_3(x, d)d()  DL_CAT(x,3)
#define DL_REPEAT_IMPL_N_5(x, d)	DL_REPEAT_IMPL_N_4(x, d)d()  DL_CAT(x,4)
#define DL_REPEAT_IMPL_N_6(x, d)	DL_REPEAT_IMPL_N_5(x, d)d()  DL_CAT(x,5)
#define DL_REPEAT_IMPL_N_7(x, d)	DL_REPEAT_IMPL_N_6(x, d)d()  DL_CAT(x,6)
#define DL_REPEAT_IMPL_N_8(x, d)	DL_REPEAT_IMPL_N_7(x, d)d()  DL_CAT(x,7)
#define DL_REPEAT_IMPL_N_9(x, d)	DL_REPEAT_IMPL_N_8(x, d)d()  DL_CAT(x,8)
#define DL_REPEAT_IMPL_N_10(x, d)	DL_REPEAT_IMPL_N_9(x, d)d()  DL_CAT(x,9)
#define DL_REPEAT_IMPL_N_11(x, d)	DL_REPEAT_IMPL_N_10(x, d)d()  DL_CAT(x,10)
#define DL_REPEAT_IMPL_N_12(x, d)	DL_REPEAT_IMPL_N_11(x, d)d()  DL_CAT(x,11)
#define DL_REPEAT_IMPL_N_13(x, d)	DL_REPEAT_IMPL_N_12(x, d)d()  DL_CAT(x,12)
#define DL_REPEAT_IMPL_N_14(x, d)	DL_REPEAT_IMPL_N_13(x, d)d()  DL_CAT(x,13)
#define DL_REPEAT_IMPL_N_15(x, d)	DL_REPEAT_IMPL_N_14(x, d)d()  DL_CAT(x,14)
#define DL_REPEAT_IMPL_N_16(x, d)	DL_REPEAT_IMPL_N_15(x, d)d()  DL_CAT(x,15)

#define DL_REPEAT_IMPL_N(n, x, d) DL_CAT(DL_REPEAT_IMPL_N_,n)(x, d)

#define DL_REPEAT_N(n,x) DL_REPEAT_IMPL_N(n, x, DL_COMMA)
#define DL_REPEAT_N_TRAILING(n,x) DL_COMMA_IF(n) DL_REPEAT_IMPL_N(n, x, DL_COMMA)

// DL_REPEAT_PARAM_N
#define DL_REPEAT_PARAM_IMPL_N0(n, m, d1, d2)
#define DL_REPEAT_PARAM_IMPL_N1(n, m, d1, d2)	DL_CAT(n,0)d2() DL_CAT(m,0)
#define DL_REPEAT_PARAM_IMPL_N2(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N1(n, m, d1, d2)d1() DL_CAT(n,1)d2() DL_CAT(m,1)
#define DL_REPEAT_PARAM_IMPL_N3(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N2(n, m, d1, d2)d1() DL_CAT(n,2)d2() DL_CAT(m,2)
#define DL_REPEAT_PARAM_IMPL_N4(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N3(n, m, d1, d2)d1() DL_CAT(n,3)d2() DL_CAT(m,3)
#define DL_REPEAT_PARAM_IMPL_N5(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N4(n, m, d1, d2)d1() DL_CAT(n,4)d2() DL_CAT(m,4)
#define DL_REPEAT_PARAM_IMPL_N6(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N5(n, m, d1, d2)d1() DL_CAT(n,5)d2() DL_CAT(m,5)
#define DL_REPEAT_PARAM_IMPL_N7(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N6(n, m, d1, d2)d1() DL_CAT(n,6)d2() DL_CAT(m,6)
#define DL_REPEAT_PARAM_IMPL_N8(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N7(n, m, d1, d2)d1() DL_CAT(n,7)d2() DL_CAT(m,7)
#define DL_REPEAT_PARAM_IMPL_N9(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N8(n, m, d1, d2)d1() DL_CAT(n,8)d2() DL_CAT(m,8)
#define DL_REPEAT_PARAM_IMPL_N10(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N9(n, m, d1, d2)d1() DL_CAT(n,9)d2() DL_CAT(m,9)
#define DL_REPEAT_PARAM_IMPL_N11(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N10(n, m, d1, d2)d1() DL_CAT(n,10)d2() DL_CAT(m,10)
#define DL_REPEAT_PARAM_IMPL_N12(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N11(n, m, d1, d2)d1() DL_CAT(n,11)d2() DL_CAT(m,11)
#define DL_REPEAT_PARAM_IMPL_N13(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N12(n, m, d1, d2)d1() DL_CAT(n,12)d2() DL_CAT(m,12)
#define DL_REPEAT_PARAM_IMPL_N14(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N13(n, m, d1, d2)d1() DL_CAT(n,13)d2() DL_CAT(m,13)
#define DL_REPEAT_PARAM_IMPL_N15(n, m, d1, d2)	DL_REPEAT_PARAM_IMPL_N14(n, m, d1, d2)d1() DL_CAT(n,14)d2() DL_CAT(m,14)
#define DL_REPEAT_PARAM_IMPL_N16(n,m, d1, d2)	DL_REPEAT_PARAM_IMPL_N15(n, m, d1, d2)d1() DL_CAT(n,15)d2() DL_CAT(m,15)

#define DL_REPEAT_PARAM_IMPL_N(c, n, m, d1, d2)	DL_CAT(DL_REPEAT_PARAM_IMPL_N,c)(n, m, d1, d2)

#define DL_REPEAT_PARAM_N(c, n, m)	DL_REPEAT_PARAM_IMPL_N(c, n, m, DL_COMMA, DL_EMPTY)

#define DL_REPEAT_PARAM_N_TRAILING(c, n, m)	DL_COMMA_IF(c) DL_REPEAT_PARAM_IMPL_N(c, n, m, DL_COMMA, DL_EMPTY)



// DL_SEQ_SIZE

#define DL_SEQ_SIZE(seq)	DL_SEQ_SIZE_IMPL(seq)
#define DL_SEQ_SIZE_IMPL(seq)	DL_CAT(DL_N_, DL_SEQ_SIZE_0 seq)

#define DL_SEQ_SIZE_0(_)	DL_SEQ_SIZE_1
#define DL_SEQ_SIZE_1(_)	DL_SEQ_SIZE_2
#define DL_SEQ_SIZE_2(_)	DL_SEQ_SIZE_3
#define DL_SEQ_SIZE_3(_)	DL_SEQ_SIZE_4
#define DL_SEQ_SIZE_4(_)	DL_SEQ_SIZE_5
#define DL_SEQ_SIZE_5(_)	DL_SEQ_SIZE_6
#define DL_SEQ_SIZE_6(_)	DL_SEQ_SIZE_7
#define DL_SEQ_SIZE_7(_)	DL_SEQ_SIZE_8
#define DL_SEQ_SIZE_8(_)	DL_SEQ_SIZE_9
#define DL_SEQ_SIZE_9(_)	DL_SEQ_SIZE_10
#define DL_SEQ_SIZE_10(_)	DL_SEQ_SIZE_11
#define DL_SEQ_SIZE_11(_)	DL_SEQ_SIZE_12
#define DL_SEQ_SIZE_12(_)	DL_SEQ_SIZE_13
#define DL_SEQ_SIZE_13(_)	DL_SEQ_SIZE_14
#define DL_SEQ_SIZE_14(_)	DL_SEQ_SIZE_15
#define DL_SEQ_SIZE_15(_)	DL_SEQ_SIZE_16

#define DL_N_DL_SEQ_SIZE_0	0
#define DL_N_DL_SEQ_SIZE_1	1
#define DL_N_DL_SEQ_SIZE_2	2
#define DL_N_DL_SEQ_SIZE_3	3
#define DL_N_DL_SEQ_SIZE_4	4
#define DL_N_DL_SEQ_SIZE_5	5
#define DL_N_DL_SEQ_SIZE_6	6
#define DL_N_DL_SEQ_SIZE_7	7
#define DL_N_DL_SEQ_SIZE_8	8
#define DL_N_DL_SEQ_SIZE_9	9
#define DL_N_DL_SEQ_SIZE_10	10
#define DL_N_DL_SEQ_SIZE_11	11
#define DL_N_DL_SEQ_SIZE_12	12
#define DL_N_DL_SEQ_SIZE_13	13
#define DL_N_DL_SEQ_SIZE_14	14
#define DL_N_DL_SEQ_SIZE_15	15
#define DL_N_DL_SEQ_SIZE_16	16

// DL_SEQ_ENUM

#define DL_SEQ_ENUM(seq)	DL_SEQ_ENUM_IMPL(seq)
#define DL_SEQ_ENUM_IMPL(seq)	DL_CAT(DL_SEQ_ENUM_, DL_SEQ_SIZE(seq)) seq

#define	DL_SEQ_ENUM_1(x)	x
#define	DL_SEQ_ENUM_2(x)	x, DL_SEQ_ENUM_1
#define	DL_SEQ_ENUM_3(x)	x, DL_SEQ_ENUM_2
#define	DL_SEQ_ENUM_4(x)	x, DL_SEQ_ENUM_3
#define	DL_SEQ_ENUM_5(x)	x, DL_SEQ_ENUM_4
#define	DL_SEQ_ENUM_6(x)	x, DL_SEQ_ENUM_5
#define	DL_SEQ_ENUM_7(x)	x, DL_SEQ_ENUM_6
#define	DL_SEQ_ENUM_8(x)	x, DL_SEQ_ENUM_7
#define	DL_SEQ_ENUM_9(x)	x, DL_SEQ_ENUM_8
#define	DL_SEQ_ENUM_10(x)	x, DL_SEQ_ENUM_9
#define	DL_SEQ_ENUM_11(x)	x, DL_SEQ_ENUM_10
#define	DL_SEQ_ENUM_12(x)	x, DL_SEQ_ENUM_11
#define	DL_SEQ_ENUM_13(x)	x, DL_SEQ_ENUM_12
#define	DL_SEQ_ENUM_14(x)	x, DL_SEQ_ENUM_13
#define	DL_SEQ_ENUM_15(x)	x, DL_SEQ_ENUM_14
#define	DL_SEQ_ENUM_16(x)	x, DL_SEQ_ENUM_15

#else

#include <boost/preprocessor/cat.hpp> 
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>

#define DL_COMMA BOOST_PP_COMMA
#define DL_EMPTY BOOST_PP_EMPTY

#define DL_CAT BOOST_PP_CAT
#define DL_STRINGIZE BOOST_PP_STRINGIZE

#define DL_SEQ_ENUM BOOST_PP_SEQ_ENUM
#define DL_SEQ_SIZE BOOST_PP_SEQ_SIZE

#define DL_REPEAT_N BOOST_PP_ENUM_PARAMS
#define DL_REPEAT_N_TRAILING BOOST_PP_ENUM_TRAILING_PARAMS

#define DL_REPEAT_PARAM_N BOOST_PP_ENUM_BINARY_PARAMS
#define DL_REPEAT_PARAM_N_TRAILING BOOST_PP_ENUM_TRAILING_BINARY_PARAMS

#endif

//  delay load names


namespace delayload
{


template <bool>
struct SelectTypeTraits
{
	template <class T, class U>
	struct Type
	{
		typedef T type;
	};
};

template <>
struct SelectTypeTraits<false>
{
	template <class T, class U>
	struct Type
	{
		typedef U type;
	};
};

template<bool flag, class T, class U>
struct SelectType
{
	typedef typename SelectTypeTraits<flag>::template Type<T, U>::type value;
};


template <class T>
struct Type2Type
{
	typedef T type;
};

template <class T, class U>
struct IsEqualType
{
private:
	typedef char yes;
	typedef char no[2];
	static yes &check(Type2Type<T>);
	static no &check(...);
public:
	enum {value = sizeof(check(Type2Type<U>())) == sizeof(yes)};
};



#define DL_NAME_ID(id)	DL_CAT(CNameId, id)

#define DL_DECLARE_NAME_ID_IMPL(id, name, ret, text)\
struct DL_NAME_ID(id)\
{\
	enum {length = sizeof(name)};\
	static ret GetStr(){return text(name);}\
};


#define DL_DECLARE_NAME_ID_A(id, name)	DL_DECLARE_NAME_ID_IMPL(id, name, LPCSTR, DL_EMPTY())
#define DL_DECLARE_NAME_ID(id, name)	DL_DECLARE_NAME_ID_IMPL(id, name, LPCTSTR,_T)


#ifdef DL_MT
//  MT only
struct CLWMutex
{
    CLWMutex(volatile LONG &pFlag):m_pFlag(pFlag)
    {
    }
    CLWMutex(const CLWMutex &lwSource): m_pFlag(lwSource.m_pFlag)
	{
	}
    CLWMutex & operator=(const CLWMutex &lwSource)
	{
		m_pFlag = lwSource.m_pFlag;
		return *this;
	}
	void Lock()
	{
        while(::InterlockedExchange(&m_pFlag, TRUE))
            ::Sleep(1);
	}
	void Unlock()
	{
        ::InterlockedExchange(&m_pFlag, FALSE);
	}
private:
    volatile LONG &m_pFlag;
};

template<class T>
struct CAutoLock
{
	CAutoLock(T& obj):m_objLock(obj)
	{
		m_objLock.Lock();
	}
	~CAutoLock()
	{
		m_objLock.Unlock();
	}
private:
    CAutoLock(const CAutoLock &);
    CAutoLock & operator=(const CAutoLock &);
private:
	T &m_objLock;
};

#endif //DL_MT


struct CModuleLoadLibraryPolicy
{
	static HMODULE Load(LPCTSTR szFileName)
	{
		return ::LoadLibrary(szFileName);
	}
	static BOOL Free(HMODULE hModule)
	{
		return ::FreeLibrary(hModule);
	}
};

struct CModuleGetModuleHandlePolicy
{
	static HMODULE Load(LPCTSTR szFileName)
	{
		return ::GetModuleHandle(szFileName);
	}
	static BOOL Free(HMODULE /*hModule*/)
	{
		return TRUE;
	}
};

template <class Name, class LoadPolicy = CModuleLoadLibraryPolicy>
class CModule
{
public:
	typedef CModule<Name, LoadPolicy>	type;
	typedef Name						name_type;
	static type &GetModule()
	{
#ifdef DL_MT
		static volatile LONG lMutex = FALSE;
		CLWMutex theMutex(lMutex);
		CAutoLock<CLWMutex> autoLock(theMutex);
#endif //DL_MT
		static type Module;
		return Module;
	}
	HMODULE GetModuleHandle() const
	{
		return m_hModule;
	}
	BOOL IsLoaded() const
	{
		return m_hModule != NULL;
	}
//  Caution - use with care. Not thread-safe
	BOOL UnloadModule()
	{
		HMODULE hModule = m_hModule;
		m_hModule = NULL;
		return LoadPolicy::Free(hModule);
	}
	~CModule()
	{
		if (m_hModule)
			UnloadModule();
	}
private:
	CModule()
	{
		m_hModule = LoadPolicy::Load(name_type::GetStr());
	}
	HMODULE m_hModule;
};


// try to minimize proxy function size

static BOOL inline DL_GetProcAddressImpl(
#ifdef DL_MT
							volatile LONG &pMutex,
							const FARPROC pProxyDef,
#endif // DL_MT
							volatile FARPROC &pProxy,
							HMODULE hModule, 
							LPCTSTR lpProcName
						   )
{
#ifdef DL_MT
		CLWMutex theMutex(pMutex);
		CAutoLock<CLWMutex> autoLock(theMutex);
	//  test for first entry
		if (pProxy != pProxyDef)
			return TRUE;
#endif // DL_MT
		FARPROC pFunction = ::GetProcAddress(hModule, lpProcName);
		if (pFunction)
		{
			pProxy = pFunction;
			return TRUE;
		}
		return FALSE;
}

template <class Module, class Name, class Proxy>
class CDynFunction
{
public:
	typedef CDynFunction<Module, Name, Proxy> type;
	typedef Proxy							  proxy_type;
	typedef Module							  module_type;
	typedef Name							  name_type;
	
	static typename proxy_type::fun_type &GetProxy()
	{
		static typename proxy_type::fun_type proxy = proxy_type::template Proxy<type>::ProxyFun;
		return proxy;
	}
	static BOOL InitFunction()
	{
#ifdef DL_MT
		static volatile LONG lMutex = FALSE;
#endif // DL_MT
		const module_type &theModule = module_type::GetModule();
		if (theModule.IsLoaded())
			return DL_GetProcAddressImpl(
#ifdef DL_MT
											lMutex,
											(const FARPROC)proxy_type::template Proxy<type>::ProxyFun,
#endif //DL_MT
											(volatile FARPROC &)GetProxy(),
											theModule.GetModuleHandle(),
											name_type::GetStr()
										);
		return FALSE;
	}
};

struct CDynFunException
{
	CDynFunException(): m_sMessage(NULL)
	{
	};
	~CDynFunException()
	{
		free(m_sMessage);
	};
	CDynFunException(LPCTSTR sMessage):m_sMessage(NULL)
	{
		SetMessage(sMessage);
	}
	CDynFunException(const CDynFunException &other):m_sMessage(NULL)
	{
		SetMessage(other.m_sMessage);
	}
	CDynFunException &operator = (const CDynFunException &other)
	{
		SetMessage(other.m_sMessage);
		return *this;
	}
	void SetMessage(LPCTSTR sMessage)
	{
		free(m_sMessage);
		m_sMessage = (LPTSTR)malloc((_tcslen(sMessage) + 1) * sizeof(TCHAR));
		if (m_sMessage)
			_tcscpy(m_sMessage, sMessage);
	}
	LPCTSTR GetMessage() const
	{
		return m_sMessage;
	}
private:
	LPTSTR m_sMessage;
};

template<class E = CDynFunException>
struct CFunProxyThrowPolicy
{
	template <class DynFunction> 
	struct FunctionTrait
	{
	//  we don't care about return value - anywhere it could not be used
		typedef typename DynFunction::proxy_type::ret_type raw_ret_type;
		typedef typename SelectType<IsEqualType<raw_ret_type, void>::value, int, raw_ret_type>::value ret_type;
		static  ret_type MakeReturn()
		{
			TCHAR szMessage[DynFunction::name_type::length + 64];
			_stprintf(szMessage, _T("Can't resolve procedure <%s>: %d"), DynFunction::name_type::GetStr(), GetLastError());
			throw E(szMessage);
		//	return ret_type();
		}
	};
};


//  we need not to implement void return type value policy, 
//  coz void function can only throw on error

template<class R, R value = R()>
struct CFunProxyValuePolicy
{
	template <class DynFunction> 
	struct FunctionTrait
	{
		static typename DynFunction::proxy_type::ret_type MakeReturn()
		{
			return value;
		}
	};
};


#define DL_FUN_PROXY(n) DL_CAT(CFunProxy,n)
#define DL_FUN_PROXY_IMPL(n) DL_CAT(DL_FUN_PROXY(n),Impl)
#define DL_FUN_PROXY_IMPL1(n) DL_CAT(DL_FUN_PROXY(n),Impl1)

#define DL_DECLARE_FUN_PROXY_IMPL(param_count) \
template <typename R>\
struct DL_FUN_PROXY_IMPL(param_count)\
{\
	template <class DynFunction DL_REPEAT_N_TRAILING(param_count, typename P), class Policy> struct RetProxy\
	{\
		static R WINAPI ProxyFun(DL_REPEAT_PARAM_N(param_count, P, v))\
		{\
			if (DynFunction::InitFunction())\
				return DynFunction::GetProxy()(DL_REPEAT_N(param_count, v));\
			return Policy::template FunctionTrait<DynFunction>::MakeReturn();\
		}\
	};\
};\
\
template <>\
struct DL_FUN_PROXY_IMPL(param_count) <void>\
{\
	template <class DynFunction DL_REPEAT_N_TRAILING(param_count, typename P), class Policy> struct RetProxy\
	{\
		static void WINAPI ProxyFun(DL_REPEAT_PARAM_N(param_count, P, v))\
		{\
			if (DynFunction::InitFunction())\
				DynFunction::GetProxy()(DL_REPEAT_N(param_count, v));\
			else\
				Policy::template FunctionTrait<DynFunction>::MakeReturn();\
		}\
	};\
};

//  instantiate ProxyFunImpl for zero parameters count

DL_DECLARE_FUN_PROXY_IMPL(0)

// VC6 bug - nested class inheritance from class instantiated with selecttype causes compiler error

#define DL_DECLARE_FUN_PROXY_IMPL1(param_count) \
DL_DECLARE_FUN_PROXY_IMPL(param_count) \
template <class R DL_REPEAT_N_TRAILING(param_count, typename P), class DynFunction, class Policy>\
struct DL_FUN_PROXY_IMPL1(param_count):public \
	SelectType<\
				IsEqualType<P0, void>::value,\
				typename DL_FUN_PROXY_IMPL(0)<R>::template RetProxy<DynFunction, Policy>,\
				typename DL_FUN_PROXY_IMPL(param_count)<R>::template RetProxy<DynFunction DL_REPEAT_N_TRAILING(param_count, P), Policy> \
			  >::value\
{\
};

#define DL_DECLARE_FUN_PROXY(param_count) \
DL_DECLARE_FUN_PROXY_IMPL1(param_count) \
template <typename R DL_REPEAT_N_TRAILING(param_count, typename P), class Policy = CFunProxyValuePolicy<R> >\
struct DL_FUN_PROXY(param_count)\
{\
	typedef R (WINAPI *fun_type)(DL_REPEAT_N(param_count, P));\
	typedef R ret_type;\
	template <class DynFunction> struct Proxy:public DL_FUN_PROXY_IMPL1(param_count)<R DL_REPEAT_N_TRAILING(param_count, P), DynFunction, Policy>\
	{\
	};\
};

DL_DECLARE_FUN_PROXY(1)
DL_DECLARE_FUN_PROXY(2)
DL_DECLARE_FUN_PROXY(3)
DL_DECLARE_FUN_PROXY(4)
DL_DECLARE_FUN_PROXY(5)
DL_DECLARE_FUN_PROXY(6)
DL_DECLARE_FUN_PROXY(7)
DL_DECLARE_FUN_PROXY(8)
DL_DECLARE_FUN_PROXY(9)
DL_DECLARE_FUN_PROXY(10)
DL_DECLARE_FUN_PROXY(11)
DL_DECLARE_FUN_PROXY(12)
DL_DECLARE_FUN_PROXY(13)
DL_DECLARE_FUN_PROXY(14)
DL_DECLARE_FUN_PROXY(15)
DL_DECLARE_FUN_PROXY(16)

}

// usefull macro's


//  module definitions

#define DL_USE_MODULE_LOAD_POLICY_BEGIN(nmspace, name, load_policy) \
namespace nmspace \
{\
	DL_DECLARE_NAME_ID(DL_CAT(_MODULE_, nmspace), name)\
	typedef delayload::CModule<DL_NAME_ID(DL_CAT(_MODULE_, nmspace)), load_policy> module_type;

#define DL_USE_MODULE_BEGIN(nmspace, name) DL_USE_MODULE_LOAD_POLICY_BEGIN(nmspace, name, delayload::CModuleLoadLibraryPolicy)
#define DL_USE_MODULE_NON_LOAD_BEGIN(nmspace, name) DL_USE_MODULE_LOAD_POLICY_BEGIN(nmspace, name, delayload::CModuleGetModuleHandlePolicy)

#define DL_USE_MODULE_END() \
};


//  function definitions

#define DL_DECLARE_FUN_ERR_POLICY(name_id, r, p, pl) \
DL_DECLARE_NAME_ID_A(name_id, DL_STRINGIZE(name_id))\
static r (WINAPI *&name_id)(DL_SEQ_ENUM(p)) = delayload::CDynFunction<module_type, DL_NAME_ID(name_id), delayload::DL_FUN_PROXY(DL_SEQ_SIZE(p))<r, DL_SEQ_ENUM(p), pl > >::GetProxy();

#define DL_DECLARE_FUN(name_id, r, p) DL_DECLARE_FUN_ERR_POLICY(name_id, r, p, delayload::CFunProxyValuePolicy<r>)
#define DL_DECLARE_FUN_THROW(name_id, r, p) DL_DECLARE_FUN_ERR_POLICY(name_id, r, p, delayload::CFunProxyThrowPolicy<>)

#define DL_DECLARE_FUN_ERR(name_id, r, p, e)\
DL_DECLARE_FUN_ERR_POLICY(name_id, r, p, delayload::CFunProxyValuePolicy<r DL_COMMA() e>)

#endif
