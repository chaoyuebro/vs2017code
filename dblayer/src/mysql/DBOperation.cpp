/*
 * DBOperation.cpp
 *
 *  Created on: 2018年11月20日
 *      Author: wangpf
 */

#include "mysql/DBOperation.h"
#include "Tables.h"
#include <etp/debug.h>
#include <etp/ETPEvent.h>
#include <etp/TKException.h>
#include <etp/Errors.h>
#include <etp/helper.h>
#include <boost/lexical_cast.hpp>

CDBOperation::CDBOperation()
: m_Port(0), m_ClientFlag(0)
{

}
CDBOperation::~CDBOperation()
{

}

bool CDBOperation::ConnectDB(string host, string user,
		 string passwd, string db,
		 unsigned int port,
		 unsigned long client_flag)
{
    try{
		if(-1 == m_DataBase.Connect(host, user, passwd, db, port, client_flag))
		{
	    	ERROR << "mysql host:" << host << std::endl;
	    	ERROR << "mysql user:" << user << std::endl;
	    	ERROR << "mysql passwd:" << passwd << std::endl;
	    	ERROR << "mysql db:" << db << std::endl;
	    	ERROR << "mysql port:" << port << std::endl;
	    	ERROR << "mysql client_flag:" << client_flag << std::endl;
			THROW(TKException(GetErrorID("COMMON_ERROR_MYSQL_DISCONNECT"), GetErrorMsg("COMMON_ERROR_MYSQL_DISCONNECT")));
		}
		m_Host = host;
		m_User = user;
		m_Passwd = passwd;
		m_DBName = db;
		m_Port = port;
		m_ClientFlag = client_flag;
		return true;
    }
    catch(std::exception & e)
    {
    	ERROR << e.what() << std::endl;
    	return false;
    }
}

void CDBOperation::ConnectDB()
{
	DEBUG_ENTRY();
    try{
		if(-1 == m_DataBase.Connect(m_Host, m_User, m_Passwd, m_DBName, m_Port, 0))
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_MYSQL_DISCONNECT"), GetErrorMsg("COMMON_ERROR_MYSQL_DISCONNECT")));
		}
    }
    catch(std::exception & e)
    {
    	ERROR << e.what() << std::endl;
    	THROW(TKException(GetErrorID("COMMON_ERROR_MYSQL_DISCONNECT"), GetErrorMsg("COMMON_ERROR_MYSQL_DISCONNECT")));
    }
}

void CDBOperation::DisConnectDB()
{
	m_DataBase.DisConnect();
}

/* 主要功能:开始事务 */
int CDBOperation::Start_Transaction()
{
	return  m_DataBase.Start_Transaction();
}
/* 主要功能:提交事务 */
int CDBOperation::Commit()
{
	return m_DataBase.Commit();
}
/* 主要功能:回滚事务 */
int CDBOperation::Rollback()
{
	return m_DataBase.Rollback();
}

void CDBOperation::CreateLoginLogTable()
{
	DEBUG_ENTRY();

	// Check table is exist, create it if not exist
	std::string strSql = (string)"CREATE TABLE IF NOT EXISTS `" + USERLOGINLOG_TABLE +\
				"` ("  \
				"`UserID` varchar(10) NOT NULL,"\
				"`BrokerID` varchar(20),"\
				"`LoginTime` varchar(20),"\
				"`Ip` varchar(20),"\
				"`Port` int,"\
				"`Mac` varchar(20),"\
				"`Comment` varchar(50),"\
				"PRIMARY KEY (`Index`),"\
				"INDEX "\
				"USER_INDEX (UserID(10))"\
			")ENGINE=InnoDB DEFAULT CHARSET=gb2312;";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if( -1 == m_DataBase.ExecQuery(strSql))
	{
		ERROR << "error sql:" << strSql << std::endl;
		THROW(TKException(GetErrorID("ADJUST_ERROR_MYSQL_EXEC"), GetErrorMsg("ADJUST_ERROR_MYSQL_EXEC")));
	}
}

