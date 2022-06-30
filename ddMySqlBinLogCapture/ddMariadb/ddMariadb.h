//////////////////////////////////////////////////////////////////////////////////
//																			
//	FILE				:ddMariadb.h
//																			
//	PURPOSE				:Defines the exported functions for the DLL application.
//																			
//	MANUFACTURER		:Gamma Soft
//
//	DATE				:29/12/2011
//
//////////////////////////////////////////////////////////////////////////////////
//
//	SAMPLE USAGE		:
//
//	void* hDatabase = NULL;
//	void* hQuery = NULL;
//	MYSQL_ROW hRow = NULL;
//	char* hMsg = NULL;
//	char* pszResult = NULL;
//	char* hSocket = NULL;
//	char *hSqlState = NULL;
//	unsigned int nError;
//	long dwResult;
//	char *pszQuery = NULL;
//
//	dwResult = MySetVerbosity(1);
//	dwResult = MyInitDB(&hDatabase);
//	dwResult = MyOpenConnection(hDatabase, "WFRDEV02", "root", "gamma", 3306, true, "TCP", &hSocket);
//	if (dwResult  == mySUCCESS)
//	{
//		pszQuery = "select * from test.categ";
//		dwResult = MyExecStatement(hDatabase, pszQuery);
//		if (dwResult != mySUCCESS)
//		{
//			dwResult = MyGetErrormessage(hDatabase, &hMsg);
//			dwResult = MyGetErrorNum(hDatabase, &nError);
//			dwResult =	MyGetSqlState(hDatabase, &hSqlState);
//		}
//		dwResult = MyGetResult(hDatabase, &hQuery);
//		if (dwResult == mySUCCESS)
//		{
//			long dwRowCount = MyGetResultCount(hQuery);
//			long dwFieldCount = MyGetNumField(hQuery);
//
//			dwResult = MyGetFetchRow(hQuery, &hRow);
//			if (dwResult == mySUCCESS)
//			{
//				dwResult = MyGetResultValue(hRow, 0, &pszResult);
//				dwResult = MyGetResultValue(hRow, 1, &pszResult);
//			}
//		}
//		else
//		{
//			dwResult = MyGetErrormessage(hDatabase, &hMsg);
//			dwResult = MyGetErrorNum(hDatabase, &nError);
//			dwResult =	MyGetSqlState(hDatabase, &hSqlState);
//		}
//		if (hQuery)
//		{
//			dwResult = MyFreeStatement(hQuery);
//		}
//		dwResult =  MyCloseConnection(hDatabase);
//	}
//	dwResult = MyFreeDB();
//
//////////////////////////////////////////////////////////////////////////////////

#ifdef DDMYSQL_EXPORTS
#define DDMYSQL_API extern "C" __declspec(dllexport)  
#else
#define DDMYSQL_API extern "C" __declspec(dllimport) 
#endif

//#define _STORE_RESULT
#define _USE_RESULT

#if (_MSC_VER && (_MSC_VER <= 1200 || (_MSC_VER >= 1900 && !DDMYSQL_EXPORTS)  ) )

typedef char **MYSQL_ROW;		/* return data as array of strings */

enum enum_field_types {
	MYSQL_TYPE_DECIMAL,
	MYSQL_TYPE_TINY,
	MYSQL_TYPE_SHORT,
	MYSQL_TYPE_LONG,
	MYSQL_TYPE_FLOAT,
	MYSQL_TYPE_DOUBLE,
	MYSQL_TYPE_NULL,
	MYSQL_TYPE_TIMESTAMP,
	MYSQL_TYPE_LONGLONG,
	MYSQL_TYPE_INT24,
	MYSQL_TYPE_DATE,
	MYSQL_TYPE_TIME,
	MYSQL_TYPE_DATETIME,
	MYSQL_TYPE_YEAR,
	MYSQL_TYPE_NEWDATE,
	MYSQL_TYPE_VARCHAR,
	MYSQL_TYPE_BIT,
	MYSQL_TYPE_NEWDECIMAL = 246,
	MYSQL_TYPE_ENUM = 247,
	MYSQL_TYPE_SET = 248,
	MYSQL_TYPE_TINY_BLOB = 249,
	MYSQL_TYPE_MEDIUM_BLOB = 250,
	MYSQL_TYPE_LONG_BLOB = 251,
	MYSQL_TYPE_BLOB = 252,
	MYSQL_TYPE_VAR_STRING = 253,
	MYSQL_TYPE_STRING = 254,
	MYSQL_TYPE_GEOMETRY = 255
};