bool CDBOperation::InsertLoginLog_Prepare(LoginLogField loginLog)
{
	try{
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		MYSQL * conn = m_DataBase.GetMysql();
		MYSQL_STMT *stmt = mysql_stmt_init(conn); //创建MYSQL_STMT句柄
		std::string strSql = (string)"INSERT INTO `" + USERLOGINLOG_TABLE + "` VALUES (?, ?, ?, ?, ?, ?, ?);";
	   if(mysql_stmt_prepare(stmt, strSql.c_str(), strSql.length()))
	   {
		   ERROR << "mysql_stmt_prepare: %s\n" << mysql_error(conn) << std::endl;;
		   return false;
	    }
	   MYSQL_BIND params[7];
	   memset(params, 0, sizeof(params));

	   std::string strUserID = UTF8toGB2312(loginLog.UserID);
	   params[0].buffer_type = MYSQL_TYPE_STRING;
	   params[0].buffer = const_cast<char*>(strUserID.c_str());
	   params[0].buffer_length = strUserID.length();

	   std::string strBrokerID = UTF8toGB2312(loginLog.BrokerID);
	   params[1].buffer_type = MYSQL_TYPE_STRING;
	   params[1].buffer = const_cast<char*>(strBrokerID.c_str());
	   params[1].buffer_length = strBrokerID.length();

	   params[2].buffer_type = MYSQL_TYPE_STRING;
	   params[2].buffer = const_cast<char*>(loginLog.LoginTime.c_str());
	   params[2].buffer_length = loginLog.LoginTime.length();

	   params[3].buffer_type = MYSQL_TYPE_STRING;
	   params[3].buffer = const_cast<char*>(loginLog.IP.c_str());
	   params[3].buffer_length = loginLog.IP.length();

	   std::string strPort = boost::lexical_cast<std::string>(loginLog.Port);
	   params[4].buffer_type = MYSQL_TYPE_STRING;
	   params[4].buffer = const_cast<char*>(strPort.c_str());
	   params[4].buffer_length = strPort.length();

	   params[5].buffer_type = MYSQL_TYPE_STRING;
	   params[5].buffer = const_cast<char*>(loginLog.Mac.c_str());
	   params[5].buffer_length = loginLog.Mac.length();

	   std::string strComment = UTF8toGB2312(loginLog.Comment);
	   params[6].buffer_type = MYSQL_TYPE_BLOB;
	   params[6].buffer = const_cast<char*>(strComment.c_str());
	   params[6].buffer_length = strComment.length();

		mysql_stmt_bind_param(stmt, params);
	   mysql_stmt_execute(stmt);           //执行与语句句柄相关的预处理
	   mysql_stmt_close(stmt);

	   return true;
	}
	catch(...)
	{
		ERROR << "Recort login log faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::InsertLoginLog(LoginLogField loginLog)
{
	string strSql = (string)"INSERT INTO `" + USERLOGINLOG_TABLE + "` VALUES ( \"" \
			+ loginLog.UserID + "\", \""
			+ loginLog.BrokerID + "\", \""
			+ loginLog.LoginTime + "\", \""
			+ loginLog.IP + "\", \""
			+ boost::lexical_cast<std::string>(loginLog.Port) + "\", \""
			+ loginLog.Mac + "\", \""
			+ loginLog.Comment
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}

// 账号信息
bool CDBOperation::UpdInvestorInfo(InvestorInfoField &field)
{
	string strSql = (string)"REPLACE INTO `" + INVESTOR_TABLE +"` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(field.ID) + "\", \""
			+ field.InvestorID + "\", \""
			+ field.InvestorName + "\", \""
			+ field.BrokerID + "\", \""
			+ field.BrokerName + "\", \""
			+ field.Password + "\", \""
			+ field.Type + "\", \""
			+ field.OpenDate + "\", \""
			+ field.CurrencyCode + "\", \""
			+ field.Status + "\", \""
			+ boost::lexical_cast<std::string>(field.MarginRateModelID) + "\", \""
			+ field.MarginRateModelComment + "\", \""
			+ boost::lexical_cast<std::string>(field.CommissionRateModelID) + "\", \""
			+ field.CommissionRateModelComment + "\", \""
			+ boost::lexical_cast<std::string>(field.Initial) + "\", \""
			+ field.Comment + "\", \""
			+ boost::lexical_cast<std::string>(field.TradeCounterID)
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	GetInvestorInfo(field, field.BrokerID, field.InvestorID);
	return true;
}
bool CDBOperation::GetInvestorInfo(InvestorInfoField &field, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVESTOR_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_INVESTOR"), GetErrorMsg("COMMON_ERROR_NO_INVESTOR")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.ID = toInt(recordSet[0][0]);
		field.InvestorID = recordSet[0][1];
		field.InvestorName = recordSet[0][2];
		field.BrokerID = recordSet[0][3];
		field.BrokerName = recordSet[0][4];
		field.Password = recordSet[0][5];
		field.Type = recordSet[0][6];
		field.OpenDate = recordSet[0][7];
		field.CurrencyCode = recordSet[0][8];
		field.Status = recordSet[0][9];
		field.MarginRateModelID = toInt(recordSet[0][10]);
		field.MarginRateModelComment = recordSet[0][11];
		field.CommissionRateModelID = toInt(recordSet[0][12]);
		field.CommissionRateModelComment = recordSet[0][13];
		field.Initial = toDouble(recordSet[0][14]);
		field.Comment = recordSet[0][15];
		field.TradeCounterID = toInt(recordSet[0][16]);

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry investor info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetInvestorInfo(InvestorInfoField &field, int id)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVESTOR_TABLE;
		strSql = strSql + "` WHERE ID = '" + boost::lexical_cast<std::string>(id) + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_INVESTOR"), GetErrorMsg("COMMON_ERROR_NO_INVESTOR")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.ID = toInt(recordSet[0][0]);
		field.InvestorID = recordSet[0][1];
		field.InvestorName = recordSet[0][2];
		field.BrokerID = recordSet[0][3];
		field.BrokerName = recordSet[0][4];
		field.Password = recordSet[0][5];
		field.Type = recordSet[0][6];
		field.OpenDate = recordSet[0][7];
		field.CurrencyCode = recordSet[0][8];
		field.Status = recordSet[0][9];
		field.MarginRateModelID = toInt(recordSet[0][10]);
		field.MarginRateModelComment = recordSet[0][11];
		field.CommissionRateModelID = toInt(recordSet[0][12]);
		field.CommissionRateModelComment = recordSet[0][13];
		field.Initial = toDouble(recordSet[0][14]);
		field.Comment = recordSet[0][15];
		field.TradeCounterID = toInt(recordSet[0][16]);

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry investor info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetInvestorInfo(vector<InvestorInfoField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVESTOR_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		InvestorInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.ID = toInt(recordSet[row][0]);
			field.InvestorID = recordSet[row][1];
			field.InvestorName = recordSet[row][2];
			field.BrokerID = recordSet[row][3];
			field.BrokerName = recordSet[row][4];
			field.Password = recordSet[row][5];
			field.Type = recordSet[row][6];
			field.OpenDate = recordSet[row][7];
			field.CurrencyCode = recordSet[row][8];
			field.Status = recordSet[row][9];
			field.MarginRateModelID = toInt(recordSet[row][10]);
			field.MarginRateModelComment = recordSet[row][11];
			field.CommissionRateModelID = toInt(recordSet[row][12]);
			field.CommissionRateModelComment = recordSet[row][13];
			field.Initial = toDouble(recordSet[row][14]);
			field.Comment = recordSet[row][15];
			field.TradeCounterID = toInt(recordSet[row][16]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all investors info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetInvestorInfo(vector<InvestorInfoField> &fieldsVec, string strServerID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVEDTORSERVERPORTCONF_TABLE + "` WHERE id = '" + strServerID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet scRecordSet(m_DataBase.GetMysql());
		int scRecordCount = scRecordSet.ExecuteSQL(strSql.c_str());
		if(scRecordCount == 0)
		{
			ERROR << "No set server port of server[" << strServerID << "]" << std::endl;
			return false;
		}
		if(scRecordCount < 0)
		{
			ERROR << "Qry server port of server[" << strServerID << "] faild!" << std::endl;
			return false;
		}
		string from = scRecordSet[0][1];
		string to = scRecordSet[0][2];

		// get sql string
		strSql = (string)"SELECT * FROM `" + INVESTOR_TABLE + "` WHERE id BETWEEN '" + from + "' AND '" + to + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		InvestorInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.ID = toInt(recordSet[row][0]);
			field.InvestorID = recordSet[row][1];
			field.InvestorName = recordSet[row][2];
			field.BrokerID = recordSet[row][3];
			field.BrokerName = recordSet[row][4];
			field.Password = recordSet[row][5];
			field.Type = recordSet[row][6];
			field.OpenDate = recordSet[row][7];
			field.CurrencyCode = recordSet[row][8];
			field.Status = recordSet[row][9];
			field.MarginRateModelID = toInt(recordSet[row][10]);
			field.MarginRateModelComment = recordSet[row][11];
			field.CommissionRateModelID = toInt(recordSet[row][12]);
			field.CommissionRateModelComment = recordSet[row][13];
			field.Initial = toDouble(recordSet[row][14]);
			field.Comment = recordSet[row][15];
			field.TradeCounterID = toInt(recordSet[0][16]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all investors info faild !" << std::endl;
		return false;
	}
}

bool CDBOperation::DelInvestorInfo(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + INVESTOR_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 交易所
bool CDBOperation::GetExchangeInfo(ExchangeInfoField &field, string strExchangeID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + EXCHANGE_TABLE;
		strSql = strSql + "` WHERE ExchangeID = '" + strExchangeID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.ExchangeID = recordSet[0][0];
		field.ExchangeName = recordSet[0][1];
		field.ExchTypeID = toInt(recordSet[0][2]);
		field.CurrencyCode = recordSet[0][3];
		field.IsMarginFavourable = toInt(recordSet[0][4]);
		field.Comment = recordSet[0][5];

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry exchange " << strExchangeID << "'s info faild !" << std::endl;
		return false;
	}
}

bool CDBOperation::GetExchangeInfo(vector<ExchangeInfoField> &fieldsVec)
{
	try
	{
		fieldsVec.clear();
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + EXCHANGE_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		ExchangeInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.ExchangeID = recordSet[row][0];
			field.ExchangeName = recordSet[row][1];
			field.ExchTypeID = toInt(recordSet[row][2]);
			field.CurrencyCode = recordSet[row][3];
			field.IsMarginFavourable = toInt(recordSet[row][4]);
			field.Comment = recordSet[row][5];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all exchanges info faild !" << std::endl;
		return false;
	}
}

// 经纪公司
bool CDBOperation::GetBrokerInfo(BrokerInfoField &field, string strBrokerID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + BROKER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.BrokerID = recordSet[0][1];
		field.ShortName = recordSet[0][2];
		field.Name = recordSet[0][3];
		field.Type = recordSet[0][4];
		field.Address = recordSet[0][5];
		field.IsActive = toInt(recordSet[0][6]);
		field.Comment = recordSet[0][7];

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry broker " << strBrokerID << "'s info faild !" << std::endl;
		return false;
	}
}

bool CDBOperation::GetBrokerInfo(vector<BrokerInfoField> &fieldsVec)
{
	try
	{
		fieldsVec.clear();
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + BROKER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		BrokerInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][1];
			field.ShortName = recordSet[row][2];
			field.Name = recordSet[row][3];
			field.Type = recordSet[row][4];
			field.Address = recordSet[row][5];
			field.IsActive = toInt(recordSet[row][6]);
			field.Comment = recordSet[row][7];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all brokers info faild !" << std::endl;
		return false;
	}
}

// 品种信息
bool CDBOperation::UpdProductInfo(ProductInfoField &field)
{
	string strSql = (string)"REPLACE INTO `" + PRODUCT_TABLE +"` VALUES ( \"" \
			+ field.ProductID + "\", \""
			+ field.ProductName + "\", \""
			+  boost::lexical_cast<std::string>(field.VolumeMultiple) + "\", \""
			+  boost::lexical_cast<std::string>(field.PriceTick) + "\", \""
			+ field.ExchangeID + "\", \""
			+ field.ExchangeName + "\", \""
			+ field.Type
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetProductInfo(ProductInfoField &field, string strProductID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + PRODUCT_TABLE;
		strSql = strSql + "` WHERE ProductID = '" + strProductID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.ProductID = recordSet[0][0];
		field.ProductName = recordSet[0][1];
		field.VolumeMultiple = toDouble(recordSet[0][2]);
		field.PriceTick = toDouble(recordSet[0][3]);
		field.ExchangeID = recordSet[0][4];
		field.ExchangeName = recordSet[0][5];
		field.Type = recordSet[0][6];

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry product " << strProductID << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetProductInfo(vector<ProductInfoField> &fieldsVec)
{
	try
	{
		fieldsVec.clear();
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + PRODUCT_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		ProductInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.ProductID = recordSet[row][0];
			field.ProductName = recordSet[row][1];
			field.VolumeMultiple = toDouble(recordSet[row][2]);
			field.PriceTick = toDouble(recordSet[row][3]);
			field.ExchangeID = recordSet[row][4];
			field.ExchangeName = recordSet[row][5];
			field.Type = recordSet[row][6];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all products info faild !" << std::endl;
		return false;
	}
}

// 合约信息
bool CDBOperation::UpdInstrumentInfo(InstrumentInfoField &inst)
{
	string strSql = (string)"REPLACE INTO `" + INSTRUMENT_TABLE +"` VALUES ( \"" \
			+ inst.InstrumentID + "\", \""
			+ inst.InstrumentName + "\", \""
			+ inst.ProductID + "\", \""
			+ inst.ExchangeID + "\", \""
			+ inst.ExchangeInstID + "\", \""
			+ inst.ProductClass + "\", \""
			+ boost::lexical_cast<std::string>(inst.DeliveryYear) + "\", \""
			+ boost::lexical_cast<std::string>(inst.DeliveryMonth) + "\", \""
			+ boost::lexical_cast<std::string>(inst.MaxMarketOrderVolume) + "\", \""
			+ boost::lexical_cast<std::string>(inst.MinMarketOrderVolume) + "\", \""
			+ boost::lexical_cast<std::string>(inst.MaxLimitOrderVolume) + "\", \""
			+ boost::lexical_cast<std::string>(inst.MinLimitOrderVolume) + "\", \""
			+ boost::lexical_cast<std::string>(inst.VolumeMultiple) + "\", \""
			+ boost::lexical_cast<std::string>(inst.PriceTick) + "\", \""
			+ inst.CreateDate + "\", \""
			+ inst.OpenDate + "\", \""
			+ inst.ExpireDate + "\", \""
			+ inst.StartDelivDate + "\", \""
			+ inst.EndDelivDate + "\", \""
			+ inst.InstLifePhase + "\", \""
			+ boost::lexical_cast<std::string>(inst.IsTrading) + "\", \""
			+ inst.PositionType + "\", \""
			+ inst.PositionDateType + "\", \""
			+ boost::lexical_cast<std::string>(inst.LongMarginRatio) + "\", \""
			+ boost::lexical_cast<std::string>(inst.ShortMarginRatio) + "\", \""
			+ boost::lexical_cast<std::string>(inst.IsArbitrage) + "\", \""
			+ inst.UnderlyingInstrID + "\", \""
			+ boost::lexical_cast<std::string>(inst.StrikePrice) + "\", \""
			+ inst.OptionsType + "\", \""
			+ boost::lexical_cast<std::string>(inst.UnderlyingMultiple) + "\", \""
			+ inst.CombinationType
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetInstrumentInfo(InstrumentInfoField &field, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INSTRUMENT_TABLE;
		strSql = strSql + "` WHERE InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount < 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.InstrumentID = recordSet[0][0];
		field.InstrumentName = recordSet[0][1];
		field.ProductID = recordSet[0][2];
		field.ExchangeID = recordSet[0][3];
		field.ExchangeInstID = recordSet[0][4];
		field.ProductClass = recordSet[0][5];
		field.DeliveryYear = toInt(recordSet[0][6]);
		field.DeliveryMonth = toInt(recordSet[0][7]);
		field.MaxMarketOrderVolume = toInt(recordSet[0][8]);
		field.MinMarketOrderVolume = toInt(recordSet[0][9]);
		field.MaxLimitOrderVolume = toInt(recordSet[0][10]);
		field.MinLimitOrderVolume = toInt(recordSet[0][11]);
		field.VolumeMultiple = toDouble(recordSet[0][12]);
		field.PriceTick = toDouble(recordSet[0][13]);
		field.CreateDate = recordSet[0][14];
		field.OpenDate = recordSet[0][15];
		field.ExpireDate = recordSet[0][16];
		field.StartDelivDate = recordSet[0][17];
		field.EndDelivDate = recordSet[0][18];
		field.InstLifePhase = recordSet[0][19];
		field.IsTrading = toInt(recordSet[0][20]);
		field.PositionType = recordSet[0][21];
		field.PositionDateType = recordSet[0][22];
		field.LongMarginRatio = toDouble(recordSet[0][23]);
		field.ShortMarginRatio = toDouble(recordSet[0][24]);
		field.IsArbitrage = toInt(recordSet[0][25]);
		field.UnderlyingInstrID = recordSet[0][26];
		field.StrikePrice = toDouble(recordSet[0][27]);
		field.OptionsType = recordSet[0][28];
		field.UnderlyingMultiple = toDouble(recordSet[0][29]);
		field.CombinationType = recordSet[0][30];

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry instrument " << strInstrumentID << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetInstrumentInfo(vector<InstrumentInfoField> &fieldsVec)
{
	try
	{
		fieldsVec.clear();
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INSTRUMENT_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		InstrumentInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.InstrumentID = recordSet[row][0];
			field.InstrumentName = recordSet[row][1];
			field.ProductID = recordSet[row][2];
			field.ExchangeID = recordSet[row][3];
			field.ExchangeInstID = recordSet[row][4];
			field.ProductClass = recordSet[row][5];
			field.DeliveryYear = toInt(recordSet[row][6]);
			field.DeliveryMonth = toInt(recordSet[row][7]);
			field.MaxMarketOrderVolume = toInt(recordSet[row][8]);
			field.MinMarketOrderVolume = toInt(recordSet[row][9]);
			field.MaxLimitOrderVolume = toInt(recordSet[row][10]);
			field.MinLimitOrderVolume = toInt(recordSet[row][11]);
			field.VolumeMultiple = toDouble(recordSet[row][12]);
			field.PriceTick = toDouble(recordSet[row][13]);
			field.CreateDate = recordSet[row][14];
			field.OpenDate = recordSet[row][15];
			field.ExpireDate = recordSet[row][16];
			field.StartDelivDate = recordSet[row][17];
			field.EndDelivDate = recordSet[row][18];
			field.InstLifePhase = recordSet[row][19];
			field.IsTrading = toInt(recordSet[row][20]);
			field.PositionType = recordSet[row][21];
			field.PositionDateType = recordSet[row][22];
			field.LongMarginRatio = toDouble(recordSet[row][23]);
			field.ShortMarginRatio = toDouble(recordSet[row][24]);
			field.IsArbitrage = toInt(recordSet[row][25]);
			field.UnderlyingInstrID = recordSet[row][26];
			field.StrikePrice = toDouble(recordSet[row][27]);
			field.OptionsType = recordSet[row][28];
			field.UnderlyingMultiple = toDouble(recordSet[row][29]);
			field.CombinationType = recordSet[row][30];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all instruments info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::DelInstrumentInfo(string strInstrumentID)
{
	string strSql = (string)"DELETE FROM `" + INSTRUMENT_TABLE + "` WHERE InstrumentID = '" + strInstrumentID + "\';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 保证金模板信息
bool CDBOperation::GetMarginRateModelInfo(MarginRateModelInfoField &field, int modelid)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + MARGINRATEMODEL_TABLE;
		strSql = strSql + "` WHERE MarginRateModelID = '" + toString(modelid) + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.MarginRateModelID = toInt(recordSet[0][0]);
		field.MarginRateModelName = recordSet[0][1];
		field.Creator = recordSet[0][2];
		field.CreateDate = recordSet[0][3];
		field.Comment = recordSet[0][4];
		field.IsBasicsModel = toInt(recordSet[0][5]);
		field.BaseOnBasics = toDouble(recordSet[0][6]);

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry margin rate model " << toString(modelid) << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllMarginRateModelInfo(std::vector<MarginRateModelInfoField> &fieldVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + MARGINRATEMODEL_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		MarginRateModelInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.MarginRateModelID = toInt(recordSet[row][0]);
			field.MarginRateModelName = recordSet[row][1];
			field.Creator = recordSet[row][2];
			field.CreateDate = recordSet[row][3];
			field.Comment = recordSet[row][4];
			field.IsBasicsModel = toInt(recordSet[row][5]);
			field.BaseOnBasics = toDouble(recordSet[row][6]);

			fieldVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all margin rate model faild !" << std::endl;
		return false;
	}
}

// 保证金信息
bool CDBOperation::UpdMarginRateInfo(MarginRateInfoField &field)
{
	string strSql = (string)"REPLACE INTO `" + MARGINRATE_TABLE +"` VALUES ( \"" \
			+ field.InstrumentID + "\", \""
			+ field.InvestorRange + "\", \""
			+ field.HedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(field.LongMarginRatioByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(field.LongMarginRatioByVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.ShortMarginRatioByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(field.ShortMarginRatioByVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.IsRelative)
			+ "\" );";
	//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetMarginRateInfo(MarginRateInfoField &field, int modelid, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + MARGINRATE_TABLE;
//		strSql = strSql + "` WHERE MarginRateModelID = '" + toString(modelid) + "' AND InstrumentID = '" + strInstrumentID + "';";
		strSql = strSql + "` WHERE InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.MarginRateModelID = modelid;
		field.InstrumentID = recordSet[0][0];
		field.InvestorRange = recordSet[0][1];
		field.HedgeFlag = recordSet[0][2];
		field.LongMarginRatioByMoney = toDouble(recordSet[0][3]);
		field.LongMarginRatioByVolume = toDouble(recordSet[0][4]);
		field.ShortMarginRatioByMoney = toDouble(recordSet[0][5]);
		field.ShortMarginRatioByVolume = toDouble(recordSet[0][6]);
		field.IsRelative = toInt(recordSet[0][7]);

		MarginRateModelInfoField marginRateModel;
		if(GetMarginRateModelInfo(marginRateModel, modelid))
		{
			if(Greater(marginRateModel.BaseOnBasics, 1.0))
			{
				field.LongMarginRatioByMoney = field.LongMarginRatioByMoney * marginRateModel.BaseOnBasics;
				field.LongMarginRatioByVolume = field.LongMarginRatioByVolume * marginRateModel.BaseOnBasics;
				field.ShortMarginRatioByMoney = field.ShortMarginRatioByMoney * marginRateModel.BaseOnBasics;
				field.ShortMarginRatioByVolume = field.ShortMarginRatioByVolume * marginRateModel.BaseOnBasics;
			}
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry margin rate of " << toString(modelid) << " and " << strInstrumentID << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetMarginRateInfo(vector<MarginRateInfoField> &fieldsVec, int modelid)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + MARGINRATE_TABLE + "`;";
//		strSql = strSql + "` WHERE MarginRateModelID = '" + toString(modelid) + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		MarginRateModelInfoField marginRateModel;
		bool ret = GetMarginRateModelInfo(marginRateModel, modelid);

		MarginRateInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.MarginRateModelID = modelid;
			field.InstrumentID = recordSet[row][0];
			field.InvestorRange = recordSet[row][1];
			field.HedgeFlag = recordSet[row][2];
			field.LongMarginRatioByMoney = toDouble(recordSet[row][3]);
			field.LongMarginRatioByVolume = toDouble(recordSet[row][4]);
			field.ShortMarginRatioByMoney = toDouble(recordSet[row][5]);
			field.ShortMarginRatioByVolume = toDouble(recordSet[row][6]);
			field.IsRelative = toInt(recordSet[row][7]);

			if(ret && Greater(marginRateModel.BaseOnBasics, 1.0))
			{
				field.LongMarginRatioByMoney = field.LongMarginRatioByMoney * marginRateModel.BaseOnBasics;
				field.LongMarginRatioByVolume = field.LongMarginRatioByVolume * marginRateModel.BaseOnBasics;
				field.ShortMarginRatioByMoney = field.ShortMarginRatioByMoney * marginRateModel.BaseOnBasics;
				field.ShortMarginRatioByVolume = field.ShortMarginRatioByVolume * marginRateModel.BaseOnBasics;
			}

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry margin rate of " << toString(modelid)  << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetMarginRateInfo(vector<MarginRateInfoField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + MARGINRATE_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		MarginRateInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
//			field.MarginRateModelID = toInt(recordSet[row][0]);
			field.InstrumentID = recordSet[row][0];
			field.InvestorRange = recordSet[row][1];
			field.HedgeFlag = recordSet[row][2];
			field.LongMarginRatioByMoney = toDouble(recordSet[row][3]);
			field.LongMarginRatioByVolume = toDouble(recordSet[row][4]);
			field.ShortMarginRatioByMoney = toDouble(recordSet[row][5]);
			field.ShortMarginRatioByVolume = toDouble(recordSet[row][6]);
			field.IsRelative = toInt(recordSet[row][7]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all margin rate info faild !" << std::endl;
		return false;
	}
}

// 手续费模板信息
bool CDBOperation::GetCommissionRateModelInfo(CommissionRateModelInfoField &field, int modelid)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + COMMISSIONRATEMODEL_TABLE;
		strSql = strSql + "` WHERE CommissionRateModelID = '" + toString(modelid) + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.CommissionRateModelID = toInt(recordSet[0][0]);
		field.CommissionRateModelName = recordSet[0][1];
		field.Creator = recordSet[0][2];
		field.CreateDate = recordSet[0][3];
		field.Comment = recordSet[0][4];
		field.IsBasicsModel = toInt(recordSet[0][5]);
		field.BaseOnBasicsByMoney = toDouble(recordSet[0][6]);
		field.BaseOnBasicsByVolume = toDouble(recordSet[0][7]);

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry commission rate model " << toString(modelid) << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllCommissionRateModelInfo(std::vector<CommissionRateModelInfoField> &fieldVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + COMMISSIONRATEMODEL_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		CommissionRateModelInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CommissionRateModelID = toInt(recordSet[row][0]);
			field.CommissionRateModelName = recordSet[row][1];
			field.Creator = recordSet[row][2];
			field.CreateDate = recordSet[row][3];
			field.Comment = recordSet[row][4];
			field.IsBasicsModel = toInt(recordSet[row][5]);
			field.BaseOnBasicsByMoney = toDouble(recordSet[row][6]);
			field.BaseOnBasicsByVolume = toDouble(recordSet[row][7]);

			fieldVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all commission rate model faild !" << std::endl;
		return false;
	}
}

// 手续费信息
bool CDBOperation::UpdCommissionRateInfo(CommissionRateInfoField &field)
{
	string strSql = (string)"REPLACE INTO `" + COMMISSIONRATE_TABLE +"` VALUES ( \"" \
			+ field.InstrumentID + "\", \""
			+ field.InvestorRange + "\", \""
			+ boost::lexical_cast<std::string>(field.OpenRatioByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(field.OpenRatioByVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.CloseRatioByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(field.CloseRatioByVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.CloseTodayRatioByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(field.CloseTodayRatioByVolume)
			+ "\" );";
	//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetCommissionRateInfo(CommissionRateInfoField &field, int modelid, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + COMMISSIONRATE_TABLE;
//		strSql = strSql + "` WHERE CommissionRateModelID = '" + toString(modelid) + "' AND InstrumentID = '" + strInstrumentID + "';";
		strSql = strSql + "` WHERE InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		field.CommissionRateModelID = modelid;
		field.InstrumentID = recordSet[0][0];
		field.InvestorRange = recordSet[0][1];
		field.OpenRatioByMoney = toDouble(recordSet[0][2]);
		field.OpenRatioByVolume = toDouble(recordSet[0][3]);
		field.CloseRatioByMoney = toDouble(recordSet[0][4]);
		field.CloseRatioByVolume = toDouble(recordSet[0][5]);
		field.CloseTodayRatioByMoney = toDouble(recordSet[0][6]);
		field.CloseTodayRatioByVolume = toDouble(recordSet[0][7]);

		CommissionRateModelInfoField commissionRateModel;
		if(GetCommissionRateModelInfo(commissionRateModel, modelid))
		{
			if(Greater(commissionRateModel.BaseOnBasicsByMoney, 1.0))
			{
				field.OpenRatioByMoney = field.OpenRatioByMoney * commissionRateModel.BaseOnBasicsByMoney;
				field.OpenRatioByVolume = field.OpenRatioByVolume * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseRatioByMoney = field.CloseRatioByMoney * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseRatioByVolume = field.CloseRatioByVolume * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseTodayRatioByMoney = field.CloseTodayRatioByMoney * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseTodayRatioByVolume = field.CloseTodayRatioByVolume * commissionRateModel.BaseOnBasicsByMoney;
			}
			if(Greater(commissionRateModel.BaseOnBasicsByVolume, 0.0))
			{
				field.OpenRatioByVolume += commissionRateModel.BaseOnBasicsByVolume;
				field.CloseRatioByVolume += commissionRateModel.BaseOnBasicsByVolume;
				field.CloseTodayRatioByVolume += commissionRateModel.BaseOnBasicsByVolume;
			}
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry commission rate of " << toString(modelid) << " and " << strInstrumentID << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetCommissionRateInfo(vector<CommissionRateInfoField> &fieldsVec, int modelid)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + COMMISSIONRATE_TABLE + "`;";
//		strSql = strSql + "` WHERE CommissionRateModelID = '" + toString(modelid) + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		CommissionRateModelInfoField commissionRateModel;
		bool ret = GetCommissionRateModelInfo(commissionRateModel, modelid);

		CommissionRateInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CommissionRateModelID = modelid;
			field.InstrumentID = recordSet[row][0];
			field.InvestorRange = recordSet[row][1];
			field.OpenRatioByMoney = toDouble(recordSet[row][2]);
			field.OpenRatioByVolume = toDouble(recordSet[row][3]);
			field.CloseRatioByMoney = toDouble(recordSet[row][4]);
			field.CloseRatioByVolume = toDouble(recordSet[row][5]);
			field.CloseTodayRatioByMoney = toDouble(recordSet[row][6]);
			field.CloseTodayRatioByVolume = toDouble(recordSet[row][7]);

			if(ret && Greater(commissionRateModel.BaseOnBasicsByMoney, 1.0))
			{
				field.OpenRatioByMoney = field.OpenRatioByMoney * commissionRateModel.BaseOnBasicsByMoney;
				field.OpenRatioByVolume = field.OpenRatioByVolume * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseRatioByMoney = field.CloseRatioByMoney * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseRatioByVolume = field.CloseRatioByVolume * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseTodayRatioByMoney = field.CloseTodayRatioByMoney * commissionRateModel.BaseOnBasicsByMoney;
				field.CloseTodayRatioByVolume = field.CloseTodayRatioByVolume * commissionRateModel.BaseOnBasicsByMoney;
			}
			if(ret && Greater(commissionRateModel.BaseOnBasicsByVolume, 0.0))
			{
				field.OpenRatioByVolume += commissionRateModel.BaseOnBasicsByVolume;
				field.CloseRatioByVolume += commissionRateModel.BaseOnBasicsByVolume;
				field.CloseTodayRatioByVolume += commissionRateModel.BaseOnBasicsByVolume;
			}

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry commission rate of " << toString(modelid)  << "'s info faild !" << std::endl;
		return false;
	}
}
bool CDBOperation::GetCommissionRateInfo(vector<CommissionRateInfoField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + COMMISSIONRATE_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			THROW(TKException(GetErrorID("COMMON_ERROR_NO_ENTRIES"), GetErrorMsg("COMMON_ERROR_NO_ENTRIES")));
		}
		if(recordCount <= 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		CommissionRateInfoField field;
		for(int row = 0; row < recordCount; row++)
		{
//			field.CommissionRateModelID = toInt(recordSet[row][0]);
			field.InstrumentID = recordSet[row][0];
			field.InvestorRange = recordSet[row][1];
			field.OpenRatioByMoney = toDouble(recordSet[row][2]);
			field.OpenRatioByVolume = toDouble(recordSet[row][3]);
			field.CloseRatioByMoney = toDouble(recordSet[row][4]);
			field.CloseRatioByVolume = toDouble(recordSet[row][5]);
			field.CloseTodayRatioByMoney = toDouble(recordSet[row][6]);
			field.CloseTodayRatioByVolume = toDouble(recordSet[row][7]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all commission rate info faild !" << std::endl;
		return false;
	}
}
// 委托信息
bool CDBOperation::UpdOrder(OrderField &order)
{
	string strSql = (string)"REPLACE INTO `" + ORDER_TABLE +"` VALUES ( \"" \
			+ order.BrokerID + "\", \""
			+ order.InvestorID + "\", \""
			+ order.InstrumentID + "\", \""
			+ order.OrderRef + "\", \""
			+ order.UserID + "\", \""
			+ order.OrderPriceType + "\", \""
			+ order.Direction + "\", \""
			+ order.CombOffsetFlag + "\", \""
			+ order.CombHedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(order.LimitPrice) + "\", \""
			+ boost::lexical_cast<std::string>(order.VolumeTotalOriginal) + "\", \""
			+ order.TimeCondition + "\", \""
			+ order.GTDDate + "\", \""
			+ order.VolumeCondition + "\", \""
			+ boost::lexical_cast<std::string>(order.MinVolume) + "\", \""
			+ order.ContingentCondition + "\", \""
			+ boost::lexical_cast<std::string>(order.StopPrice) + "\", \""
			+ order.ForceCloseReason + "\", \""
			+ boost::lexical_cast<std::string>(order.IsAutoSuspend) + "\", \""
			+ order.BusinessUnit + "\", \""
			+ boost::lexical_cast<std::string>(order.RequestID) + "\", \""
			+ order.OrderLocalID + "\", \""
			+ order.ExchangeID + "\", \""
			+ order.ParticipantID + "\", \""
			+ order.ClientID + "\", \""
			+ order.ExchangeInstID + "\", \""
			+ order.TraderID + "\", \""
			+ boost::lexical_cast<std::string>(order.InstallID) + "\", \""
			+ order.OrderSubmitStatus + "\", \""
			+ boost::lexical_cast<std::string>(order.NotifySequence) + "\", \""
			+ order.TradingDay + "\", \""
			+ boost::lexical_cast<std::string>(order.SettlementID) + "\", \""
			+ order.OrderSysID + "\", \""
			+ order.OrderSource + "\", \""
			+ order.OrderStatus + "\", \""
			+ order.OrderType + "\", \""
			+ boost::lexical_cast<std::string>(order.VolumeTraded) + "\", \""
			+ boost::lexical_cast<std::string>(order.VolumeTotal) + "\", \""
			+ order.InsertDate + "\", \""
			+ order.InsertTime + "\", \""
			+ order.ActiveTime + "\", \""
			+ order.SuspendTime + "\", \""
			+ order.UpdateTime + "\", \""
			+ order.CancelTime + "\", \""
			+ order.ActiveTraderID + "\", \""
			+ order.ClearingPartID + "\", \""
			+ boost::lexical_cast<std::string>(order.SequenceNo) + "\", \""
			+ boost::lexical_cast<std::string>(order.FrontID) + "\", \""
			+ boost::lexical_cast<std::string>(order.SessionID) + "\", \""
			+ order.UserProductInfo + "\", \""
			+ order.StatusMsg + "\", \""
			+ boost::lexical_cast<std::string>(order.UserForceClose) + "\", \""
			+ order.ActiveUserID + "\", \""
			+ boost::lexical_cast<std::string>(order.BrokerOrderSeq) + "\", \""
			+ order.RelativeOrderSysID + "\", \""
			+ boost::lexical_cast<std::string>(order.ZCETotalTradedVolume) + "\", \""
			+ boost::lexical_cast<std::string>(order.IsSwapOrder)
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::CleanOrder()
{
	string strSql = (string)"truncate table `" + ORDER_TABLE +"`;" ;

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetOrder(vector<OrderField> &ordersVec, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + ORDER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			field.OrderPriceType = recordSet[row][5][0];
			field.Direction = recordSet[row][6][0];
			field.CombOffsetFlag[0] = recordSet[row][7][0];
			field.CombHedgeFlag[0] = recordSet[row][8][0];
			field.LimitPrice = toDouble(recordSet[row][9]);
			field.VolumeTotalOriginal = toInt(recordSet[row][10]);
			field.TimeCondition = recordSet[row][11][0];
			strcpy(field.GTDDate, recordSet[row][12].c_str());
			field.VolumeCondition = recordSet[row][13][0];
			field.MinVolume = toInt(recordSet[row][14]);
			field.ContingentCondition = recordSet[row][15][0];
			field.StopPrice = toDouble(recordSet[row][16]);
			field.ForceCloseReason = recordSet[row][17][0];
			field.IsAutoSuspend = toInt(recordSet[row][18]);
			strcpy(field.BusinessUnit, recordSet[row][19].c_str());
			field.RequestID = toInt(recordSet[row][20]);
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ExchangeID, recordSet[row][22].c_str());
			strcpy(field.ParticipantID, recordSet[row][23].c_str());
			strcpy(field.ClientID, recordSet[row][24].c_str());
			strcpy(field.ExchangeInstID, recordSet[row][25].c_str());
			strcpy(field.TraderID, recordSet[row][26].c_str());
			field.InstallID = toInt(recordSet[row][27]);
			field.OrderSubmitStatus = recordSet[row][28][0];
			field.NotifySequence = toInt(recordSet[row][29]);
			strcpy(field.TradingDay, recordSet[row][30].c_str());
			field.SettlementID = toInt(recordSet[row][31]);
			strcpy(field.OrderSysID, recordSet[row][32].c_str());
			field.OrderSource = recordSet[row][33][0];
			field.OrderStatus = recordSet[row][34][0];
			field.OrderType = recordSet[row][35][0];
			field.VolumeTraded = toInt(recordSet[row][36]);
			field.VolumeTotal = toInt(recordSet[row][37]);
			strcpy(field.InsertDate, recordSet[row][38].c_str());
			strcpy(field.InsertTime, recordSet[row][39].c_str());
			strcpy(field.ActiveTime, recordSet[row][40].c_str());
			strcpy(field.SuspendTime, recordSet[row][41].c_str());
			strcpy(field.UpdateTime, recordSet[row][42].c_str());
			strcpy(field.CancelTime, recordSet[row][43].c_str());
			strcpy(field.ActiveTraderID, recordSet[row][44].c_str());
			strcpy(field.ClearingPartID, recordSet[row][45].c_str());
			field.SequenceNo = toInt(recordSet[row][46]);
			field.FrontID = toInt(recordSet[row][47]);
			field.SessionID = toInt(recordSet[row][48]);
			strcpy(field.UserProductInfo, recordSet[row][49].c_str());
			strcpy(field.StatusMsg, recordSet[row][50].c_str());
			field.UserForceClose = toInt(recordSet[row][51]);
			strcpy(field.ActiveUserID, recordSet[row][52].c_str());
			field.BrokerOrderSeq = toInt(recordSet[row][53]);
			strcpy(field.RelativeOrderSysID, recordSet[row][54].c_str());
			field.ZCETotalTradedVolume = toInt(recordSet[row][55]);
			field.IsSwapOrder = toInt(recordSet[row][56]);

			ordersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all orders of " << strTradingDay << "and investor:" << strBrokerID << "-" << strInvestorID  << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllOrder(vector<OrderField> &ordersVec, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + ORDER_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			field.OrderPriceType = recordSet[row][5][0];
			field.Direction = recordSet[row][6][0];
			field.CombOffsetFlag[0] = recordSet[row][7][0];
			field.CombHedgeFlag[0] = recordSet[row][8][0];
			field.LimitPrice = toDouble(recordSet[row][9]);
			field.VolumeTotalOriginal = toInt(recordSet[row][10]);
			field.TimeCondition = recordSet[row][11][0];
			strcpy(field.GTDDate, recordSet[row][12].c_str());
			field.VolumeCondition = recordSet[row][13][0];
			field.MinVolume = toInt(recordSet[row][14]);
			field.ContingentCondition = recordSet[row][15][0];
			field.StopPrice = toDouble(recordSet[row][16]);
			field.ForceCloseReason = recordSet[row][17][0];
			field.IsAutoSuspend = toInt(recordSet[row][18]);
			strcpy(field.BusinessUnit, recordSet[row][19].c_str());
			field.RequestID = toInt(recordSet[row][20]);
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ExchangeID, recordSet[row][22].c_str());
			strcpy(field.ParticipantID, recordSet[row][23].c_str());
			strcpy(field.ClientID, recordSet[row][24].c_str());
			strcpy(field.ExchangeInstID, recordSet[row][25].c_str());
			strcpy(field.TraderID, recordSet[row][26].c_str());
			field.InstallID = toInt(recordSet[row][27]);
			field.OrderSubmitStatus = recordSet[row][28][0];
			field.NotifySequence = toInt(recordSet[row][29]);
			strcpy(field.TradingDay, recordSet[row][30].c_str());
			field.SettlementID = toInt(recordSet[row][31]);
			strcpy(field.OrderSysID, recordSet[row][32].c_str());
			field.OrderSource = recordSet[row][33][0];
			field.OrderStatus = recordSet[row][34][0];
			field.OrderType = recordSet[row][35][0];
			field.VolumeTraded = toInt(recordSet[row][36]);
			field.VolumeTotal = toInt(recordSet[row][37]);
			strcpy(field.InsertDate, recordSet[row][38].c_str());
			strcpy(field.InsertTime, recordSet[row][39].c_str());
			strcpy(field.ActiveTime, recordSet[row][40].c_str());
			strcpy(field.SuspendTime, recordSet[row][41].c_str());
			strcpy(field.UpdateTime, recordSet[row][42].c_str());
			strcpy(field.CancelTime, recordSet[row][43].c_str());
			strcpy(field.ActiveTraderID, recordSet[row][44].c_str());
			strcpy(field.ClearingPartID, recordSet[row][45].c_str());
			field.SequenceNo = toInt(recordSet[row][46]);
			field.FrontID = toInt(recordSet[row][47]);
			field.SessionID = toInt(recordSet[row][48]);
			strcpy(field.UserProductInfo, recordSet[row][49].c_str());
			strcpy(field.StatusMsg, recordSet[row][50].c_str());
			field.UserForceClose = toInt(recordSet[row][51]);
			strcpy(field.ActiveUserID, recordSet[row][52].c_str());
			field.BrokerOrderSeq = toInt(recordSet[row][53]);
			strcpy(field.RelativeOrderSysID, recordSet[row][54].c_str());
			field.ZCETotalTradedVolume = toInt(recordSet[row][55]);
			field.IsSwapOrder = toInt(recordSet[row][56]);

			ordersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all orders of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllOrder(vector<OrderField> &ordersVec, string strTradingDay, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + ORDER_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "' AND InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			field.OrderPriceType = recordSet[row][5][0];
			field.Direction = recordSet[row][6][0];
			field.CombOffsetFlag[0] = recordSet[row][7][0];
			field.CombHedgeFlag[0] = recordSet[row][8][0];
			field.LimitPrice = toDouble(recordSet[row][9]);
			field.VolumeTotalOriginal = toInt(recordSet[row][10]);
			field.TimeCondition = recordSet[row][11][0];
			strcpy(field.GTDDate, recordSet[row][12].c_str());
			field.VolumeCondition = recordSet[row][13][0];
			field.MinVolume = toInt(recordSet[row][14]);
			field.ContingentCondition = recordSet[row][15][0];
			field.StopPrice = toDouble(recordSet[row][16]);
			field.ForceCloseReason = recordSet[row][17][0];
			field.IsAutoSuspend = toInt(recordSet[row][18]);
			strcpy(field.BusinessUnit, recordSet[row][19].c_str());
			field.RequestID = toInt(recordSet[row][20]);
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ExchangeID, recordSet[row][22].c_str());
			strcpy(field.ParticipantID, recordSet[row][23].c_str());
			strcpy(field.ClientID, recordSet[row][24].c_str());
			strcpy(field.ExchangeInstID, recordSet[row][25].c_str());
			strcpy(field.TraderID, recordSet[row][26].c_str());
			field.InstallID = toInt(recordSet[row][27]);
			field.OrderSubmitStatus = recordSet[row][28][0];
			field.NotifySequence = toInt(recordSet[row][29]);
			strcpy(field.TradingDay, recordSet[row][30].c_str());
			field.SettlementID = toInt(recordSet[row][31]);
			strcpy(field.OrderSysID, recordSet[row][32].c_str());
			field.OrderSource = recordSet[row][33][0];
			field.OrderStatus = recordSet[row][34][0];
			field.OrderType = recordSet[row][35][0];
			field.VolumeTraded = toInt(recordSet[row][36]);
			field.VolumeTotal = toInt(recordSet[row][37]);
			strcpy(field.InsertDate, recordSet[row][38].c_str());
			strcpy(field.InsertTime, recordSet[row][39].c_str());
			strcpy(field.ActiveTime, recordSet[row][40].c_str());
			strcpy(field.SuspendTime, recordSet[row][41].c_str());
			strcpy(field.UpdateTime, recordSet[row][42].c_str());
			strcpy(field.CancelTime, recordSet[row][43].c_str());
			strcpy(field.ActiveTraderID, recordSet[row][44].c_str());
			strcpy(field.ClearingPartID, recordSet[row][45].c_str());
			field.SequenceNo = toInt(recordSet[row][46]);
			field.FrontID = toInt(recordSet[row][47]);
			field.SessionID = toInt(recordSet[row][48]);
			strcpy(field.UserProductInfo, recordSet[row][49].c_str());
			strcpy(field.StatusMsg, recordSet[row][50].c_str());
			field.UserForceClose = toInt(recordSet[row][51]);
			strcpy(field.ActiveUserID, recordSet[row][52].c_str());
			field.BrokerOrderSeq = toInt(recordSet[row][53]);
			strcpy(field.RelativeOrderSysID, recordSet[row][54].c_str());
			field.ZCETotalTradedVolume = toInt(recordSet[row][55]);
			field.IsSwapOrder = toInt(recordSet[row][56]);

			ordersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all orders of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllOrder(vector<OrderField> &ordersVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + ORDER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			field.OrderPriceType = recordSet[row][5][0];
			field.Direction = recordSet[row][6][0];
			field.CombOffsetFlag[0] = recordSet[row][7][0];
			field.CombHedgeFlag[0] = recordSet[row][8][0];
			field.LimitPrice = toDouble(recordSet[row][9]);
			field.VolumeTotalOriginal = toInt(recordSet[row][10]);
			field.TimeCondition = recordSet[row][11][0];
			strcpy(field.GTDDate, recordSet[row][12].c_str());
			field.VolumeCondition = recordSet[row][13][0];
			field.MinVolume = toInt(recordSet[row][14]);
			field.ContingentCondition = recordSet[row][15][0];
			field.StopPrice = toDouble(recordSet[row][16]);
			field.ForceCloseReason = recordSet[row][17][0];
			field.IsAutoSuspend = toInt(recordSet[row][18]);
			strcpy(field.BusinessUnit, recordSet[row][19].c_str());
			field.RequestID = toInt(recordSet[row][20]);
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ExchangeID, recordSet[row][22].c_str());
			strcpy(field.ParticipantID, recordSet[row][23].c_str());
			strcpy(field.ClientID, recordSet[row][24].c_str());
			strcpy(field.ExchangeInstID, recordSet[row][25].c_str());
			strcpy(field.TraderID, recordSet[row][26].c_str());
			field.InstallID = toInt(recordSet[row][27]);
			field.OrderSubmitStatus = recordSet[row][28][0];
			field.NotifySequence = toInt(recordSet[row][29]);
			strcpy(field.TradingDay, recordSet[row][30].c_str());
			field.SettlementID = toInt(recordSet[row][31]);
			strcpy(field.OrderSysID, recordSet[row][32].c_str());
			field.OrderSource = recordSet[row][33][0];
			field.OrderStatus = recordSet[row][34][0];
			field.OrderType = recordSet[row][35][0];
			field.VolumeTraded = toInt(recordSet[row][36]);
			field.VolumeTotal = toInt(recordSet[row][37]);
			strcpy(field.InsertDate, recordSet[row][38].c_str());
			strcpy(field.InsertTime, recordSet[row][39].c_str());
			strcpy(field.ActiveTime, recordSet[row][40].c_str());
			strcpy(field.SuspendTime, recordSet[row][41].c_str());
			strcpy(field.UpdateTime, recordSet[row][42].c_str());
			strcpy(field.CancelTime, recordSet[row][43].c_str());
			strcpy(field.ActiveTraderID, recordSet[row][44].c_str());
			strcpy(field.ClearingPartID, recordSet[row][45].c_str());
			field.SequenceNo = toInt(recordSet[row][46]);
			field.FrontID = toInt(recordSet[row][47]);
			field.SessionID = toInt(recordSet[row][48]);
			strcpy(field.UserProductInfo, recordSet[row][49].c_str());
			strcpy(field.StatusMsg, recordSet[row][50].c_str());
			field.UserForceClose = toInt(recordSet[row][51]);
			strcpy(field.ActiveUserID, recordSet[row][52].c_str());
			field.BrokerOrderSeq = toInt(recordSet[row][53]);
			strcpy(field.RelativeOrderSysID, recordSet[row][54].c_str());
			field.ZCETotalTradedVolume = toInt(recordSet[row][55]);
			field.IsSwapOrder = toInt(recordSet[row][56]);

			ordersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all orders faild !"  << std::endl;
		return false;
	}
}

long long CDBOperation::GetMaxOrderSysID()
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT MAX(ordersysid) FROM `" + ORDER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}
		if(recordCount == 0)
			return 0;

	   return toLongLong(recordSet[0][0]);
	}
	catch(...)
	{
		ERROR << "Qry max ordersysid faild !"  << std::endl;
		return -1;
	}
}
bool CDBOperation::GetInvestorsOfOrder(set<string> &investorsSet)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT DISTINCT investorid FROM `" + ORDER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			investorsSet.insert(recordSet[row][0]);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all investors of orders faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorOrder(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + ORDER_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 历史委托信息
bool CDBOperation::UpdHisOrder(OrderField &order)
{
	string strSql = (string)"REPLACE INTO `" + HISORDER_TABLE +"` VALUES ( \"" \
			+ order.BrokerID + "\", \""
			+ order.InvestorID + "\", \""
			+ order.InstrumentID + "\", \""
			+ order.OrderRef + "\", \""
			+ order.UserID + "\", \""
			+ order.OrderPriceType + "\", \""
			+ order.Direction + "\", \""
			+ order.CombOffsetFlag + "\", \""
			+ order.CombHedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(order.LimitPrice) + "\", \""
			+ boost::lexical_cast<std::string>(order.VolumeTotalOriginal) + "\", \""
			+ order.TimeCondition + "\", \""
			+ order.GTDDate + "\", \""
			+ order.VolumeCondition + "\", \""
			+ boost::lexical_cast<std::string>(order.MinVolume) + "\", \""
			+ order.ContingentCondition + "\", \""
			+ boost::lexical_cast<std::string>(order.StopPrice) + "\", \""
			+ order.ForceCloseReason + "\", \""
			+ boost::lexical_cast<std::string>(order.IsAutoSuspend) + "\", \""
			+ order.BusinessUnit + "\", \""
			+ boost::lexical_cast<std::string>(order.RequestID) + "\", \""
			+ order.OrderLocalID + "\", \""
			+ order.ExchangeID + "\", \""
			+ order.ParticipantID + "\", \""
			+ order.ClientID + "\", \""
			+ order.ExchangeInstID + "\", \""
			+ order.TraderID + "\", \""
			+ boost::lexical_cast<std::string>(order.InstallID) + "\", \""
			+ order.OrderSubmitStatus + "\", \""
			+ boost::lexical_cast<std::string>(order.NotifySequence) + "\", \""
			+ order.TradingDay + "\", \""
			+ boost::lexical_cast<std::string>(order.SettlementID) + "\", \""
			+ order.OrderSysID + "\", \""
			+ order.OrderSource + "\", \""
			+ order.OrderStatus + "\", \""
			+ order.OrderType + "\", \""
			+ boost::lexical_cast<std::string>(order.VolumeTraded) + "\", \""
			+ boost::lexical_cast<std::string>(order.VolumeTotal) + "\", \""
			+ order.InsertDate + "\", \""
			+ order.InsertTime + "\", \""
			+ order.ActiveTime + "\", \""
			+ order.SuspendTime + "\", \""
			+ order.UpdateTime + "\", \""
			+ order.CancelTime + "\", \""
			+ order.ActiveTraderID + "\", \""
			+ order.ClearingPartID + "\", \""
			+ boost::lexical_cast<std::string>(order.SequenceNo) + "\", \""
			+ boost::lexical_cast<std::string>(order.FrontID) + "\", \""
			+ boost::lexical_cast<std::string>(order.SessionID) + "\", \""
			+ order.UserProductInfo + "\", \""
			+ order.StatusMsg + "\", \""
			+ boost::lexical_cast<std::string>(order.UserForceClose) + "\", \""
			+ order.ActiveUserID + "\", \""
			+ boost::lexical_cast<std::string>(order.BrokerOrderSeq) + "\", \""
			+ order.RelativeOrderSysID + "\", \""
			+ boost::lexical_cast<std::string>(order.ZCETotalTradedVolume) + "\", \""
			+ boost::lexical_cast<std::string>(order.IsSwapOrder) + "\", \""
			+ "0\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetHisOrder(vector<OrderField> &ordersVec, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISORDER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			field.OrderPriceType = recordSet[row][5][0];
			field.Direction = recordSet[row][6][0];
			field.CombOffsetFlag[0] = recordSet[row][7][0];
			field.CombHedgeFlag[0] = recordSet[row][8][0];
			field.LimitPrice = toDouble(recordSet[row][9]);
			field.VolumeTotalOriginal = toInt(recordSet[row][10]);
			field.TimeCondition = recordSet[row][11][0];
			strcpy(field.GTDDate, recordSet[row][12].c_str());
			field.VolumeCondition = recordSet[row][13][0];
			field.MinVolume = toInt(recordSet[row][14]);
			field.ContingentCondition = recordSet[row][15][0];
			field.StopPrice = toDouble(recordSet[row][16]);
			field.ForceCloseReason = recordSet[row][17][0];
			field.IsAutoSuspend = toInt(recordSet[row][18]);
			strcpy(field.BusinessUnit, recordSet[row][19].c_str());
			field.RequestID = toInt(recordSet[row][20]);
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ExchangeID, recordSet[row][22].c_str());
			strcpy(field.ParticipantID, recordSet[row][23].c_str());
			strcpy(field.ClientID, recordSet[row][24].c_str());
			strcpy(field.ExchangeInstID, recordSet[row][25].c_str());
			strcpy(field.TraderID, recordSet[row][26].c_str());
			field.InstallID = toInt(recordSet[row][27]);
			field.OrderSubmitStatus = recordSet[row][28][0];
			field.NotifySequence = toInt(recordSet[row][29]);
			strcpy(field.TradingDay, recordSet[row][30].c_str());
			field.SettlementID = toInt(recordSet[row][31]);
			strcpy(field.OrderSysID, recordSet[row][32].c_str());
			field.OrderSource = recordSet[row][33][0];
			field.OrderStatus = recordSet[row][34][0];
			field.OrderType = recordSet[row][35][0];
			field.VolumeTraded = toInt(recordSet[row][36]);
			field.VolumeTotal = toInt(recordSet[row][37]);
			strcpy(field.InsertDate, recordSet[row][38].c_str());
			strcpy(field.InsertTime, recordSet[row][39].c_str());
			strcpy(field.ActiveTime, recordSet[row][40].c_str());
			strcpy(field.SuspendTime, recordSet[row][41].c_str());
			strcpy(field.UpdateTime, recordSet[row][42].c_str());
			strcpy(field.CancelTime, recordSet[row][43].c_str());
			strcpy(field.ActiveTraderID, recordSet[row][44].c_str());
			strcpy(field.ClearingPartID, recordSet[row][45].c_str());
			field.SequenceNo = toInt(recordSet[row][46]);
			field.FrontID = toInt(recordSet[row][47]);
			field.SessionID = toInt(recordSet[row][48]);
			strcpy(field.UserProductInfo, recordSet[row][49].c_str());
			strcpy(field.StatusMsg, recordSet[row][50].c_str());
			field.UserForceClose = toInt(recordSet[row][51]);
			strcpy(field.ActiveUserID, recordSet[row][52].c_str());
			field.BrokerOrderSeq = toInt(recordSet[row][53]);
			strcpy(field.RelativeOrderSysID, recordSet[row][54].c_str());
			field.ZCETotalTradedVolume = toInt(recordSet[row][55]);
			field.IsSwapOrder = toInt(recordSet[row][56]);

			ordersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all hisorders of " << strTradingDay << "and investor:" << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllHisOrder(vector<OrderField> &ordersVec, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISORDER_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			field.OrderPriceType = recordSet[row][5][0];
			field.Direction = recordSet[row][6][0];
			field.CombOffsetFlag[0] = recordSet[row][7][0];
			field.CombHedgeFlag[0] = recordSet[row][8][0];
			field.LimitPrice = toDouble(recordSet[row][9]);
			field.VolumeTotalOriginal = toInt(recordSet[row][10]);
			field.TimeCondition = recordSet[row][11][0];
			strcpy(field.GTDDate, recordSet[row][12].c_str());
			field.VolumeCondition = recordSet[row][13][0];
			field.MinVolume = toInt(recordSet[row][14]);
			field.ContingentCondition = recordSet[row][15][0];
			field.StopPrice = toDouble(recordSet[row][16]);
			field.ForceCloseReason = recordSet[row][17][0];
			field.IsAutoSuspend = toInt(recordSet[row][18]);
			strcpy(field.BusinessUnit, recordSet[row][19].c_str());
			field.RequestID = toInt(recordSet[row][20]);
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ExchangeID, recordSet[row][22].c_str());
			strcpy(field.ParticipantID, recordSet[row][23].c_str());
			strcpy(field.ClientID, recordSet[row][24].c_str());
			strcpy(field.ExchangeInstID, recordSet[row][25].c_str());
			strcpy(field.TraderID, recordSet[row][26].c_str());
			field.InstallID = toInt(recordSet[row][27]);
			field.OrderSubmitStatus = recordSet[row][28][0];
			field.NotifySequence = toInt(recordSet[row][29]);
			strcpy(field.TradingDay, recordSet[row][30].c_str());
			field.SettlementID = toInt(recordSet[row][31]);
			strcpy(field.OrderSysID, recordSet[row][32].c_str());
			field.OrderSource = recordSet[row][33][0];
			field.OrderStatus = recordSet[row][34][0];
			field.OrderType = recordSet[row][35][0];
			field.VolumeTraded = toInt(recordSet[row][36]);
			field.VolumeTotal = toInt(recordSet[row][37]);
			strcpy(field.InsertDate, recordSet[row][38].c_str());
			strcpy(field.InsertTime, recordSet[row][39].c_str());
			strcpy(field.ActiveTime, recordSet[row][40].c_str());
			strcpy(field.SuspendTime, recordSet[row][41].c_str());
			strcpy(field.UpdateTime, recordSet[row][42].c_str());
			strcpy(field.CancelTime, recordSet[row][43].c_str());
			strcpy(field.ActiveTraderID, recordSet[row][44].c_str());
			strcpy(field.ClearingPartID, recordSet[row][45].c_str());
			field.SequenceNo = toInt(recordSet[row][46]);
			field.FrontID = toInt(recordSet[row][47]);
			field.SessionID = toInt(recordSet[row][48]);
			strcpy(field.UserProductInfo, recordSet[row][49].c_str());
			strcpy(field.StatusMsg, recordSet[row][50].c_str());
			field.UserForceClose = toInt(recordSet[row][51]);
			strcpy(field.ActiveUserID, recordSet[row][52].c_str());
			field.BrokerOrderSeq = toInt(recordSet[row][53]);
			strcpy(field.RelativeOrderSysID, recordSet[row][54].c_str());
			field.ZCETotalTradedVolume = toInt(recordSet[row][55]);
			field.IsSwapOrder = toInt(recordSet[row][56]);

			ordersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all hisorders of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorHisOrder(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + HISORDER_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}


// 成交信息
bool CDBOperation::UpdTrade(TradeField &trade)
{
	string strSql = (string)"REPLACE INTO `" + TRADE_TABLE + "` VALUES ( \"" \
			+ trade.BrokerID + "\", \""
			+ trade.InvestorID + "\", \""
			+ trade.InstrumentID + "\", \""
			+ trade.OrderRef + "\", \""
			+ trade.UserID + "\", \""
			+ trade.ExchangeID + "\", \""
			+ trade.TradeID + "\", \""
			+ trade.Direction + "\", \""
			+ trade.OrderSysID + "\", \""
			+ trade.ParticipantID + "\", \""
			+ trade.ClientID + "\", \""
			+ trade.TradingRole + "\", \""
			+ trade.ExchangeInstID + "\", \""
			+ trade.OffsetFlag + "\", \""
			+ trade.HedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(trade.Price) + "\", \""
			+ boost::lexical_cast<std::string>(trade.Volume) + "\", \""
			+ trade.TradeDate + "\", \""
			+ trade.TradeTime + "\", \""
			+ trade.TradeType + "\", \""
			+ trade.PriceSource + "\", \""
			+ trade.OrderLocalID + "\", \""
			+ trade.ClearingPartID + "\", \""
			+ trade.BusinessUnit + "\", \""
			+ boost::lexical_cast<std::string>(trade.SequenceNo) + "\", \""
			+ trade.TradingDay + "\", \""
			+ boost::lexical_cast<std::string>(trade.SettlementID) + "\", \""
			+ boost::lexical_cast<std::string>(trade.BrokerOrderSeq) + "\", \""
			+ trade.TradeSource + "\", \""
			+ boost::lexical_cast<std::string>(trade.CloseProfit) + "\", \""
			+ boost::lexical_cast<std::string>(trade.Commission)
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::CleanTrade()
{
	string strSql = (string)"truncate table `" + TRADE_TABLE +"`;" ;

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADE_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all trades of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADE_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all trades of " << strBrokerID << "-" << strInvestorID << "-" << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllTrade(vector<TradeField> &tradesVec, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADE_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all trades of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllTrade(vector<TradeField> &tradesVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADE_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all trades of faild !"  << std::endl;
		return false;
	}
}

long long CDBOperation::GetMaxTradeID()
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT MAX(tradeid) FROM `" + TRADE_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return -1;
		}
		if(recordCount == 0)
			return 0;

	   return toLongLong(recordSet[0][0]);
	}
	catch(...)
	{
		ERROR << "Qry max tradeid faild !"  << std::endl;
		return -1;
	}
}
bool CDBOperation::GetInvestorsOfTrade(set<string> &investorsSet)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT DISTINCT investorid FROM `" + TRADE_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		OrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			investorsSet.insert(recordSet[row][0]);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all investors of trades faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorTrade(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + TRADE_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 历史成交信息
bool CDBOperation::UpdHisTrade(TradeField &trade)
{
	string strSql = (string)"REPLACE INTO `" + HISTRADE_TABLE + "` VALUES ( \"" \
			+ trade.BrokerID + "\", \""
			+ trade.InvestorID + "\", \""
			+ trade.InstrumentID + "\", \""
			+ trade.OrderRef + "\", \""
			+ trade.UserID + "\", \""
			+ trade.ExchangeID + "\", \""
			+ trade.TradeID + "\", \""
			+ trade.Direction + "\", \""
			+ trade.OrderSysID + "\", \""
			+ trade.ParticipantID + "\", \""
			+ trade.ClientID + "\", \""
			+ trade.TradingRole + "\", \""
			+ trade.ExchangeInstID + "\", \""
			+ trade.OffsetFlag + "\", \""
			+ trade.HedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(trade.Price) + "\", \""
			+ boost::lexical_cast<std::string>(trade.Volume) + "\", \""
			+ trade.TradeDate + "\", \""
			+ trade.TradeTime + "\", \""
			+ trade.TradeType + "\", \""
			+ trade.PriceSource + "\", \""
			+ trade.OrderLocalID + "\", \""
			+ trade.ClearingPartID + "\", \""
			+ trade.BusinessUnit + "\", \""
			+ boost::lexical_cast<std::string>(trade.SequenceNo) + "\", \""
			+ trade.TradingDay + "\", \""
			+ boost::lexical_cast<std::string>(trade.SettlementID) + "\", \""
			+ boost::lexical_cast<std::string>(trade.BrokerOrderSeq) + "\", \""
			+ trade.TradeSource + "\", \""
			+ boost::lexical_cast<std::string>(trade.CloseProfit) + "\", \""
			+ boost::lexical_cast<std::string>(trade.Commission) + "\", \""
			+ "0\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetHisTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISTRADE_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all histrades of " << strTradingDay << "and investor:" << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetHisTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID, string strBeginDate, string strEndDate)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISTRADE_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = " + strInvestorID;
		if(strBeginDate.empty() && !strEndDate.empty())
			strSql = strSql + "' AND TradingDay <= " + strEndDate;
		else if(!strBeginDate.empty() && strEndDate.empty())
			strSql = strSql + "' AND TradingDay >= " + strBeginDate;
		else if(!strBeginDate.empty() && !strEndDate.empty())
			strSql = strSql + "' AND TradingDay BETWEEN " + strBeginDate + " AND " + strEndDate;
		strSql = strSql + ";";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all histrades of [" << strBeginDate << ", " << strEndDate << "] and investor:" << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllHisTrade(vector<TradeField> &tradesVec, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISTRADE_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

//		if(recordCount == 0)
//		{
//			INFO << "No data!" << std::endl;
//			return false;
//		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		TradeField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			strcpy(field.OrderRef, recordSet[row][3].c_str());
			strcpy(field.UserID, recordSet[row][4].c_str());
			strcpy(field.ExchangeID, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.Direction = recordSet[row][7][0];
			strcpy(field.OrderSysID, recordSet[row][8].c_str());
			strcpy(field.ParticipantID, recordSet[row][9].c_str());
			strcpy(field.ClientID, recordSet[row][10].c_str());
			field.TradingRole = recordSet[row][11][0];
			strcpy(field.ExchangeInstID, recordSet[row][12].c_str());
			field.OffsetFlag = recordSet[row][13][0];
			field.HedgeFlag = recordSet[row][14][0];
			field.Price = toDouble(recordSet[row][15]);
			field.Volume = toInt(recordSet[row][16]);
			strcpy(field.TradeDate, recordSet[row][17].c_str());
			strcpy(field.TradeTime, recordSet[row][18].c_str());
			field.TradeType = recordSet[row][19][0];
			field.PriceSource = recordSet[row][20][0];
			strcpy(field.OrderLocalID, recordSet[row][21].c_str());
			strcpy(field.ClearingPartID, recordSet[row][22].c_str());
			strcpy(field.BusinessUnit, recordSet[row][23].c_str());
			field.SequenceNo = toInt(recordSet[row][24]);
			strcpy(field.TradingDay, recordSet[row][25].c_str());
			field.SettlementID = toInt(recordSet[row][26]);
			field.BrokerOrderSeq = toInt(recordSet[row][27]);
			field.TradeSource = recordSet[row][28][0];
			field.CloseProfit = toDouble(recordSet[row][29]);
			field.Commission = toDouble(recordSet[row][30]);

			tradesVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all histrades of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorHisTrade(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + HISTRADE_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 资金信息
bool CDBOperation::UpdTradingAccount(TradingAccountField &field)
{
	try
	{
		string strSql = (string)"REPLACE INTO `" + TRADINGACCOUNT_TABLE+ "` VALUES ( \"" \
				+ field.BrokerID + "\", \""
				+ field.AccountID + "\", \""
				+ boost::lexical_cast<std::string>(field.PreMortgage) + "\", \""
				+ boost::lexical_cast<std::string>(field.PreCredit) + "\", \""
				+ boost::lexical_cast<std::string>(field.PreDeposit) + "\", \""
				+ boost::lexical_cast<std::string>(field.PreBalance) + "\", \""
				+ boost::lexical_cast<std::string>(field.PreMargin) + "\", \""
				+ boost::lexical_cast<std::string>(field.InterestBase) + "\", \""
				+ boost::lexical_cast<std::string>(field.Interest) + "\", \""
				+ boost::lexical_cast<std::string>(field.Deposit) + "\", \""
				+ boost::lexical_cast<std::string>(field.Withdraw) + "\", \""
				+ boost::lexical_cast<std::string>(field.FrozenMargin) + "\", \""
				+ boost::lexical_cast<std::string>(field.FrozenCash) + "\", \""
				+ boost::lexical_cast<std::string>(field.FrozenCommission) + "\", \""
				+ boost::lexical_cast<std::string>(field.CurrMargin) + "\", \""
				+ boost::lexical_cast<std::string>(field.CashIn) + "\", \""
				+ boost::lexical_cast<std::string>(field.Commission) + "\", \""
				+ boost::lexical_cast<std::string>(field.CloseProfit) + "\", \""
				+ boost::lexical_cast<std::string>(field.PositionProfit) + "\", \""
				+ boost::lexical_cast<std::string>(field.Balance) + "\", \""
				+ boost::lexical_cast<std::string>(field.Available) + "\", \""
				+ boost::lexical_cast<std::string>(field.WithdrawQuota) + "\", \""
				+ boost::lexical_cast<std::string>(field.Reserve) + "\", \""
				+ boost::lexical_cast<std::string>(field.TradingDay) + "\", \""
				+ boost::lexical_cast<std::string>(field.SettlementID) + "\", \""
				+ boost::lexical_cast<std::string>(field.Credit) + "\", \""
				+ boost::lexical_cast<std::string>(field.Mortgage) + "\", \""
				+ boost::lexical_cast<std::string>(field.ExchangeMargin) + "\", \""
				+ boost::lexical_cast<std::string>(field.DeliveryMargin) + "\", \""
				+ boost::lexical_cast<std::string>(field.ExchangeDeliveryMargin)
				+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
	}
	catch(...)
	{
		ERROR << "update tradingaccount of " << field.BrokerID << "-" << field.AccountID << " faild !" << std::endl;
		return false;
	}
}
int CDBOperation::GetTradingAccount(TradingAccountField &field, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADINGACCOUNT_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}

		strcpy(field.BrokerID, recordSet[0][0].c_str());
		strcpy(field.AccountID, recordSet[0][1].c_str());
		field.PreMortgage = toDouble(recordSet[0][2]);
		field.PreCredit = toDouble(recordSet[0][3]);
		field.PreDeposit = toDouble(recordSet[0][4]);
		field.PreBalance = toDouble(recordSet[0][5]);
		field.PreMargin = toDouble(recordSet[0][6]);
		field.InterestBase = toDouble(recordSet[0][7]);
		field.Interest = toDouble(recordSet[0][8]);
		field.Deposit = toDouble(recordSet[0][9]);
		field.Withdraw = toDouble(recordSet[0][10]);
		field.FrozenMargin = toDouble(recordSet[0][11]);
		field.FrozenCash = toDouble(recordSet[0][12]);
		field.FrozenCommission = toDouble(recordSet[0][13]);
		field.CurrMargin = toDouble(recordSet[0][14]);
		field.CashIn = toDouble(recordSet[0][15]);
		field.Commission = toDouble(recordSet[0][16]);
		field.CloseProfit = toDouble(recordSet[0][17]);
		field.PositionProfit = toDouble(recordSet[0][18]);
		field.Balance = toDouble(recordSet[0][19]);
		field.Available = toDouble(recordSet[0][20]);
		field.WithdrawQuota = toDouble(recordSet[0][21]);
		field.Reserve = toDouble(recordSet[0][22]);
		strcpy(field.TradingDay, recordSet[0][23].c_str());
		field.SettlementID = toInt(recordSet[0][24]);
		field.Credit = toDouble(recordSet[0][25]);
		field.Mortgage = toDouble(recordSet[0][26]);
		field.ExchangeMargin = toDouble(recordSet[0][27]);
		field.DeliveryMargin = toDouble(recordSet[0][28]);
		field.ExchangeDeliveryMargin = toDouble(recordSet[0][29]);

	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry tradingaccount of " << strBrokerID << "-" << strInvestorID << " faild !" << std::endl;
		return -1;
	}
}
bool CDBOperation::DelInvestorTradingAccount(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + TRADINGACCOUNT_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 持仓信息
bool CDBOperation::UpdPosition(InvestorPositionField &position)
{
	string strSql = (string)"REPLACE INTO `" + POSITION_TABLE + "` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(position.TradingDay) + "\", \""
			+ position.BrokerID + "\", \""
			+ position.InvestorID + "\", \""
			+ position.InstrumentID + "\", \""
			+ position.PosiDirection + "\", \""
			+ position.HedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(position.AvgPositionPrice) + "\", \""
			+ position.PositionDate + "\", \""
			+ boost::lexical_cast<std::string>(position.YdPosition) + "\", \""
			+ boost::lexical_cast<std::string>(position.Position) + "\", \""
			+ boost::lexical_cast<std::string>(position.LongFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.ShortFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.LongFrozenAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.ShortFrozenAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.OpenVolume) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseVolume) + "\", \""
			+ boost::lexical_cast<std::string>(position.OpenAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.PositionCost) + "\", \""
			+ boost::lexical_cast<std::string>(position.PreMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.UseMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.FrozenMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.FrozenCash) + "\", \""
			+ boost::lexical_cast<std::string>(position.FrozenCommission) + "\", \""
			+ boost::lexical_cast<std::string>(position.CashIn) + "\", \""
			+ boost::lexical_cast<std::string>(position.Commission) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseProfit) + "\", \""
			+ boost::lexical_cast<std::string>(position.PositionProfit) + "\", \""
			+ boost::lexical_cast<std::string>(position.PreSettlementPrice) + "\", \""
			+ boost::lexical_cast<std::string>(position.SettlementPrice) + "\", \""
			+ boost::lexical_cast<std::string>(position.SettlementID) + "\", \""
			+ boost::lexical_cast<std::string>(position.OpenCost) + "\", \""
			+ boost::lexical_cast<std::string>(position.ExchangeMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.CombPosition) + "\", \""
			+ boost::lexical_cast<std::string>(position.CombLongFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.CombShortFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseProfitByDate) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseProfitByTrade) + "\", \""
			+ boost::lexical_cast<std::string>(position.TodayPosition) + "\", \""
			+ boost::lexical_cast<std::string>(position.MarginRateByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(position.MarginRateByVolume) + "\", \""
			+ boost::lexical_cast<std::string>(position.ClosableNum) + "\", \""
			+ boost::lexical_cast<std::string>(position.TodayClosableNum)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelPosition(InvestorPositionField &position)
{
	string strSql = (string)"DELETE FROM `" + POSITION_TABLE + "` WHERE TradingDay = \'" + boost::lexical_cast<std::string>(position.TradingDay)
				+ "' AND BrokerID = '" + position.BrokerID + "' AND InvestorID = '" + position.InvestorID
				+ "' AND InstrumentID = '" + position.InstrumentID + "' AND PosiDirection = '" + position.PosiDirection
				+ "' AND HedgeFlag = '" + position.HedgeFlag + "\';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelZeroPositions()
{
	string strSql = (string)"DELETE FROM `" + POSITION_TABLE + "` WHERE Position = '0';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::GetPositions(vector<InvestorPositionField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITION_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.TradingDay, recordSet[row][0].c_str());
			strcpy(field.BrokerID, recordSet[row][1].c_str());
			strcpy(field.InvestorID, recordSet[row][2].c_str());
			strcpy(field.InstrumentID, recordSet[row][3].c_str());
			field.PosiDirection = recordSet[row][4][0];
			field.HedgeFlag = recordSet[row][5][0];
			field.AvgPositionPrice = toDouble(recordSet[row][6]);
			field.PositionDate = recordSet[row][7][0];
			field.YdPosition = toInt(recordSet[row][8]);
			field.Position = toInt(recordSet[row][9]);
			field.LongFrozen = toInt(recordSet[row][10]);
			field.ShortFrozen = toInt(recordSet[row][11]);
			field.LongFrozenAmount = toDouble(recordSet[row][12]);
			field.ShortFrozenAmount = toDouble(recordSet[row][13]);
			field.OpenVolume = toInt(recordSet[row][14]);
			field.CloseVolume = toInt(recordSet[row][15]);
			field.OpenAmount = toDouble(recordSet[row][16]);
			field.CloseAmount = toDouble(recordSet[row][17]);
			field.PositionCost = toDouble(recordSet[row][18]);
			field.PreMargin = toDouble(recordSet[row][19]);
			field.UseMargin = toDouble(recordSet[row][20]);
			field.FrozenMargin = toDouble(recordSet[row][21]);
			field.FrozenCash = toDouble(recordSet[row][22]);
			field.FrozenCommission = toDouble(recordSet[row][23]);
			field.CashIn = toDouble(recordSet[row][24]);
			field.Commission = toDouble(recordSet[row][25]);
			field.CloseProfit = toDouble(recordSet[row][26]);
			field.PositionProfit = toDouble(recordSet[row][27]);
			field.PreSettlementPrice = toDouble(recordSet[row][28]);
			field.SettlementPrice = toDouble(recordSet[row][29]);
			field.SettlementID = toInt(recordSet[row][30]);
			field.OpenCost = toDouble(recordSet[row][31]);
			field.ExchangeMargin = toDouble(recordSet[row][32]);
			field.CombPosition = toInt(recordSet[row][33]);
			field.CombLongFrozen = toInt(recordSet[row][34]);
			field.CombShortFrozen = toInt(recordSet[row][35]);
			field.CloseProfitByDate = toDouble(recordSet[row][36]);
			field.CloseProfitByTrade = toDouble(recordSet[row][37]);
			field.TodayPosition = toInt(recordSet[row][38]);
			field.MarginRateByMoney = toDouble(recordSet[row][39]);
			field.MarginRateByVolume = toDouble(recordSet[row][40]);
			field.ClosableNum = toInt(recordSet[row][41]);
			field.TodayClosableNum = toInt(recordSet[row][42]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry positions of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllPositions(vector<InvestorPositionField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITION_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.TradingDay, recordSet[row][0].c_str());
			strcpy(field.BrokerID, recordSet[row][1].c_str());
			strcpy(field.InvestorID, recordSet[row][2].c_str());
			strcpy(field.InstrumentID, recordSet[row][3].c_str());
			field.PosiDirection = recordSet[row][4][0];
			field.HedgeFlag = recordSet[row][5][0];
			field.AvgPositionPrice = toDouble(recordSet[row][6]);
			field.PositionDate = recordSet[row][7][0];
			field.YdPosition = toInt(recordSet[row][8]);
			field.Position = toInt(recordSet[row][9]);
			field.LongFrozen = toInt(recordSet[row][10]);
			field.ShortFrozen = toInt(recordSet[row][11]);
			field.LongFrozenAmount = toDouble(recordSet[row][12]);
			field.ShortFrozenAmount = toDouble(recordSet[row][13]);
			field.OpenVolume = toInt(recordSet[row][14]);
			field.CloseVolume = toInt(recordSet[row][15]);
			field.OpenAmount = toDouble(recordSet[row][16]);
			field.CloseAmount = toDouble(recordSet[row][17]);
			field.PositionCost = toDouble(recordSet[row][18]);
			field.PreMargin = toDouble(recordSet[row][19]);
			field.UseMargin = toDouble(recordSet[row][20]);
			field.FrozenMargin = toDouble(recordSet[row][21]);
			field.FrozenCash = toDouble(recordSet[row][22]);
			field.FrozenCommission = toDouble(recordSet[row][23]);
			field.CashIn = toDouble(recordSet[row][24]);
			field.Commission = toDouble(recordSet[row][25]);
			field.CloseProfit = toDouble(recordSet[row][26]);
			field.PositionProfit = toDouble(recordSet[row][27]);
			field.PreSettlementPrice = toDouble(recordSet[row][28]);
			field.SettlementPrice = toDouble(recordSet[row][29]);
			field.SettlementID = toInt(recordSet[row][30]);
			field.OpenCost = toDouble(recordSet[row][31]);
			field.ExchangeMargin = toDouble(recordSet[row][32]);
			field.CombPosition = toInt(recordSet[row][33]);
			field.CombLongFrozen = toInt(recordSet[row][34]);
			field.CombShortFrozen = toInt(recordSet[row][35]);
			field.CloseProfitByDate = toDouble(recordSet[row][36]);
			field.CloseProfitByTrade = toDouble(recordSet[row][37]);
			field.TodayPosition = toInt(recordSet[row][38]);
			field.MarginRateByMoney = toDouble(recordSet[row][39]);
			field.MarginRateByVolume = toDouble(recordSet[row][40]);
			field.ClosableNum = toInt(recordSet[row][41]);
			field.TodayClosableNum = toInt(recordSet[row][42]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all positions faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetPosition(InvestorPositionField &field, string strBrokerID, string strInvestorID, string strInstrumentID, string strPosiDirection)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITION_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND InstrumentID = '" + strInstrumentID+ "' AND PosiDirection = '" + strPosiDirection  + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		strcpy(field.TradingDay, recordSet[0][0].c_str());
		strcpy(field.BrokerID, recordSet[0][1].c_str());
		strcpy(field.InvestorID, recordSet[0][2].c_str());
		strcpy(field.InstrumentID, recordSet[0][3].c_str());
		field.PosiDirection = recordSet[0][4][0];
		field.HedgeFlag = recordSet[0][5][0];
		field.AvgPositionPrice = toDouble(recordSet[0][6]);
		field.PositionDate = recordSet[0][7][0];
		field.YdPosition = toInt(recordSet[0][8]);
		field.Position = toInt(recordSet[0][9]);
		field.LongFrozen = toInt(recordSet[0][10]);
		field.ShortFrozen = toInt(recordSet[0][11]);
		field.LongFrozenAmount = toDouble(recordSet[0][12]);
		field.ShortFrozenAmount = toDouble(recordSet[0][13]);
		field.OpenVolume = toInt(recordSet[0][14]);
		field.CloseVolume = toInt(recordSet[0][15]);
		field.OpenAmount = toDouble(recordSet[0][16]);
		field.CloseAmount = toDouble(recordSet[0][17]);
		field.PositionCost = toDouble(recordSet[0][18]);
		field.PreMargin = toDouble(recordSet[0][19]);
		field.UseMargin = toDouble(recordSet[0][20]);
		field.FrozenMargin = toDouble(recordSet[0][21]);
		field.FrozenCash = toDouble(recordSet[0][22]);
		field.FrozenCommission = toDouble(recordSet[0][23]);
		field.CashIn = toDouble(recordSet[0][24]);
		field.Commission = toDouble(recordSet[0][25]);
		field.CloseProfit = toDouble(recordSet[0][26]);
		field.PositionProfit = toDouble(recordSet[0][27]);
		field.PreSettlementPrice = toDouble(recordSet[0][28]);
		field.SettlementPrice = toDouble(recordSet[0][29]);
		field.SettlementID = toInt(recordSet[0][30]);
		field.OpenCost = toDouble(recordSet[0][31]);
		field.ExchangeMargin = toDouble(recordSet[0][32]);
		field.CombPosition = toInt(recordSet[0][33]);
		field.CombLongFrozen = toInt(recordSet[0][34]);
		field.CombShortFrozen = toInt(recordSet[0][35]);
		field.CloseProfitByDate = toDouble(recordSet[0][36]);
		field.CloseProfitByTrade = toDouble(recordSet[0][37]);
		field.TodayPosition = toInt(recordSet[0][38]);
		field.MarginRateByMoney = toDouble(recordSet[0][39]);
		field.MarginRateByVolume = toDouble(recordSet[0][40]);
		field.ClosableNum = toInt(recordSet[0][41]);
		field.TodayClosableNum = toInt(recordSet[0][42]);

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry position of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetPosition(vector<InvestorPositionField> &fieldsVec, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITION_TABLE;
		strSql = strSql + "` WHERE InstrumentID = '" + strInstrumentID+ "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.TradingDay, recordSet[row][0].c_str());
			strcpy(field.BrokerID, recordSet[row][1].c_str());
			strcpy(field.InvestorID, recordSet[row][2].c_str());
			strcpy(field.InstrumentID, recordSet[row][3].c_str());
			field.PosiDirection = recordSet[row][4][0];
			field.HedgeFlag = recordSet[row][5][0];
			field.AvgPositionPrice = toDouble(recordSet[row][6]);
			field.PositionDate = recordSet[row][7][0];
			field.YdPosition = toInt(recordSet[row][8]);
			field.Position = toInt(recordSet[row][9]);
			field.LongFrozen = toInt(recordSet[row][10]);
			field.ShortFrozen = toInt(recordSet[row][11]);
			field.LongFrozenAmount = toDouble(recordSet[row][12]);
			field.ShortFrozenAmount = toDouble(recordSet[row][13]);
			field.OpenVolume = toInt(recordSet[row][14]);
			field.CloseVolume = toInt(recordSet[row][15]);
			field.OpenAmount = toDouble(recordSet[row][16]);
			field.CloseAmount = toDouble(recordSet[row][17]);
			field.PositionCost = toDouble(recordSet[row][18]);
			field.PreMargin = toDouble(recordSet[row][19]);
			field.UseMargin = toDouble(recordSet[row][20]);
			field.FrozenMargin = toDouble(recordSet[row][21]);
			field.FrozenCash = toDouble(recordSet[row][22]);
			field.FrozenCommission = toDouble(recordSet[row][23]);
			field.CashIn = toDouble(recordSet[row][24]);
			field.Commission = toDouble(recordSet[row][25]);
			field.CloseProfit = toDouble(recordSet[row][26]);
			field.PositionProfit = toDouble(recordSet[row][27]);
			field.PreSettlementPrice = toDouble(recordSet[row][28]);
			field.SettlementPrice = toDouble(recordSet[row][29]);
			field.SettlementID = toInt(recordSet[row][30]);
			field.OpenCost = toDouble(recordSet[row][31]);
			field.ExchangeMargin = toDouble(recordSet[row][32]);
			field.CombPosition = toInt(recordSet[row][33]);
			field.CombLongFrozen = toInt(recordSet[row][34]);
			field.CombShortFrozen = toInt(recordSet[row][35]);
			field.CloseProfitByDate = toDouble(recordSet[row][36]);
			field.CloseProfitByTrade = toDouble(recordSet[row][37]);
			field.TodayPosition = toInt(recordSet[row][38]);
			field.MarginRateByMoney = toDouble(recordSet[row][39]);
			field.MarginRateByVolume = toDouble(recordSet[row][40]);
			field.ClosableNum = toInt(recordSet[row][41]);
			field.TodayClosableNum = toInt(recordSet[row][42]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry position of " << strInstrumentID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorPosition(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + POSITION_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 云条件单信息
bool CDBOperation::UpdCloudOrder(CloudOrderField &field)
{
	string strCloudOrderID = (field.CloudOrderID <= 0) ? "0" : boost::lexical_cast<std::string>(field.CloudOrderID);
	string strSql = (string)"REPLACE INTO `" + CLOUDORDER_TABLE+ "` VALUES ( \"" \
			+ strCloudOrderID + "\", \""
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.ExchangeID + "\", \""
			+ field.InstrumentID + "\", \""
			+ field.Direction + "\", \""
			+ field.OffsetFlag + "\", \""
			+ field.HedgeFlag + "\", \""
			+ field.Type + "\", \""
			+ field.TriggeringTime + "\", \""
			+ field.ValidityPeriodType + "\", \""
			+ field.InsertDate + "\", \""
			+ field.InsertTime + "\", \""
			+ boost::lexical_cast<std::string>(field.Price) + "\", \""
			+ boost::lexical_cast<std::string>(field.TotalVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.TradedVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.FrozenVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.Status) + "\", \""
			+ field.StatusMsg + "\", \""
			+ field.PriceType + "\", \""
			+ field.TriggeringPriceType + "\", \""
			+ boost::lexical_cast<std::string>(field.MinPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.MaxPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.LeadOrderInsertTick) + "\", \""
			+ boost::lexical_cast<std::string>(field.Step) + "\", \""
			+ boost::lexical_cast<std::string>(field.MaxWinningPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopWinningTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopLossingTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopWinningPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopLossingPrice) + "\", \""
			+ field.TriggeringDate
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		if(field.CloudOrderID == 0)
		{
			strSql = (string)"SELECT MAX(CloudOrderID) FROM `" + CLOUDORDER_TABLE + "`;";
			DEBUG << strSql << std::endl;
			CRecordSet recordSet(m_DataBase.GetMysql());
			int recordCount = recordSet.ExecuteSQL(strSql.c_str());

			if(recordCount == 0)
			{
	//			INFO << "No data!" << std::endl;
				return false;
			}
			if(recordCount < 0)
			{
				ERROR << "Execute faild!" << std::endl;
				return false;
			}
			field.CloudOrderID = toInt(recordSet[0][0]);
		}
		return true;
}
bool CDBOperation::GetCloudOrder(vector<CloudOrderField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + CLOUDORDER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		CloudOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CloudOrderID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.ExchangeID = recordSet[row][3];
			field.InstrumentID = recordSet[row][4];
			field.Direction = recordSet[row][5];
			field.OffsetFlag = recordSet[row][6];
			field.HedgeFlag = recordSet[row][7];
			field.Type = recordSet[row][8];
			field.TriggeringTime = recordSet[row][9];
			field.ValidityPeriodType = recordSet[row][10];
			field.InsertDate = recordSet[row][11];
			field.InsertTime = recordSet[row][12];
			field.Price = toDouble(recordSet[row][13]);
			field.TotalVolume = toInt(recordSet[row][14]);
			field.TradedVolume = toInt(recordSet[row][15]);
			field.FrozenVolume = toInt(recordSet[row][16]);
			field.Status = toInt(recordSet[row][17]);
			field.StatusMsg = recordSet[row][18];
			field.PriceType = recordSet[row][19];
			field.TriggeringPriceType = recordSet[row][20];
			field.MinPrice = toDouble(recordSet[row][21]);
			field.MaxPrice = toDouble(recordSet[row][22]);
			field.LeadOrderInsertTick = toInt(recordSet[row][23]);
			field.Step = toInt(recordSet[row][24]);
			field.MaxWinningPrice = toDouble(recordSet[row][25]);
			field.StopWinningTickNum = toInt(recordSet[row][26]);
			field.StopLossingTickNum = toInt(recordSet[row][27]);
			field.StopWinningPrice = toDouble(recordSet[row][28]);
			field.StopLossingPrice = toDouble(recordSet[row][29]);
			field.TriggeringDate = recordSet[row][30];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry cloud orders of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllCloudOrder(vector<CloudOrderField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + CLOUDORDER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		CloudOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CloudOrderID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.ExchangeID = recordSet[row][3];
			field.InstrumentID = recordSet[row][4];
			field.Direction = recordSet[row][5];
			field.OffsetFlag = recordSet[row][6];
			field.HedgeFlag = recordSet[row][7];
			field.Type = recordSet[row][8];
			field.TriggeringTime = recordSet[row][9];
			field.ValidityPeriodType = recordSet[row][10];
			field.InsertDate = recordSet[row][11];
			field.InsertTime = recordSet[row][12];
			field.Price = toDouble(recordSet[row][13]);
			field.TotalVolume = toInt(recordSet[row][14]);
			field.TradedVolume = toInt(recordSet[row][15]);
			field.FrozenVolume = toInt(recordSet[row][16]);
			field.Status = toInt(recordSet[row][17]);
			field.StatusMsg = recordSet[row][18];
			field.PriceType = recordSet[row][19];
			field.TriggeringPriceType = recordSet[row][20];
			field.MinPrice = toDouble(recordSet[row][21]);
			field.MaxPrice = toDouble(recordSet[row][22]);
			field.LeadOrderInsertTick = toInt(recordSet[row][23]);
			field.Step = toInt(recordSet[row][24]);
			field.MaxWinningPrice = toDouble(recordSet[row][25]);
			field.StopWinningTickNum = toInt(recordSet[row][26]);
			field.StopLossingTickNum = toInt(recordSet[row][27]);
			field.StopWinningPrice = toDouble(recordSet[row][28]);
			field.StopLossingPrice = toDouble(recordSet[row][29]);
			field.TriggeringDate = recordSet[row][30];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all cloud orders faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetInvalidCloudOrder(vector<CloudOrderField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + CLOUDORDER_TABLE;
		strSql = strSql + "` WHERE status <> '0';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		CloudOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CloudOrderID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.ExchangeID = recordSet[row][3];
			field.InstrumentID = recordSet[row][4];
			field.Direction = recordSet[row][5];
			field.OffsetFlag = recordSet[row][6];
			field.HedgeFlag = recordSet[row][7];
			field.Type = recordSet[row][8];
			field.TriggeringTime = recordSet[row][9];
			field.ValidityPeriodType = recordSet[row][10];
			field.InsertDate = recordSet[row][11];
			field.InsertTime = recordSet[row][12];
			field.Price = toDouble(recordSet[row][13]);
			field.TotalVolume = toInt(recordSet[row][14]);
			field.TradedVolume = toInt(recordSet[row][15]);
			field.FrozenVolume = toInt(recordSet[row][16]);
			field.Status = toInt(recordSet[row][17]);
			field.StatusMsg = recordSet[row][18];
			field.PriceType = recordSet[row][19];
			field.TriggeringPriceType = recordSet[row][20];
			field.MinPrice = toDouble(recordSet[row][21]);
			field.MaxPrice = toDouble(recordSet[row][22]);
			field.LeadOrderInsertTick = toInt(recordSet[row][23]);
			field.Step = toInt(recordSet[row][24]);
			field.MaxWinningPrice = toDouble(recordSet[row][25]);
			field.StopWinningTickNum = toInt(recordSet[row][26]);
			field.StopLossingTickNum = toInt(recordSet[row][27]);
			field.StopWinningPrice = toDouble(recordSet[row][28]);
			field.StopLossingPrice = toDouble(recordSet[row][29]);
			field.TriggeringDate = recordSet[row][30];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry invalid cloud orders faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelCloudOrder(string strInstrumentID, string strStatus)
{
	string strSql = (string)"DELETE FROM `" + CLOUDORDER_TABLE + "` WHERE InstrumentID = '" + strInstrumentID + "' AND status = '" + strStatus + "\';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelInvestorCloudOrder(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + CLOUDORDER_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelAllInvalidCloudOrder()
{
	string strSql = (string)"DELETE FROM `" + CLOUDORDER_TABLE + "` WHERE status <> '0';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 历史云条件单信息
bool CDBOperation::UpdHisCloudOrder(CloudOrderField &field)
{
	string strCloudOrderID = (field.CloudOrderID <= 0) ? "0" : boost::lexical_cast<std::string>(field.CloudOrderID);
	string strSql = (string)"REPLACE INTO `" + HISCLOUDORDER_TABLE+ "` VALUES ( \"" \
			+ strCloudOrderID + "\", \""
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.ExchangeID + "\", \""
			+ field.InstrumentID + "\", \""
			+ field.Direction + "\", \""
			+ field.OffsetFlag + "\", \""
			+ field.HedgeFlag + "\", \""
			+ field.Type + "\", \""
			+ field.TriggeringTime + "\", \""
			+ field.ValidityPeriodType + "\", \""
			+ field.InsertDate + "\", \""
			+ field.InsertTime + "\", \""
			+ boost::lexical_cast<std::string>(field.Price) + "\", \""
			+ boost::lexical_cast<std::string>(field.TotalVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.TradedVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.FrozenVolume) + "\", \""
			+ boost::lexical_cast<std::string>(field.Status) + "\", \""
			+ field.StatusMsg + "\", \""
			+ field.PriceType + "\", \""
			+ field.TriggeringPriceType + "\", \""
			+ boost::lexical_cast<std::string>(field.MinPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.MaxPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.LeadOrderInsertTick) + "\", \""
			+ boost::lexical_cast<std::string>(field.Step) + "\", \""
			+ boost::lexical_cast<std::string>(field.MaxWinningPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopWinningTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopLossingTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopWinningPrice) + "\", \""
			+ boost::lexical_cast<std::string>(field.StopLossingPrice) + "\", \""
			+ field.TriggeringDate
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		if(field.CloudOrderID == 0)
		{
			strSql = (string)"SELECT MAX(CloudOrderID) FROM `" + HISCLOUDORDER_TABLE + "`;";
			DEBUG << strSql << std::endl;
			CRecordSet recordSet(m_DataBase.GetMysql());
			int recordCount = recordSet.ExecuteSQL(strSql.c_str());

			if(recordCount == 0)
			{
	//			INFO << "No data!" << std::endl;
				return false;
			}
			if(recordCount < 0)
			{
				ERROR << "Execute faild!" << std::endl;
				return false;
			}
			field.CloudOrderID = toInt(recordSet[0][0]);
		}
		return true;
}
bool CDBOperation::GetHisCloudOrder(vector<CloudOrderField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISCLOUDORDER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
//			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		CloudOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CloudOrderID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.ExchangeID = recordSet[row][3];
			field.InstrumentID = recordSet[row][4];
			field.Direction = recordSet[row][5];
			field.OffsetFlag = recordSet[row][6];
			field.HedgeFlag = recordSet[row][7];
			field.Type = recordSet[row][8];
			field.TriggeringTime = recordSet[row][9];
			field.ValidityPeriodType = recordSet[row][10];
			field.InsertDate = recordSet[row][11];
			field.InsertTime = recordSet[row][12];
			field.Price = toDouble(recordSet[row][13]);
			field.TotalVolume = toInt(recordSet[row][14]);
			field.TradedVolume = toInt(recordSet[row][15]);
			field.FrozenVolume = toInt(recordSet[row][16]);
			field.Status = toInt(recordSet[row][17]);
			field.StatusMsg = recordSet[row][18];
			field.PriceType = recordSet[row][19];
			field.TriggeringPriceType = recordSet[row][20];
			field.MinPrice = toDouble(recordSet[row][21]);
			field.MaxPrice = toDouble(recordSet[row][22]);
			field.LeadOrderInsertTick = toInt(recordSet[row][23]);
			field.Step = toInt(recordSet[row][24]);
			field.MaxWinningPrice = toDouble(recordSet[row][25]);
			field.StopWinningTickNum = toInt(recordSet[row][26]);
			field.StopLossingTickNum = toInt(recordSet[row][27]);
			field.StopWinningPrice = toDouble(recordSet[row][28]);
			field.StopLossingPrice = toDouble(recordSet[row][29]);
			field.TriggeringDate = recordSet[row][30];

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry cloud orders of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}

// 出入金流水
bool CDBOperation::UpdCashFlow(CashFlowField &field)
{
	string strSql = (string)"REPLACE INTO `" + CASHFLOW_TABLE+ "` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(field.CashFlowID) + "\", \""
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.CurrencyCode + "\", \""
			+ field.Type + "\", \""
			+ boost::lexical_cast<std::string>(field.Amount) + "\", \""
			+ field.Date + "\", \""
			+ field.Time + "\", \""
			+ field.OperatorCode + "\", \""
			+ boost::lexical_cast<std::string>(field.PreCapital) + "\", \""
			+ boost::lexical_cast<std::string>(field.Capital) + "\", \""
			+ field.Comment + "\", \""
			+ boost::lexical_cast<std::string>(field.IsHandled)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetUnHandleCashFlow(vector<CashFlowField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + CASHFLOW_TABLE;
		strSql = strSql + "` WHERE IsHandled = '0';";
//		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		CashFlowField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CashFlowID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.CurrencyCode = recordSet[row][3];
			field.Type = recordSet[row][4];
			field.Amount = toDouble(recordSet[row][5]);
			field.Date = recordSet[row][6];
			field.Time = recordSet[row][7];
			field.OperatorCode = recordSet[row][8];
			field.PreCapital = toDouble(recordSet[row][9]);
			field.Capital = toDouble(recordSet[row][10]);
			field.Comment = recordSet[row][11];
			field.IsHandled = toInt(recordSet[row][12]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry unhandle cash flow faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetCashFlow(vector<CashFlowField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + CASHFLOW_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		CashFlowField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CashFlowID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.CurrencyCode = recordSet[row][3];
			field.Type = recordSet[row][4];
			field.Amount = toDouble(recordSet[row][5]);
			field.Date = recordSet[row][6];
			field.Time = recordSet[row][7];
			field.OperatorCode = recordSet[row][8];
			field.PreCapital = toDouble(recordSet[row][9]);
			field.Capital = toDouble(recordSet[row][10]);
			field.Comment = recordSet[row][11];
			field.IsHandled = toInt(recordSet[row][12]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry cash flows of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllCashFlow(vector<CashFlowField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + CASHFLOW_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		CashFlowField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CashFlowID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.CurrencyCode = recordSet[row][3];
			field.Type = recordSet[row][4];
			field.Amount = toDouble(recordSet[row][5]);
			field.Date = recordSet[row][6];
			field.Time = recordSet[row][7];
			field.OperatorCode = recordSet[row][8];
			field.PreCapital = toDouble(recordSet[row][9]);
			field.Capital = toDouble(recordSet[row][10]);
			field.Comment = recordSet[row][11];
			field.IsHandled = toInt(recordSet[row][12]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all cash flows faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelCashFlow(CashFlowField &field)
{
	string strSql = (string)"DELETE FROM `" + CASHFLOW_TABLE + "` WHERE CashFlowID = \'" + boost::lexical_cast<std::string>(field.CashFlowID) + "\';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelInvestorCashFlow(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + CASHFLOW_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 历史出入金流水
bool CDBOperation::UpdHisCashFlow(CashFlowField &field)
{
	string strSql = (string)"REPLACE INTO `" + HISCASHFLOW_TABLE+ "` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(field.CashFlowID) + "\", \""
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.CurrencyCode + "\", \""
			+ field.Type + "\", \""
			+ boost::lexical_cast<std::string>(field.Amount) + "\", \""
			+ field.Date + "\", \""
			+ field.Time + "\", \""
			+ field.OperatorCode + "\", \""
			+ boost::lexical_cast<std::string>(field.PreCapital) + "\", \""
			+ boost::lexical_cast<std::string>(field.Capital) + "\", \""
			+ field.Comment + "\", \""
			+ boost::lexical_cast<std::string>(field.IsHandled)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetUnHandleHisCashFlow(vector<CashFlowField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISCASHFLOW_TABLE;
		strSql = strSql + "` WHERE IsHandled = '0';";
//		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		CashFlowField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CashFlowID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.CurrencyCode = recordSet[row][3];
			field.Type = recordSet[row][4];
			field.Amount = toDouble(recordSet[row][5]);
			field.Date = recordSet[row][6];
			field.Time = recordSet[row][7];
			field.OperatorCode = recordSet[row][8];
			field.PreCapital = toDouble(recordSet[row][9]);
			field.Capital = toDouble(recordSet[row][10]);
			field.Comment = recordSet[row][11];
			field.IsHandled = toInt(recordSet[row][12]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry unhandle hiscash flow faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetHisCashFlow(vector<CashFlowField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISCASHFLOW_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		CashFlowField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.CashFlowID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.InvestorID = recordSet[row][2];
			field.CurrencyCode = recordSet[row][3];
			field.Type = recordSet[row][4];
			field.Amount = toDouble(recordSet[row][5]);
			field.Date = recordSet[row][6];
			field.Time = recordSet[row][7];
			field.OperatorCode = recordSet[row][8];
			field.PreCapital = toDouble(recordSet[row][9]);
			field.Capital = toDouble(recordSet[row][10]);
			field.Comment = recordSet[row][11];
			field.IsHandled = toInt(recordSet[row][12]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry unhandle cash flow faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorHisCashFlow(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + HISCASHFLOW_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 结算资金信息
bool CDBOperation::UpdSettlementCash(TradingAccountField &field)
{
	string strSql = (string)"REPLACE INTO `" + INVESTORSETTLEMENTCASH_TABLE+ "` VALUES ( \"" \
			+ field.BrokerID + "\", \""
			+ field.AccountID + "\", \""
			+ boost::lexical_cast<std::string>(field.PreMortgage) + "\", \""
			+ boost::lexical_cast<std::string>(field.PreCredit) + "\", \""
			+ boost::lexical_cast<std::string>(field.PreDeposit) + "\", \""
			+ boost::lexical_cast<std::string>(field.PreBalance) + "\", \""
			+ boost::lexical_cast<std::string>(field.PreMargin) + "\", \""
			+ boost::lexical_cast<std::string>(field.InterestBase) + "\", \""
			+ boost::lexical_cast<std::string>(field.Interest) + "\", \""
			+ boost::lexical_cast<std::string>(field.Deposit) + "\", \""
			+ boost::lexical_cast<std::string>(field.Withdraw) + "\", \""
			+ boost::lexical_cast<std::string>(field.FrozenMargin) + "\", \""
			+ boost::lexical_cast<std::string>(field.FrozenCash) + "\", \""
			+ boost::lexical_cast<std::string>(field.FrozenCommission) + "\", \""
			+ boost::lexical_cast<std::string>(field.CurrMargin) + "\", \""
			+ boost::lexical_cast<std::string>(field.CashIn) + "\", \""
			+ boost::lexical_cast<std::string>(field.Commission) + "\", \""
			+ boost::lexical_cast<std::string>(field.CloseProfit) + "\", \""
			+ boost::lexical_cast<std::string>(field.PositionProfit) + "\", \""
			+ boost::lexical_cast<std::string>(field.Balance) + "\", \""
			+ boost::lexical_cast<std::string>(field.Available) + "\", \""
			+ boost::lexical_cast<std::string>(field.WithdrawQuota) + "\", \""
			+ boost::lexical_cast<std::string>(field.Reserve) + "\", \""
			+ boost::lexical_cast<std::string>(field.TradingDay) + "\", \""
			+ boost::lexical_cast<std::string>(field.SettlementID) + "\", \""
			+ boost::lexical_cast<std::string>(field.Credit) + "\", \""
			+ boost::lexical_cast<std::string>(field.Mortgage) + "\", \""
			+ boost::lexical_cast<std::string>(field.ExchangeMargin) + "\", \""
			+ boost::lexical_cast<std::string>(field.DeliveryMargin) + "\", \""
			+ boost::lexical_cast<std::string>(field.ExchangeDeliveryMargin)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
// return -1:faild; 0: no data; 1:true
int CDBOperation::GetSettlementCash(TradingAccountField &field, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVESTORSETTLEMENTCASH_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return 0;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}

		strcpy(field.BrokerID, recordSet[0][0].c_str());
		strcpy(field.AccountID, recordSet[0][1].c_str());
		field.PreMortgage = toDouble(recordSet[0][2]);
		field.PreCredit = toDouble(recordSet[0][3]);
		field.PreDeposit = toDouble(recordSet[0][4]);
		field.PreBalance = toDouble(recordSet[0][5]);
		field.PreMargin = toDouble(recordSet[0][6]);
		field.InterestBase = toDouble(recordSet[0][7]);
		field.Interest = toDouble(recordSet[0][8]);
		field.Deposit = toDouble(recordSet[0][9]);
		field.Withdraw = toDouble(recordSet[0][10]);
		field.FrozenMargin = toDouble(recordSet[0][11]);
		field.FrozenCash = toDouble(recordSet[0][12]);
		field.FrozenCommission = toDouble(recordSet[0][13]);
		field.CurrMargin = toDouble(recordSet[0][14]);
		field.CashIn = toDouble(recordSet[0][15]);
		field.Commission = toDouble(recordSet[0][16]);
		field.CloseProfit = toDouble(recordSet[0][17]);
		field.PositionProfit = toDouble(recordSet[0][18]);
		field.Balance = toDouble(recordSet[0][19]);
		field.Available = toDouble(recordSet[0][20]);
		field.WithdrawQuota = toDouble(recordSet[0][21]);
		field.Reserve = toDouble(recordSet[0][22]);
		strcpy(field.TradingDay, recordSet[0][23].c_str());
		field.SettlementID = toInt(recordSet[0][24]);
		field.Credit = toDouble(recordSet[0][25]);
		field.Mortgage = toDouble(recordSet[0][26]);
		field.ExchangeMargin = toDouble(recordSet[0][27]);
		field.DeliveryMargin = toDouble(recordSet[0][28]);
		field.ExchangeDeliveryMargin = toDouble(recordSet[0][29]);

	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry tradingaccount of " << strBrokerID << "-" << strInvestorID << " faild !" << std::endl;
		return -1;
	}
}
bool CDBOperation::DelInvestorSettlementCash(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + INVESTORSETTLEMENTCASH_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 历史持仓信息
bool CDBOperation::UpdHisPosition(InvestorPositionField &position)
{
	string strSql = (string)"REPLACE INTO `" + HISPOSITION_TABLE + "` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(position.TradingDay) + "\", \""
			+ position.BrokerID + "\", \""
			+ position.InvestorID + "\", \""
			+ position.InstrumentID + "\", \""
			+ position.PosiDirection + "\", \""
			+ position.HedgeFlag + "\", \""
			+ boost::lexical_cast<std::string>(position.AvgPositionPrice) + "\", \""
			+ position.PositionDate + "\", \""
			+ boost::lexical_cast<std::string>(position.YdPosition) + "\", \""
			+ boost::lexical_cast<std::string>(position.Position) + "\", \""
			+ boost::lexical_cast<std::string>(position.LongFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.ShortFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.LongFrozenAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.ShortFrozenAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.OpenVolume) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseVolume) + "\", \""
			+ boost::lexical_cast<std::string>(position.OpenAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseAmount) + "\", \""
			+ boost::lexical_cast<std::string>(position.PositionCost) + "\", \""
			+ boost::lexical_cast<std::string>(position.PreMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.UseMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.FrozenMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.FrozenCash) + "\", \""
			+ boost::lexical_cast<std::string>(position.FrozenCommission) + "\", \""
			+ boost::lexical_cast<std::string>(position.CashIn) + "\", \""
			+ boost::lexical_cast<std::string>(position.Commission) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseProfit) + "\", \""
			+ boost::lexical_cast<std::string>(position.PositionProfit) + "\", \""
			+ boost::lexical_cast<std::string>(position.PreSettlementPrice) + "\", \""
			+ boost::lexical_cast<std::string>(position.SettlementPrice) + "\", \""
			+ boost::lexical_cast<std::string>(position.SettlementID) + "\", \""
			+ boost::lexical_cast<std::string>(position.OpenCost) + "\", \""
			+ boost::lexical_cast<std::string>(position.ExchangeMargin) + "\", \""
			+ boost::lexical_cast<std::string>(position.CombPosition) + "\", \""
			+ boost::lexical_cast<std::string>(position.CombLongFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.CombShortFrozen) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseProfitByDate) + "\", \""
			+ boost::lexical_cast<std::string>(position.CloseProfitByTrade) + "\", \""
			+ boost::lexical_cast<std::string>(position.TodayPosition) + "\", \""
			+ boost::lexical_cast<std::string>(position.MarginRateByMoney) + "\", \""
			+ boost::lexical_cast<std::string>(position.MarginRateByVolume) + "\", \""
			+ boost::lexical_cast<std::string>(position.ClosableNum) + "\", \""
			+ boost::lexical_cast<std::string>(position.TodayClosableNum)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::GetHisPositions(vector<InvestorPositionField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISPOSITION_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.TradingDay, recordSet[row][0].c_str());
			strcpy(field.BrokerID, recordSet[row][1].c_str());
			strcpy(field.InvestorID, recordSet[row][2].c_str());
			strcpy(field.InstrumentID, recordSet[row][3].c_str());
			field.PosiDirection = recordSet[row][4][0];
			field.HedgeFlag = recordSet[row][5][0];
			field.AvgPositionPrice = toDouble(recordSet[row][6]);
			field.PositionDate = recordSet[row][7][0];
			field.YdPosition = toInt(recordSet[row][8]);
			field.Position = toInt(recordSet[row][9]);
			field.LongFrozen = toInt(recordSet[row][10]);
			field.ShortFrozen = toInt(recordSet[row][11]);
			field.LongFrozenAmount = toDouble(recordSet[row][12]);
			field.ShortFrozenAmount = toDouble(recordSet[row][13]);
			field.OpenVolume = toInt(recordSet[row][14]);
			field.CloseVolume = toInt(recordSet[row][15]);
			field.OpenAmount = toDouble(recordSet[row][16]);
			field.CloseAmount = toDouble(recordSet[row][17]);
			field.PositionCost = toDouble(recordSet[row][18]);
			field.PreMargin = toDouble(recordSet[row][19]);
			field.UseMargin = toDouble(recordSet[row][20]);
			field.FrozenMargin = toDouble(recordSet[row][21]);
			field.FrozenCash = toDouble(recordSet[row][22]);
			field.FrozenCommission = toDouble(recordSet[row][23]);
			field.CashIn = toDouble(recordSet[row][24]);
			field.Commission = toDouble(recordSet[row][25]);
			field.CloseProfit = toDouble(recordSet[row][26]);
			field.PositionProfit = toDouble(recordSet[row][27]);
			field.PreSettlementPrice = toDouble(recordSet[row][28]);
			field.SettlementPrice = toDouble(recordSet[row][29]);
			field.SettlementID = toInt(recordSet[row][30]);
			field.OpenCost = toDouble(recordSet[row][31]);
			field.ExchangeMargin = toDouble(recordSet[row][32]);
			field.CombPosition = toInt(recordSet[row][33]);
			field.CombLongFrozen = toInt(recordSet[row][34]);
			field.CombShortFrozen = toInt(recordSet[row][35]);
			field.CloseProfitByDate = toDouble(recordSet[row][36]);
			field.CloseProfitByTrade = toDouble(recordSet[row][37]);
			field.TodayPosition = toInt(recordSet[row][38]);
			field.MarginRateByMoney = toDouble(recordSet[row][39]);
			field.MarginRateByVolume = toDouble(recordSet[row][40]);
			field.ClosableNum = toInt(recordSet[row][41]);
			field.TodayClosableNum = toInt(recordSet[row][42]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry positions of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllHisPositions(vector<InvestorPositionField> &fieldsVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISPOSITION_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.TradingDay, recordSet[row][0].c_str());
			strcpy(field.BrokerID, recordSet[row][1].c_str());
			strcpy(field.InvestorID, recordSet[row][2].c_str());
			strcpy(field.InstrumentID, recordSet[row][3].c_str());
			field.PosiDirection = recordSet[row][4][0];
			field.HedgeFlag = recordSet[row][5][0];
			field.AvgPositionPrice = toDouble(recordSet[row][6]);
			field.PositionDate = recordSet[row][7][0];
			field.YdPosition = toInt(recordSet[row][8]);
			field.Position = toInt(recordSet[row][9]);
			field.LongFrozen = toInt(recordSet[row][10]);
			field.ShortFrozen = toInt(recordSet[row][11]);
			field.LongFrozenAmount = toDouble(recordSet[row][12]);
			field.ShortFrozenAmount = toDouble(recordSet[row][13]);
			field.OpenVolume = toInt(recordSet[row][14]);
			field.CloseVolume = toInt(recordSet[row][15]);
			field.OpenAmount = toDouble(recordSet[row][16]);
			field.CloseAmount = toDouble(recordSet[row][17]);
			field.PositionCost = toDouble(recordSet[row][18]);
			field.PreMargin = toDouble(recordSet[row][19]);
			field.UseMargin = toDouble(recordSet[row][20]);
			field.FrozenMargin = toDouble(recordSet[row][21]);
			field.FrozenCash = toDouble(recordSet[row][22]);
			field.FrozenCommission = toDouble(recordSet[row][23]);
			field.CashIn = toDouble(recordSet[row][24]);
			field.Commission = toDouble(recordSet[row][25]);
			field.CloseProfit = toDouble(recordSet[row][26]);
			field.PositionProfit = toDouble(recordSet[row][27]);
			field.PreSettlementPrice = toDouble(recordSet[row][28]);
			field.SettlementPrice = toDouble(recordSet[row][29]);
			field.SettlementID = toInt(recordSet[row][30]);
			field.OpenCost = toDouble(recordSet[row][31]);
			field.ExchangeMargin = toDouble(recordSet[row][32]);
			field.CombPosition = toInt(recordSet[row][33]);
			field.CombLongFrozen = toInt(recordSet[row][34]);
			field.CombShortFrozen = toInt(recordSet[row][35]);
			field.CloseProfitByDate = toDouble(recordSet[row][36]);
			field.CloseProfitByTrade = toDouble(recordSet[row][37]);
			field.TodayPosition = toInt(recordSet[row][38]);
			field.MarginRateByMoney = toDouble(recordSet[row][39]);
			field.MarginRateByVolume = toDouble(recordSet[row][40]);
			field.ClosableNum = toInt(recordSet[row][41]);
			field.TodayClosableNum = toInt(recordSet[row][42]);

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all positions faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorHisPosition(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + HISPOSITION_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 结算单信息
bool CDBOperation::UpdSettlementSheet(SettlementSheetField &field)
{
	std::string strSettlementSheet =
	        field.ChineseHeaderString + field.ChineseDateString
	        + field.ChineseEquityString + field.ChineseEquityAdjustString
	        + field.ChineseTradeString
	        + field.ChinesePositionString + field.ChinesePositionAdjustString
	        + field.ChineseTailString
			+ field.EnglishHeaderString + field.EnglishDateString
			+ field.EnglishEquityString + field.EnglishEquityAdjustString
			+ field.EnglishTradeString
			+ field.EnglishPositionString + field.EnglishPositionAdjustString
			+ field.EnglishTailString;

	string strSql = (string)"REPLACE INTO `" + INVESTORSETTLEMENTSHEET_TABLE + "` VALUES ( \"" \
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.SettlementDate + "\", \""
			+ boost::lexical_cast<std::string>(field.SettlementID) + "\", \""
//			+ UTF8toGB2312(strSettlementSheet)
			+ strSettlementSheet
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";

	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
// return -1:faild; 0: no data; 1:true
int CDBOperation::GetSettlementSheet(string &strSettlementSheet, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVESTORSETTLEMENTSHEET_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return 0;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}

		strSettlementSheet = recordSet[0][0];
	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry settlement sheet of " << strBrokerID << "-" << strInvestorID << "-" << strTradingDay << " faild !" << std::endl;
		return -1;
	}
}
bool CDBOperation::DelInvestorSettlementSheet(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + INVESTORSETTLEMENTSHEET_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 结算单确认信息
bool CDBOperation::UpdSettlementConfirmInfo(SettlementSheetConfirmInfoField &field)
{
	string strSql = (string)"REPLACE INTO `" + SETTLEMENTCONFIRMINFO_TABLE + "` VALUES ( \"" \
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.ConfirmDate + "\", \""
			+ field.ConfirmTime
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";

	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
int CDBOperation::GetSettlementConfirmInfo(SettlementSheetConfirmInfoField &field, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + SETTLEMENTCONFIRMINFO_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return 0;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}

		field.BrokerID = recordSet[0][0];
		field.InvestorID = recordSet[0][1];
		field.ConfirmDate = recordSet[0][2];
		field.ConfirmTime = recordSet[0][3];

	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry settlement confirminfo of " << strBrokerID << "-" << strInvestorID << " faild !" << std::endl;
		return -1;
	}
}
bool CDBOperation::DelInvestorSettlementConfirmInfo(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + SETTLEMENTCONFIRMINFO_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// tick行情信息
bool CDBOperation::UpdLastTick(DepthMarketDataField &tick)
{
	string strSql = (string)"REPLACE INTO `" + LASTTICK_TABLE + "` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(tick.TradingDay) + "\", \""
			+ tick.InstrumentID + "\", \""
			+ tick.ExchangeID + "\", \""
			+ boost::lexical_cast<std::string>(tick.LastPrice) + "\", \""
			+ boost::lexical_cast<std::string>(tick.OpenPrice) + "\", \""
			+ boost::lexical_cast<std::string>(tick.HighestPrice) + "\", \""
			+ boost::lexical_cast<std::string>(tick.LowestPrice) + "\", \""
			+ boost::lexical_cast<std::string>(tick.ClosePrice) + "\", \""
			+ boost::lexical_cast<std::string>(tick.OpenInterest) + "\", \""
			+ boost::lexical_cast<std::string>(tick.Turnover) + "\", \""
			+ tick.UpdateTime + "\", \""
			+ boost::lexical_cast<std::string>(tick.UpdateMillisec)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
// return -1:faild; 0: no data; 1:true
int CDBOperation::GetLastTick(DepthMarketDataField &tick, std::string strTradingDay, std::string strExchangeID, std::string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + LASTTICK_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "' AND ExchangeID = '" + strExchangeID + "' AND InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return 0;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}

		strcpy(tick.TradingDay, recordSet[0][0].c_str());
		strcpy(tick.InstrumentID, recordSet[0][1].c_str());
		strcpy(tick.ExchangeID, recordSet[0][2].c_str());
		tick.LastPrice = toDouble(recordSet[0][3]);
		tick.OpenPrice = toDouble(recordSet[0][4]);
		tick.HighestPrice = toDouble(recordSet[0][5]);
		tick.LowestPrice = toDouble(recordSet[0][6]);
		tick.ClosePrice = toDouble(recordSet[0][7]);
		tick.OpenInterest = toDouble(recordSet[0][8]);
		tick.Turnover = toDouble(recordSet[0][9]);
		strcpy(tick.UpdateTime, recordSet[0][10].c_str());
		tick.UpdateMillisec = toInt(recordSet[0][11]);

	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry last tick of " << strTradingDay << "-" << strExchangeID << "-" << strInstrumentID << " faild !" << std::endl;
		return -1;
	}
}
// return -1:faild; 0: no data; 1:true
int CDBOperation::GetLastTick(std::vector<DepthMarketDataField> &ticksVec, std::string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + LASTTICK_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return 0;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}
		DepthMarketDataField tick;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(tick.TradingDay, recordSet[row][0].c_str());
			strcpy(tick.InstrumentID, recordSet[row][1].c_str());
			strcpy(tick.ExchangeID, recordSet[row][2].c_str());
			tick.LastPrice = toDouble(recordSet[row][3]);
			tick.OpenPrice = toDouble(recordSet[row][4]);
			tick.HighestPrice = toDouble(recordSet[row][5]);
			tick.LowestPrice = toDouble(recordSet[row][6]);
			tick.ClosePrice = toDouble(recordSet[row][7]);
			tick.OpenInterest = toDouble(recordSet[row][8]);
			tick.Turnover = toDouble(recordSet[row][9]);
			strcpy(tick.UpdateTime, recordSet[row][10].c_str());
			tick.UpdateMillisec = toInt(recordSet[row][11]);

			ticksVec.push_back(tick);
		}

	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry last ticks of " << strTradingDay  << " faild !" << std::endl;
		return -1;
	}
}

// 结算信息
bool CDBOperation::UpdDailySettlementFlag(DailySettlementFlagField &field)
{
	DEBUG_ENTRY();
	string strSql = (string)"REPLACE INTO `" + DSF_TABLE + "` VALUES ( \"" \
			+ boost::lexical_cast<std::string>(field.DailySettlementFlagID) + "\", \""
			+ field.SettlementDate + "\", \""
			+ field.SettlementTime + "\", \""
			+ field.SettlementFlag + "\", \""
			+ field.SettlementBackupFlag + "\", \""
			+ field.BackupTime + "\", \""
			+ field.UserCode + "\", \""
			+ field.NeedSettlementDate + "\", \""
			+ field.Comment + "\", \""
			+ boost::lexical_cast<std::string>(field.IsNightClose)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
// return -1:faild; 0: no data; 1:true
int CDBOperation::GetLastDailySettlementFlag(DailySettlementFlagField &field)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + DSF_TABLE + "` order by id desc limit 1;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return 0;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return -1;
		}

		field.DailySettlementFlagID = toInt(recordSet[0][0]);
		field.SettlementDate = recordSet[0][1];
		field.SettlementTime = recordSet[0][2];
		field.SettlementFlag = recordSet[0][3];
		field.SettlementBackupFlag = recordSet[0][4];
		field.BackupTime = recordSet[0][5];
		field.UserCode = recordSet[0][6];
		field.NeedSettlementDate = recordSet[0][7];
		field.Comment = recordSet[0][8];
		field.IsNightClose = toInt(recordSet[0][9]);

	   return 1;
	}
	catch(...)
	{
		ERROR << "Qry last DailySettlementFlag faild !" << std::endl;
		return -1;
	}
}

// 持仓信息
bool CDBOperation::UpdPositionDetail(InvestorPositionDetailField &positionDetail)
{
	string strSql = (string)"REPLACE INTO `" + POSITIONDETAIL_TABLE + "` VALUES ( \"" \
			+ positionDetail.BrokerID + "\", \""
			+ positionDetail.InvestorID + "\", \""
			+ positionDetail.InstrumentID + "\", \""
			+ string(1, positionDetail.Direction) + "\", \""
			+ string(1, positionDetail.HedgeFlag) + "\", \""
			+ positionDetail.OpenDate + "\", \""
			+ positionDetail.TradeID + "\", \""
			+ boost::lexical_cast<std::string>(positionDetail.OpenPrice) + "\", \""
			+ boost::lexical_cast<std::string>(positionDetail.Volume) + "\", \""
			+ string(1, positionDetail.TradeType) + "\", \""
			+ positionDetail.CombInstrumentID + "\", \""
			+ positionDetail.ExchangeID + "\", \""
			+ boost::lexical_cast<std::string>(positionDetail.Margin) + "\", \""
			+ boost::lexical_cast<std::string>(positionDetail.LastSettlementPrice) + "\", \""
			+ positionDetail.TradingDay
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelPositionDetail(InvestorPositionDetailField &positionDetail)
{
	string strSql = (string)"DELETE FROM `" + POSITIONDETAIL_TABLE + "` WHERE BrokerID = \'" + positionDetail.BrokerID
				+ "' AND InvestorID = '" + positionDetail.InvestorID
				+ "' AND InstrumentID = '" + positionDetail.InstrumentID
				+ "' AND Direction = '" + positionDetail.Direction
				+ "' AND HedgeFlag = '" + positionDetail.HedgeFlag
				+ "' AND OpenDate = '" + positionDetail.OpenDate
				+ "' AND TradeID = '" + positionDetail.TradeID
				+ "\';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelZeroPositionDetails()
{
	string strSql = (string)"DELETE FROM `" + POSITIONDETAIL_TABLE + "` WHERE Volume = '0';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strBrokerID, string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITIONDETAIL_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			DEBUG << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionDetailField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			field.Direction = recordSet[row][3][0];
			field.HedgeFlag = recordSet[row][4][0];
			strcpy(field.OpenDate, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.OpenPrice = toDouble(recordSet[row][7]);
			field.Volume = toInt(recordSet[row][8]);
			field.TradeType = recordSet[row][9][0];
			strcpy(field.CombInstrumentID, recordSet[row][10].c_str());
			strcpy(field.ExchangeID, recordSet[row][11].c_str());
			field.Margin = toDouble(recordSet[row][12]);
			field.LastSettlementPrice = toDouble(recordSet[row][13]);
			strcpy(field.TradingDay, recordSet[row][14].c_str());

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry positiondetails of " << strBrokerID << "-" << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strBrokerID, string strInvestorID, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITIONDETAIL_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			DEBUG << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionDetailField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			field.Direction = recordSet[row][3][0];
			field.HedgeFlag = recordSet[row][4][0];
			strcpy(field.OpenDate, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.OpenPrice = toDouble(recordSet[row][7]);
			field.Volume = toInt(recordSet[row][8]);
			field.TradeType = recordSet[row][9][0];
			strcpy(field.CombInstrumentID, recordSet[row][10].c_str());
			strcpy(field.ExchangeID, recordSet[row][11].c_str());
			field.Margin = toDouble(recordSet[row][12]);
			field.LastSettlementPrice = toDouble(recordSet[row][13]);
			strcpy(field.TradingDay, recordSet[row][14].c_str());

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry positiondetails of " << strBrokerID << "-" << strInvestorID  << "-" << strInstrumentID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strBrokerID, string strInvestorID, string strInstrumentID, string strDirection)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITIONDETAIL_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND InstrumentID = '" + strInstrumentID+ "' AND Direction = '" + strDirection + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			DEBUG << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionDetailField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			field.Direction = recordSet[row][3][0];
			field.HedgeFlag = recordSet[row][4][0];
			strcpy(field.OpenDate, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.OpenPrice = toDouble(recordSet[row][7]);
			field.Volume = toInt(recordSet[row][8]);
			field.TradeType = recordSet[row][9][0];
			strcpy(field.CombInstrumentID, recordSet[row][10].c_str());
			strcpy(field.ExchangeID, recordSet[row][11].c_str());
			field.Margin = toDouble(recordSet[row][12]);
			field.LastSettlementPrice = toDouble(recordSet[row][13]);
			strcpy(field.TradingDay, recordSet[row][14].c_str());

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry positiondetails of " << strBrokerID << "-" << strInvestorID  << "-" << strInstrumentID << "-" << strDirection << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + POSITIONDETAIL_TABLE;
		strSql = strSql + "` WHERE InstrumentID = '" + strInstrumentID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			DEBUG << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		InvestorPositionDetailField field;
		for(int row = 0; row < recordCount; row++)
		{
			strcpy(field.BrokerID, recordSet[row][0].c_str());
			strcpy(field.InvestorID, recordSet[row][1].c_str());
			strcpy(field.InstrumentID, recordSet[row][2].c_str());
			field.Direction = recordSet[row][3][0];
			field.HedgeFlag = recordSet[row][4][0];
			strcpy(field.OpenDate, recordSet[row][5].c_str());
			strcpy(field.TradeID, recordSet[row][6].c_str());
			field.OpenPrice = toDouble(recordSet[row][7]);
			field.Volume = toInt(recordSet[row][8]);
			field.TradeType = recordSet[row][9][0];
			strcpy(field.CombInstrumentID, recordSet[row][10].c_str());
			strcpy(field.ExchangeID, recordSet[row][11].c_str());
			field.Margin = toDouble(recordSet[row][12]);
			field.LastSettlementPrice = toDouble(recordSet[row][13]);
			strcpy(field.TradingDay, recordSet[row][14].c_str());

			fieldsVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry positiondetails of " << strInstrumentID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorPositionDetails(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + POSITIONDETAIL_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 是否有内部出入金权限
bool CDBOperation::IsHasDepositAuth(string strInvestorID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT matchid FROM `" + MEMBER_TABLE;
		strSql = strSql + "` WHERE InvestorID = '" + strInvestorID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}

		strSql = (string)"SELECT iscash FROM `" + INFO_TABLE + "` WHERE ID = '" + recordSet[0][0] + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet1(m_DataBase.GetMysql());
		recordCount = recordSet1.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		if(recordSet1[0][0] == "0")
			return false;
		else
		   return true;
	}
	catch(...)
	{
		ERROR << "Qry has deposit auth of " << strInvestorID << " faild !"  << std::endl;
		return false;
	}
}

// 查询账号范围
bool CDBOperation::GetServerInvestorRange(string strServerID, long long &from, long long &to)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + INVEDTORSERVERPORTCONF_TABLE + "` WHERE id = '" + strServerID + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet scRecordSet(m_DataBase.GetMysql());
		int scRecordCount = scRecordSet.ExecuteSQL(strSql.c_str());
		if(scRecordCount == 0)
		{
			ERROR << "No set server port of server[" << strServerID << "]" << std::endl;
			return false;
		}
		if(scRecordCount < 0)
		{
			ERROR << "Qry server port of server[" << strServerID << "] faild!" << std::endl;
			return false;
		}
		from = toLongLong(scRecordSet[0][1]);
		to = toLongLong(scRecordSet[0][2]);

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry server investors range of [" << strServerID << "] faild !" << std::endl;
		return false;
	}
}

// 自动止损信息
bool CDBOperation::UpdAutoStopLoss(AutoStopLossField &field)
{
	string strSql = (string)"REPLACE INTO `" + AUTOSTOPLOSS_TABLE + "` VALUES ( \"" \
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ field.InstrumentID + "\", \""
			+ boost::lexical_cast<std::string>(field.AutoStopLossTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.AutoWinningTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.FloatingStopLossTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.FloatingStopWinningTickNum) + "\", \""
			+ boost::lexical_cast<std::string>(field.Step)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::DelAutoStopLoss(const string strBrokerID, const string strInvestorID, const string strInstrumentID)
{
	string strSql = (string)"DELETE FROM `" + AUTOSTOPLOSS_TABLE + "` WHERE BrokerID = \'" + strBrokerID
				+ "' AND InvestorID = '" + strInvestorID;
	if(!strInstrumentID.empty())
		strSql = strSql + "' AND InstrumentID = '" + strInstrumentID;
	strSql = strSql + "\';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::GetAutoStopLoss(vector<AutoStopLossField> &fieldVec, const string strBrokerID, const string strInvestorID, const string strInstrumentID)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + AUTOSTOPLOSS_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID;
		if(!strInstrumentID.empty())
			strSql = strSql + "' AND InstrumentID = '" + strInstrumentID;
		strSql = strSql + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		AutoStopLossField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.InstrumentID = recordSet[row][2];
			field.AutoStopLossTickNum = toInt(recordSet[row][3]);
			field.AutoWinningTickNum = toInt(recordSet[row][4]);
			field.FloatingStopLossTickNum = toInt(recordSet[row][5]);
			field.FloatingStopWinningTickNum = toInt(recordSet[row][6]);
			field.Step = toDouble(recordSet[row][7]);

			fieldVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry auto stop loss of " << strBrokerID << "-" << strInvestorID  << "-" << strInstrumentID << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::UpdTradeRiskTmpl_Prepare(TradeRiskTmplField &field)
{
	try{
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		MYSQL * conn = m_DataBase.GetMysql();
		MYSQL_STMT *stmt = mysql_stmt_init(conn); //创建MYSQL_STMT句柄
		std::string strSql = (string)"INSERT INTO `" + TRADERISKTMPL_TABLE + "` VALUES (?, ?, ?, ?, ?);";
	   if(mysql_stmt_prepare(stmt, strSql.c_str(), strSql.length()))
	   {
		   ERROR << "mysql_stmt_prepare: %s\n" << mysql_error(conn) << std::endl;;
		   return false;
	    }
	   MYSQL_BIND params[5];
	   memset(params, 0, sizeof(params));

	   std::string strBrokerID = UTF8toGB2312(field.BrokerID);
	   params[0].buffer_type = MYSQL_TYPE_STRING;
	   params[0].buffer = const_cast<char*>(strBrokerID.c_str());
	   params[0].buffer_length = strBrokerID.length();

	   std::string strInvestorID = UTF8toGB2312(field.InvestorID);
	   params[1].buffer_type = MYSQL_TYPE_STRING;
	   params[1].buffer = const_cast<char*>(strInvestorID.c_str());
	   params[1].buffer_length = strInvestorID.length();

	   std::string strClientType = boost::lexical_cast<std::string>(field.ClientType);
	   params[2].buffer_type = MYSQL_TYPE_STRING;
	   params[2].buffer = const_cast<char*>(strClientType.c_str());
	   params[2].buffer_length = strClientType.length();

	   params[3].buffer_type = MYSQL_TYPE_STRING;
	   params[3].buffer = const_cast<char*>(field.Time.c_str());
	   params[3].buffer_length = field.Time.length();

	   params[4].buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
	   params[4].buffer = const_cast<char*>(field.TradeRiskTmplTxt.c_str());
	   params[4].buffer_length = field.TradeRiskTmplTxt.length();

		mysql_stmt_bind_param(stmt, params);
	   mysql_stmt_execute(stmt);           //执行与语句句柄相关的预处理
	   mysql_stmt_close(stmt);

	   return true;
	}
	catch(...)
	{
		ERROR << "Record trade risk tmpl of " << field.BrokerID << "-" << field.InvestorID  << "-" << field.ClientType << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::UpdTradeRiskTmpl(TradeRiskTmplField &field)
{
	string strSql = (string)"REPLACE INTO `" + TRADERISKTMPL_TABLE + "` VALUES ( \"" \
			+ field.BrokerID + "\", \""
			+ field.InvestorID + "\", \""
			+ boost::lexical_cast<std::string>(field.ClientType) + "\", \""
			+ field.Time + "\", \""
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}
bool CDBOperation::GetTradeRiskTmpl(TradeRiskTmplField &field, const string strBrokerID, const string strInvestorID, int clientType)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADERISKTMPL_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID;
		strSql = strSql + "' AND ClientType = '" + boost::lexical_cast<std::string>(clientType) + "';";

		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());
		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.ClientType = toInt(recordSet[row][2]);
			field.Time = recordSet[row][3];
			field.TradeRiskTmplTxt = recordSet[row][4];
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry trade risk tmpl of " << strBrokerID << "-" << strInvestorID  << "-" << clientType << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::DelInvestorTradeRiskTmpl(string strBrokerID, string strInvestorID)
{
	string strSql = (string)"DELETE FROM `" + TRADERISKTMPL_TABLE + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "';";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		return true;
}

// 交易柜台信息
bool CDBOperation::UpdTradeCounter(TradeCounterField &field)
{
	string strTradeCounterID = (field.TradeCounterID <= 0) ? "0" : boost::lexical_cast<std::string>(field.TradeCounterID);
	string strSql = (string)"REPLACE INTO `" + TRADECOUNTER_TABLE+ "` VALUES ( \"" \
			+ strTradeCounterID + "\", \""
			+ field.BrokerID + "\", \""
			+ field.Name + "\", \""
			+ field.Protocol + "\", \""
			+ field.FrontAddr + "\", \""
			+ field.AppID + "\", \""
			+ field.RelayAppID + "\", \""
			+ field.AuthCode + "\", \""
			+ boost::lexical_cast<std::string>(field.IsDefault)
			+ "\" );";

	//		std::cout << "SQL :" << strSql << "\n";
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		if(-1 == m_DataBase.ExecQuery(strSql))
		{
			BUG_FILE()
			ERROR << "Mysql execute failed !" << std::endl;
			ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
			ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
			std::cout << "SQL :" << strSql << "\n";
			return false;
		}
		if(field.TradeCounterID == 0)
		{
			strSql = (string)"SELECT MAX(TradeCounterID) FROM `" + TRADECOUNTER_TABLE + "`;";
			DEBUG << strSql << std::endl;
			CRecordSet recordSet(m_DataBase.GetMysql());
			int recordCount = recordSet.ExecuteSQL(strSql.c_str());

			if(recordCount == 0)
			{
	//			INFO << "No data!" << std::endl;
				return false;
			}
			if(recordCount < 0)
			{
				ERROR << "Execute faild!" << std::endl;
				return false;
			}
			field.TradeCounterID = toInt(recordSet[0][0]);
		}
		return true;
}
bool CDBOperation::GetTradeCounter(vector<TradeCounterField> &fieldVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + TRADECOUNTER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount == 0)
		{
			INFO << "No data!" << std::endl;
			return false;
		}
		if(recordCount < 0)
		{
			INFO << "Execute faild!" << std::endl;
			return false;
		}
		TradeCounterField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.TradeCounterID = toInt(recordSet[row][0]);
			field.BrokerID = recordSet[row][1];
			field.Name = recordSet[row][2];
			field.Protocol = recordSet[row][3];
			field.FrontAddr = recordSet[row][4];
			field.AppID = recordSet[row][5];
			field.RelayAppID = recordSet[row][6];
			field.AuthCode = recordSet[row][7];
			field.IsDefault = toInt(recordSet[row][8]);

			fieldVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry trade counters faild !"  << std::endl;
		return false;
	}
}

//执行宣告信息
bool CDBOperation::UpdExecOrder(ExecOrderField &execOrder)
{
	string strSql = (string)"REPLACE INTO `" + EXECORDER_TABLE +"` VALUES ( \"" \
			+ execOrder.BrokerID + "\", \""
			+ execOrder.InvestorID + "\", \""
			+ execOrder.InstrumentID + "\", \""
			+ execOrder.ExecOrderRef + "\", \""
			+ execOrder.UserID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.Volume) + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.RequestID) + "\", \""
			+ execOrder.BusinessUnit + "\", \""
			+ execOrder.OffsetFlag + "\", \""
			+ execOrder.HedgeFlag + "\", \""
			+ execOrder.ActionType + "\", \""
			+ execOrder.PosiDirection + "\", \""
			+ execOrder.ReservePositionFlag + "\", \""
			+ execOrder.CloseFlag + "\", \""
			+ execOrder.ExecOrderLocalID + "\", \""
			+ execOrder.ExchangeID + "\", \""
			+ execOrder.ParticipantID + "\", \""
			+ execOrder.ClientID + "\", \""
			+ execOrder.ExchangeInstID + "\", \""
			+ execOrder.TraderID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.InstallID) + "\", \""
			+ execOrder.OrderSubmitStatus + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.NotifySequence) + "\", \""
			+ execOrder.TradingDay + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.SettlementID) + "\", \""
			+ execOrder.ExecOrderSysID + "\", \""
			+ execOrder.InsertDate + "\", \""
			+ execOrder.InsertTime + "\", \""
			+ execOrder.CancelTime + "\", \""
			+ execOrder.ExecResult + "\", \""
			+ execOrder.ClearingPartID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.SequenceNo) + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.FrontID) + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.SessionID) + "\", \""
			+ execOrder.UserProductInfo + "\", \""
			+ execOrder.StatusMsg + "\", \""
			+ execOrder.ActiveUserID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.BrokerExecOrderSeq) + "\", \""
			+ execOrder.BranchID + "\", \""
			+ execOrder.InvestUnitID + "\", \""
			+ execOrder.AccountID + "\", \""
			+ execOrder.CurrencyID + "\", \""
			+ execOrder.IPAddress + "\", \""
			+ execOrder.MacAddress
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::CleanExecOrder()
{
	string strSql = (string)"truncate table `" + EXECORDER_TABLE +"`;" ;

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetExecOrder(vector<ExecOrderField> &execOrdersVec, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + EXECORDER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		ExecOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.InstrumentID = recordSet[row][2];
			field.ExecOrderRef = recordSet[row][3];
			field.UserID = recordSet[row][4];
		   field.Volume = toInt(recordSet[row][5]);
		   field.RequestID = toInt(recordSet[row][6]);
		   field.BusinessUnit = recordSet[row][7];
		   field.OffsetFlag = recordSet[row][8];
		   field.HedgeFlag = recordSet[row][9];
		   field.ActionType = recordSet[row][10];
		   field.PosiDirection = recordSet[row][11];
		   field.ReservePositionFlag = recordSet[row][12];
		   field.CloseFlag = recordSet[row][13];
		   field.ExecOrderLocalID = recordSet[row][14];
		   field.ExchangeID = recordSet[row][15];
		   field.ParticipantID = recordSet[row][16];
		   field.ClientID = recordSet[row][17];
		   field.ExchangeInstID = recordSet[row][18];
		   field.TraderID = recordSet[row][19];
		   field.InstallID = toInt(recordSet[row][20]);
		   field.OrderSubmitStatus = recordSet[row][21];
		   field.NotifySequence = toInt(recordSet[row][22]);
		   field.TradingDay = recordSet[row][23];
		   field.SettlementID = toInt(recordSet[row][24]);
		   field.ExecOrderSysID = recordSet[row][25];
		   field.InsertDate = recordSet[row][26];
		   field.InsertTime = recordSet[row][27];
		   field.CancelTime = recordSet[row][28];
		   field.ExecResult = recordSet[row][29];
		   field.ClearingPartID = recordSet[row][30];
		   field.SequenceNo = toInt(recordSet[row][31]);
		   field.FrontID = toInt(recordSet[row][32]);
		   field.SessionID = toInt(recordSet[row][33]);
		   field.UserProductInfo = recordSet[row][34];
		   field.StatusMsg = recordSet[row][35];
		   field.ActiveUserID = recordSet[row][36];
		   field.BrokerExecOrderSeq = toInt(recordSet[row][37]);
		   field.BranchID = recordSet[row][38];
		   field.InvestUnitID = recordSet[row][39];
		   field.AccountID = recordSet[row][40];
		   field.CurrencyID = recordSet[row][41];
		   field.IPAddress = recordSet[row][42];
		   field.MacAddress = recordSet[row][43];

			execOrdersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all execute orders of " << strTradingDay << "and investor:" << strBrokerID << "-" << strInvestorID  << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllExecOrder(vector<ExecOrderField> &execOrdersVec, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + EXECORDER_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		ExecOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.InstrumentID = recordSet[row][2];
			field.ExecOrderRef = recordSet[row][3];
			field.UserID = recordSet[row][4];
		   field.Volume = toInt(recordSet[row][5]);
		   field.RequestID = toInt(recordSet[row][6]);
		   field.BusinessUnit = recordSet[row][7];
		   field.OffsetFlag = recordSet[row][8];
		   field.HedgeFlag = recordSet[row][9];
		   field.ActionType = recordSet[row][10];
		   field.PosiDirection = recordSet[row][11];
		   field.ReservePositionFlag = recordSet[row][12];
		   field.CloseFlag = recordSet[row][13];
		   field.ExecOrderLocalID = recordSet[row][14];
		   field.ExchangeID = recordSet[row][15];
		   field.ParticipantID = recordSet[row][16];
		   field.ClientID = recordSet[row][17];
		   field.ExchangeInstID = recordSet[row][18];
		   field.TraderID = recordSet[row][19];
		   field.InstallID = toInt(recordSet[row][20]);
		   field.OrderSubmitStatus = recordSet[row][21];
		   field.NotifySequence = toInt(recordSet[row][22]);
		   field.TradingDay = recordSet[row][23];
		   field.SettlementID = toInt(recordSet[row][24]);
		   field.ExecOrderSysID = recordSet[row][25];
		   field.InsertDate = recordSet[row][26];
		   field.InsertTime = recordSet[row][27];
		   field.CancelTime = recordSet[row][28];
		   field.ExecResult = recordSet[row][29];
		   field.ClearingPartID = recordSet[row][30];
		   field.SequenceNo = toInt(recordSet[row][31]);
		   field.FrontID = toInt(recordSet[row][32]);
		   field.SessionID = toInt(recordSet[row][33]);
		   field.UserProductInfo = recordSet[row][34];
		   field.StatusMsg = recordSet[row][35];
		   field.ActiveUserID = recordSet[row][36];
		   field.BrokerExecOrderSeq = toInt(recordSet[row][37]);
		   field.BranchID = recordSet[row][38];
		   field.InvestUnitID = recordSet[row][39];
		   field.AccountID = recordSet[row][40];
		   field.CurrencyID = recordSet[row][41];
		   field.IPAddress = recordSet[row][42];
		   field.MacAddress = recordSet[row][43];

			execOrdersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all execute orders of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllExecOrder(vector<ExecOrderField> &execOrdersVec)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + EXECORDER_TABLE + "`;";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		ExecOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.InstrumentID = recordSet[row][2];
			field.ExecOrderRef = recordSet[row][3];
			field.UserID = recordSet[row][4];
		   field.Volume = toInt(recordSet[row][5]);
		   field.RequestID = toInt(recordSet[row][6]);
		   field.BusinessUnit = recordSet[row][7];
		   field.OffsetFlag = recordSet[row][8];
		   field.HedgeFlag = recordSet[row][9];
		   field.ActionType = recordSet[row][10];
		   field.PosiDirection = recordSet[row][11];
		   field.ReservePositionFlag = recordSet[row][12];
		   field.CloseFlag = recordSet[row][13];
		   field.ExecOrderLocalID = recordSet[row][14];
		   field.ExchangeID = recordSet[row][15];
		   field.ParticipantID = recordSet[row][16];
		   field.ClientID = recordSet[row][17];
		   field.ExchangeInstID = recordSet[row][18];
		   field.TraderID = recordSet[row][19];
		   field.InstallID = toInt(recordSet[row][20]);
		   field.OrderSubmitStatus = recordSet[row][21];
		   field.NotifySequence = toInt(recordSet[row][22]);
		   field.TradingDay = recordSet[row][23];
		   field.SettlementID = toInt(recordSet[row][24]);
		   field.ExecOrderSysID = recordSet[row][25];
		   field.InsertDate = recordSet[row][26];
		   field.InsertTime = recordSet[row][27];
		   field.CancelTime = recordSet[row][28];
		   field.ExecResult = recordSet[row][29];
		   field.ClearingPartID = recordSet[row][30];
		   field.SequenceNo = toInt(recordSet[row][31]);
		   field.FrontID = toInt(recordSet[row][32]);
		   field.SessionID = toInt(recordSet[row][33]);
		   field.UserProductInfo = recordSet[row][34];
		   field.StatusMsg = recordSet[row][35];
		   field.ActiveUserID = recordSet[row][36];
		   field.BrokerExecOrderSeq = toInt(recordSet[row][37]);
		   field.BranchID = recordSet[row][38];
		   field.InvestUnitID = recordSet[row][39];
		   field.AccountID = recordSet[row][40];
		   field.CurrencyID = recordSet[row][41];
		   field.IPAddress = recordSet[row][42];
		   field.MacAddress = recordSet[row][43];

			execOrdersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all execute orders faild !"  << std::endl;
		return false;
	}
}

//执行宣告信息
bool CDBOperation::UpdHisExecOrder(ExecOrderField &execOrder)
{
	string strSql = (string)"REPLACE INTO `" + HISEXECORDER_TABLE +"` VALUES ( \"" \
			+ execOrder.BrokerID + "\", \""
			+ execOrder.InvestorID + "\", \""
			+ execOrder.InstrumentID + "\", \""
			+ execOrder.ExecOrderRef + "\", \""
			+ execOrder.UserID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.Volume) + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.RequestID) + "\", \""
			+ execOrder.BusinessUnit + "\", \""
			+ execOrder.OffsetFlag + "\", \""
			+ execOrder.HedgeFlag + "\", \""
			+ execOrder.ActionType + "\", \""
			+ execOrder.PosiDirection + "\", \""
			+ execOrder.ReservePositionFlag + "\", \""
			+ execOrder.CloseFlag + "\", \""
			+ execOrder.ExecOrderLocalID + "\", \""
			+ execOrder.ExchangeID + "\", \""
			+ execOrder.ParticipantID + "\", \""
			+ execOrder.ClientID + "\", \""
			+ execOrder.ExchangeInstID + "\", \""
			+ execOrder.TraderID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.InstallID) + "\", \""
			+ execOrder.OrderSubmitStatus + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.NotifySequence) + "\", \""
			+ execOrder.TradingDay + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.SettlementID) + "\", \""
			+ execOrder.ExecOrderSysID + "\", \""
			+ execOrder.InsertDate + "\", \""
			+ execOrder.InsertTime + "\", \""
			+ execOrder.CancelTime + "\", \""
			+ execOrder.ExecResult + "\", \""
			+ execOrder.ClearingPartID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.SequenceNo) + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.FrontID) + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.SessionID) + "\", \""
			+ execOrder.UserProductInfo + "\", \""
			+ execOrder.StatusMsg + "\", \""
			+ execOrder.ActiveUserID + "\", \""
			+ boost::lexical_cast<std::string>(execOrder.BrokerExecOrderSeq) + "\", \""
			+ execOrder.BranchID + "\", \""
			+ execOrder.InvestUnitID + "\", \""
			+ execOrder.AccountID + "\", \""
			+ execOrder.CurrencyID + "\", \""
			+ execOrder.IPAddress + "\", \""
			+ execOrder.MacAddress
			+ "\" );";

//		std::cout << "SQL :" << strSql << "\n";
	if(-1 == m_DataBase.Ping())
	{
		ConnectDB();
	}
	if(-1 == m_DataBase.ExecQuery(strSql))
	{
		BUG_FILE()
		ERROR << "Mysql execute failed !" << std::endl;
		ERROR << "Mysql errno : " << m_DataBase.Get_errno() << std::endl;
		ERROR << "Mysql error : " << m_DataBase.Get_error() << std::endl;
		std::cout << "SQL :" << strSql << "\n";
		return false;
	}
	return true;
}
bool CDBOperation::GetHisExecOrder(vector<ExecOrderField> &execOrdersVec, string strBrokerID, string strInvestorID, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISEXECORDER_TABLE;
		strSql = strSql + "` WHERE BrokerID = '" + strBrokerID + "' AND InvestorID = '" + strInvestorID + "' AND TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		ExecOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.InstrumentID = recordSet[row][2];
			field.ExecOrderRef = recordSet[row][3];
			field.UserID = recordSet[row][4];
		   field.Volume = toInt(recordSet[row][5]);
		   field.RequestID = toInt(recordSet[row][6]);
		   field.BusinessUnit = recordSet[row][7];
		   field.OffsetFlag = recordSet[row][8];
		   field.HedgeFlag = recordSet[row][9];
		   field.ActionType = recordSet[row][10];
		   field.PosiDirection = recordSet[row][11];
		   field.ReservePositionFlag = recordSet[row][12];
		   field.CloseFlag = recordSet[row][13];
		   field.ExecOrderLocalID = recordSet[row][14];
		   field.ExchangeID = recordSet[row][15];
		   field.ParticipantID = recordSet[row][16];
		   field.ClientID = recordSet[row][17];
		   field.ExchangeInstID = recordSet[row][18];
		   field.TraderID = recordSet[row][19];
		   field.InstallID = toInt(recordSet[row][20]);
		   field.OrderSubmitStatus = recordSet[row][21];
		   field.NotifySequence = toInt(recordSet[row][22]);
		   field.TradingDay = recordSet[row][23];
		   field.SettlementID = toInt(recordSet[row][24]);
		   field.ExecOrderSysID = recordSet[row][25];
		   field.InsertDate = recordSet[row][26];
		   field.InsertTime = recordSet[row][27];
		   field.CancelTime = recordSet[row][28];
		   field.ExecResult = recordSet[row][29];
		   field.ClearingPartID = recordSet[row][30];
		   field.SequenceNo = toInt(recordSet[row][31]);
		   field.FrontID = toInt(recordSet[row][32]);
		   field.SessionID = toInt(recordSet[row][33]);
		   field.UserProductInfo = recordSet[row][34];
		   field.StatusMsg = recordSet[row][35];
		   field.ActiveUserID = recordSet[row][36];
		   field.BrokerExecOrderSeq = toInt(recordSet[row][37]);
		   field.BranchID = recordSet[row][38];
		   field.InvestUnitID = recordSet[row][39];
		   field.AccountID = recordSet[row][40];
		   field.CurrencyID = recordSet[row][41];
		   field.IPAddress = recordSet[row][42];
		   field.MacAddress = recordSet[row][43];

			execOrdersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all history execute orders of " << strTradingDay << "and investor:" << strBrokerID << "-" << strInvestorID  << " faild !"  << std::endl;
		return false;
	}
}
bool CDBOperation::GetAllHisExecOrder(vector<ExecOrderField> &execOrdersVec, string strTradingDay)
{
	try
	{
		// get sql string
		std::string strSql = (string)"SELECT * FROM `" + HISEXECORDER_TABLE;
		strSql = strSql + "` WHERE TradingDay = '" + strTradingDay + "';";
		DEBUG << strSql << std::endl;

		// execute sql
		if(-1 == m_DataBase.Ping())
		{
			ConnectDB();
		}
		CRecordSet recordSet(m_DataBase.GetMysql());
		int recordCount = recordSet.ExecuteSQL(strSql.c_str());

		if(recordCount < 0)
		{
			ERROR << "Execute faild!" << std::endl;
			return false;
		}
		ExecOrderField field;
		for(int row = 0; row < recordCount; row++)
		{
			field.BrokerID = recordSet[row][0];
			field.InvestorID = recordSet[row][1];
			field.InstrumentID = recordSet[row][2];
			field.ExecOrderRef = recordSet[row][3];
			field.UserID = recordSet[row][4];
		   field.Volume = toInt(recordSet[row][5]);
		   field.RequestID = toInt(recordSet[row][6]);
		   field.BusinessUnit = recordSet[row][7];
		   field.OffsetFlag = recordSet[row][8];
		   field.HedgeFlag = recordSet[row][9];
		   field.ActionType = recordSet[row][10];
		   field.PosiDirection = recordSet[row][11];
		   field.ReservePositionFlag = recordSet[row][12];
		   field.CloseFlag = recordSet[row][13];
		   field.ExecOrderLocalID = recordSet[row][14];
		   field.ExchangeID = recordSet[row][15];
		   field.ParticipantID = recordSet[row][16];
		   field.ClientID = recordSet[row][17];
		   field.ExchangeInstID = recordSet[row][18];
		   field.TraderID = recordSet[row][19];
		   field.InstallID = toInt(recordSet[row][20]);
		   field.OrderSubmitStatus = recordSet[row][21];
		   field.NotifySequence = toInt(recordSet[row][22]);
		   field.TradingDay = recordSet[row][23];
		   field.SettlementID = toInt(recordSet[row][24]);
		   field.ExecOrderSysID = recordSet[row][25];
		   field.InsertDate = recordSet[row][26];
		   field.InsertTime = recordSet[row][27];
		   field.CancelTime = recordSet[row][28];
		   field.ExecResult = recordSet[row][29];
		   field.ClearingPartID = recordSet[row][30];
		   field.SequenceNo = toInt(recordSet[row][31]);
		   field.FrontID = toInt(recordSet[row][32]);
		   field.SessionID = toInt(recordSet[row][33]);
		   field.UserProductInfo = recordSet[row][34];
		   field.StatusMsg = recordSet[row][35];
		   field.ActiveUserID = recordSet[row][36];
		   field.BrokerExecOrderSeq = toInt(recordSet[row][37]);
		   field.BranchID = recordSet[row][38];
		   field.InvestUnitID = recordSet[row][39];
		   field.AccountID = recordSet[row][40];
		   field.CurrencyID = recordSet[row][41];
		   field.IPAddress = recordSet[row][42];
		   field.MacAddress = recordSet[row][43];

			execOrdersVec.push_back(field);
		}

	   return true;
	}
	catch(...)
	{
		ERROR << "Qry all history execute orders of " << strTradingDay << " faild !"  << std::endl;
		return false;
	}
}