typedef struct st_mysql_field {
	char *name;                 /* Name of column */
	char *org_name;             /* Original column name, if an alias */
	char *table;                /* Table of column if column was a field */
	char *org_table;            /* Org table name, if table was an alias */
	char *db;                   /* Database for table */
	char *catalog;			  /* Catalog for table */
	char *def;                  /* Default value (set by mysql_list_fields) */
	unsigned long length;       /* Width of column (create length) */
	unsigned long max_length;   /* Max width for selected set */
	unsigned int name_length;
	unsigned int org_name_length;
	unsigned int table_length;
	unsigned int org_table_length;
	unsigned int db_length;
	unsigned int catalog_length;
	unsigned int def_length;
	unsigned int flags;         /* Div flags */
	unsigned int decimals;      /* Number of decimals in field */
	unsigned int charsetnr;     /* Character set */
	enum enum_field_types type; /* Type of field. See mysql_com.h for types */
	void *extension;
} MYSQL_FIELD;

#else

#include <mysql.h>

#endif

#if (_MSC_VER && (_MSC_VER <= 1200 || (_MSC_VER >= 1900 && !DDMYSQL_EXPORTS)  ) )
enum mysql_option
{
	MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_COMPRESS, MYSQL_OPT_NAMED_PIPE,
	MYSQL_INIT_COMMAND, MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP,
	MYSQL_SET_CHARSET_DIR, MYSQL_SET_CHARSET_NAME, MYSQL_OPT_LOCAL_INFILE,
	MYSQL_OPT_PROTOCOL, MYSQL_SHARED_MEMORY_BASE_NAME, MYSQL_OPT_READ_TIMEOUT,
	MYSQL_OPT_WRITE_TIMEOUT, MYSQL_OPT_USE_RESULT,
	MYSQL_OPT_USE_REMOTE_CONNECTION, MYSQL_OPT_USE_EMBEDDED_CONNECTION,
	MYSQL_OPT_GUESS_CONNECTION, MYSQL_SET_CLIENT_IP, MYSQL_SECURE_AUTH,
	MYSQL_REPORT_DATA_TRUNCATION, MYSQL_OPT_RECONNECT,
	MYSQL_OPT_SSL_VERIFY_SERVER_CERT, MYSQL_PLUGIN_DIR, MYSQL_DEFAULT_AUTH
};

#endif

const long mySUCCESS = 0;
const long myERROR_HANDLE_CONNECTION = -1;
const long myERROR_HANDLE_QUERY = -2;
const long myERROR_HANDLE_MSG = -3;
const long myERROR_HANDLE_RESULT = -4;
const long myERROR_HANDLE_INIT = -5;
const long myERROR_HANDLE_ROW = -6;
const long myERROR_PROTOCOL = -7;
const long myERROR_NO_DATA = -8;
const long myERROR_PARAM = -9;

// Function name
#define	NAME_MySetVerbosity					"_MySetVerbosity@4"
#define	NAME_MyInitDB						"_MyInitDB@4"
#define	NAME_MyFreeDB						"_MyFreeDB@0"	
#define	NAME_MySetOptionDB					"_MySetOptionDB@12"	
#define	NAME_MyOpenConnection				"_MyOpenConnection@32"
#define	NAME_MyCloseConnection				"_MyCloseConnection@4"
#define	NAME_MyGetErrormessage				"_MyGetErrormessage@8"
#define	NAME_MyGetErrorNum					"_MyGetErrorNum@8"	
#define	NAME_MyGetSqlState					"_MyGetSqlState@8"	
#define	NAME_MyFreeStatement				"_MyFreeStatement@4"
#define	NAME_MyExecStatement				"_MyExecStatement@8"
#define	NAME_MyGetResult					"_MyGetResult@8"	
#define	NAME_MyGetResultValue				"_MyGetResultValue@12"	
#define	NAME_MyGetFetchRow					"_MyGetFetchRow@8"		
#define	NAME_MyGetAffectedRows				"_MyGetAffectedRows@4"
#define	NAME_MyGetInfo						"_MyGetInfo@8"
#define	NAME_MyGetResultCount				"_MyGetResultCount@4"
#define	NAME_MyGetNumField					"_MyGetNumField@4"
#define	NAME_MyGetResultLength				"_MyGetResultLength@8"
#define	NAME_MyGetResultField				"_MyGetResultField@8"

// Function prototype
typedef long(__stdcall* _MySetVerbosity)(const int verbose);
typedef long(__stdcall* _MyInitDB)(void **handle);
typedef long(__stdcall* _MyFreeDB)();
typedef long(__stdcall* _MySetOptionDB)(void *handle, mysql_option option, const void *pArg);
typedef long(__stdcall* _MyOpenConnection)(void *handle, const char *pszHost, const char *pszUser, const char *pszPassword, int iPort, bool fAutoReconnect, char *pszProtocol, char ** socket);
typedef long(__stdcall* _MyCloseConnection)(void *handle);
typedef long(__stdcall* _MyGetErrormessage)(void *handle, char **hMsg);
typedef long(__stdcall* _MyGetErrorNum)(void *handle, unsigned int *hMsg);
typedef long(__stdcall* _MyGetSqlState)(void *handle, char **hMsg);
typedef long(__stdcall* _MyFreeStatement)(void *hQuery);
typedef long(__stdcall* _MyExecStatement)(void *handle, const char *query);
typedef long(__stdcall* _MyGetResult)(void *handle, void **hQuery);
typedef long(__stdcall* _MyGetResultValue)(MYSQL_ROW hRow, int nColumn, char **pszResult);
typedef long(__stdcall* _MyGetFetchRow)(void *hQuery, MYSQL_ROW *hRow);
typedef long(__stdcall* _MyGetAffectedRows)(void *handle);
typedef long(__stdcall* _MyGetInfo)(void *handle, char **hMsg);
typedef long(__stdcall* _MyGetResultCount)(void *hQuery);
typedef long(__stdcall* _MyGetNumField)(void *hQuery);
typedef long(__stdcall* _MyGetResultLength)(void *hQuery, unsigned long **hLength);
typedef long(__stdcall* _MyGetResultField)(void *hQuery, MYSQL_FIELD **hField);

extern _MySetVerbosity			hMySetVerbosity;
extern _MyInitDB				hMyInitDB;
extern _MyFreeDB				hMyFreeDB;
extern _MySetOptionDB			hMySetOptionDB;
extern _MyOpenConnection		hMyOpenConnection;
extern _MyCloseConnection		hMyCloseConnection;
extern _MyGetErrormessage		hMyGetErrormessage;
extern _MyGetErrorNum			hMyGetErrorNum;
extern _MyGetSqlState			hMyGetSqlState;
extern _MyFreeStatement			hMyFreeStatement;
extern _MyExecStatement			hMyExecStatement;
extern _MyGetResult				hMyGetResult;
extern _MyGetResultValue		hMyGetResultValue;
extern _MyGetFetchRow			hMyGetFetchRow;
extern _MyGetAffectedRows		hMyGetAffectedRows;
extern _MyGetInfo				hMyGetInfo;
extern _MyGetResultCount		hMyGetResultCount;
extern _MyGetNumField			hMyGetNumField;
extern _MyGetResultLength		hMyGetResultLength;
extern _MyGetResultField		hMyGetResultField;


DDMYSQL_API long PASCAL MySetVerbosity(const int verbose);
DDMYSQL_API long PASCAL MyInitDB(void **handle);
DDMYSQL_API long PASCAL MyFreeDB();
DDMYSQL_API long PASCAL MySetOptionDB(void *handle, mysql_option option, const void *pArg);
DDMYSQL_API long PASCAL MyOpenConnection(void *handle, const char *pszHost, const char *pszUser, const char *pszPassword, int iPort, bool fAutoReconnect, char *pszProtocol, char ** socket);
DDMYSQL_API long PASCAL MyCloseConnection(void *handle);
DDMYSQL_API long PASCAL MyGetErrormessage(void *handle, char **hMsg);
DDMYSQL_API long PASCAL MyGetErrorNum(void *handle, unsigned int *hMsg);
DDMYSQL_API long PASCAL MyGetSqlState(void *handle, char **hMsg);
DDMYSQL_API long PASCAL MyFreeStatement(void *hQuery);
DDMYSQL_API long PASCAL MyExecStatement(void *handle, const char *query);
DDMYSQL_API long PASCAL MyGetResult(void *handle, void **hQuery);
DDMYSQL_API long PASCAL MyGetResultValue(MYSQL_ROW hRow, int nColumn, char **pszResult);
DDMYSQL_API long PASCAL MyGetFetchRow(void *hQuery, MYSQL_ROW *hRow);
DDMYSQL_API long PASCAL MyGetAffectedRows(void *handle);
DDMYSQL_API long PASCAL MyGetInfo(void *handle, char **hMsg);
DDMYSQL_API long PASCAL MyGetResultCount(void *hQuery);
DDMYSQL_API long PASCAL MyGetNumField(void *hQuery);
DDMYSQL_API long PASCAL MyGetResultLength(void *hQuery, unsigned long **hLength);
DDMYSQL_API long PASCAL MyGetResultField(void *hQuery, MYSQL_FIELD **hField);
