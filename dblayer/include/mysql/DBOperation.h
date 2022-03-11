/*
 * DBOperation.h
 *
 *  Created on: 2018年11月20日
 *      Author: wangpf
 */

#ifndef DBOPERATION_H_
#define DBOPERATION_H_

#include <string>
#include "MysqlDataBase.h"
#include "pbserdes/ETPTypes.h"
#include <vector>
#include <set>
//#include <etp/TKMutex.h>
//#include <etp/TimedTask.h>
using namespace std;

class CDBOperation
{
public:
	CDBOperation();
	~CDBOperation();

public:
	bool ConnectDB(string host, string user,
			 string passwd, string db,
			 unsigned int port,
			 unsigned long client_flag);
	void DisConnectDB();

public:
	 /* 主要功能:开始事务 */
	 int Start_Transaction();
	 /* 主要功能:提交事务 */
	 int Commit();
	 /* 主要功能:回滚事务 */
	 int Rollback();

public:
	// 创建表
	void CreateLoginLogTable(); // 创建用户登录日志表


	// 操作
	//账号登录日志
	bool InsertLoginLog_Prepare(LoginLogField loginLog);
	bool InsertLoginLog(LoginLogField loginLog);

	// 账号信息
	bool UpdInvestorInfo(InvestorInfoField &field);
	bool GetInvestorInfo(InvestorInfoField &field, string strBrokerID, string strInvestorID);
	bool GetInvestorInfo(InvestorInfoField &field, int id);
	bool GetInvestorInfo(vector<InvestorInfoField> &fieldsVec);
	bool GetInvestorInfo(vector<InvestorInfoField> &fieldsVec, string strServerID);
	bool DelInvestorInfo(string strBrokerID, string strInvestorID);

	// 交易所
	bool GetExchangeInfo(ExchangeInfoField &field, string strExchangeID);
	bool GetExchangeInfo(vector<ExchangeInfoField> &fieldsVec);

	// 经纪公司
	bool GetBrokerInfo(BrokerInfoField &field, string strBrokerID);
	bool GetBrokerInfo(vector<BrokerInfoField> &fieldsVec);

	// 品种信息
	bool UpdProductInfo(ProductInfoField &field);
	bool GetProductInfo(ProductInfoField &field, string strProductID);
	bool GetProductInfo(vector<ProductInfoField> &fieldsVec);

	// 合约信息
	bool UpdInstrumentInfo(InstrumentInfoField &field);
	bool GetInstrumentInfo(InstrumentInfoField &field, string strInstrumentID);
	bool GetInstrumentInfo(vector<InstrumentInfoField> &fieldsVec);
	bool DelInstrumentInfo(string strInstrumentID);

	// 保证金模板信息
	bool GetMarginRateModelInfo(MarginRateModelInfoField &field, int modelid);
	bool GetAllMarginRateModelInfo(std::vector<MarginRateModelInfoField> &fieldVec);

	// 保证金信息
	bool UpdMarginRateInfo(MarginRateInfoField &field);
	bool GetMarginRateInfo(MarginRateInfoField &field, int modelid, string strInstrumentID);
	bool GetMarginRateInfo(vector<MarginRateInfoField> &fieldsVec, int modelid);
	bool GetMarginRateInfo(vector<MarginRateInfoField> &fieldsVec);

	// 手续费模板信息
	bool GetCommissionRateModelInfo(CommissionRateModelInfoField &field, int modelid);
	bool GetAllCommissionRateModelInfo(std::vector<CommissionRateModelInfoField> &fieldVec);

	// 手续费信息
	bool UpdCommissionRateInfo(CommissionRateInfoField &field);
	bool GetCommissionRateInfo(CommissionRateInfoField &field, int modelid, string strInstrumentID);
	bool GetCommissionRateInfo(vector<CommissionRateInfoField> &fieldsVec, int modelid);
	bool GetCommissionRateInfo(vector<CommissionRateInfoField> &fieldsVec);

	//委托信息
	bool UpdOrder(OrderField &order);
	bool CleanOrder();
	bool GetOrder(vector<OrderField> &ordersVec, string strBrokerID, string strInvestorID, string strTradingDay);
	bool GetAllOrder(vector<OrderField> &ordersVec, string strTradingDay);
	bool GetAllOrder(vector<OrderField> &ordersVec, string strTradingDay, string strInstrumentID);
	bool GetAllOrder(vector<OrderField> &ordersVec);
	long long GetMaxOrderSysID();
	bool GetInvestorsOfOrder(set<string> &investorsSet);
	bool DelInvestorOrder(string strBrokerID, string strInvestorID);

	// 历史委托信息
	bool UpdHisOrder(OrderField &order);
	bool GetHisOrder(vector<OrderField> &ordersVec, string strBrokerID, string strInvestorID, string strTradingDay);
	bool GetAllHisOrder(vector<OrderField> &ordersVec, string strTradingDay);
	bool DelInvestorHisOrder(string strBrokerID, string strInvestorID);

	// 成交信息
	bool UpdTrade(TradeField &trade);
	bool CleanTrade();
	bool GetTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID);
	bool GetTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID, string strTradingDay);
	bool GetAllTrade(vector<TradeField> &tradesVec, string strTradingDay);
	bool GetAllTrade(vector<TradeField> &tradesVec);
	long long GetMaxTradeID();
	bool GetInvestorsOfTrade(set<string> &investorsSet);
	bool DelInvestorTrade(string strBrokerID, string strInvestorID);

	// 历史成交信息
	bool UpdHisTrade(TradeField &trade);
	bool GetHisTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID, string strTradingDay);
	bool GetHisTrade(vector<TradeField> &tradesVec, string strBrokerID, string strInvestorID, string strBeginDate, string strEndDate);
	bool GetAllHisTrade(vector<TradeField> &tradesVec, string strTradingDay);
	bool DelInvestorHisTrade(string strBrokerID, string strInvestorID);

	// 资金信息
	bool UpdTradingAccount(TradingAccountField &field);
	// return -1:faild; 0: no data; 1:true
	int GetTradingAccount(TradingAccountField &field, string strBrokerID, string strInvestorID);
	bool DelInvestorTradingAccount(string strBrokerID, string strInvestorID);

	// 持仓信息
	bool UpdPosition(InvestorPositionField &position);
	bool DelPosition(InvestorPositionField &position);
	bool DelZeroPositions();
	bool GetPositions(vector<InvestorPositionField> &fieldsVec, string strBrokerID, string strInvestorID);
	bool GetAllPositions(vector<InvestorPositionField> &fieldsVec);
	bool GetPosition(InvestorPositionField &field, string strBrokerID, string strInvestorID, string strInstrumentID, string strPosiDirection);
	bool GetPosition(vector<InvestorPositionField> &fieldsVec, string strInstrumentID);
	bool DelInvestorPosition(string strBrokerID, string strInvestorID);

	// 云条件单信息
	bool UpdCloudOrder(CloudOrderField &field);
	bool GetCloudOrder(vector<CloudOrderField> &fieldsVec, string strBrokerID, string strInvestorID);
	bool GetAllCloudOrder(vector<CloudOrderField> &fieldsVec);
	bool GetInvalidCloudOrder(vector<CloudOrderField> &fieldsVec);
	bool DelCloudOrder(string strInstrumentID, string strStatus);
	bool DelInvestorCloudOrder(string strBrokerID, string strInvestorID);
	bool DelAllInvalidCloudOrder();

	// 历史云条件单信息
	bool UpdHisCloudOrder(CloudOrderField &field);
	bool GetHisCloudOrder(vector<CloudOrderField> &fieldsVec, string strBrokerID, string strInvestorID);

	// 出入金流水
	bool UpdCashFlow(CashFlowField &field);
	bool GetUnHandleCashFlow(vector<CashFlowField> &fieldsVec);
	bool GetCashFlow(vector<CashFlowField> &fieldsVec, string strBrokerID, string strInvestorID);
	bool GetAllCashFlow(vector<CashFlowField> &fieldsVec);
	bool DelCashFlow(CashFlowField &field);
	bool DelInvestorCashFlow(string strBrokerID, string strInvestorID);

	// 历史出入金流水
	bool UpdHisCashFlow(CashFlowField &field);
	bool GetUnHandleHisCashFlow(vector<CashFlowField> &fieldsVec);
	bool GetHisCashFlow(vector<CashFlowField> &fieldsVec, string strBrokerID, string strInvestorID);
	bool DelInvestorHisCashFlow(string strBrokerID, string strInvestorID);

	// 结算资金信息
	bool UpdSettlementCash(TradingAccountField &field);
	// return -1:faild; 0: no data; 1:true
	int GetSettlementCash(TradingAccountField &field, string strBrokerID, string strInvestorID, string strTradingDay);
	bool DelInvestorSettlementCash(string strBrokerID, string strInvestorID);

	// 历史持仓信息
	bool UpdHisPosition(InvestorPositionField &position);
	bool GetHisPositions(vector<InvestorPositionField> &fieldsVec, string strBrokerID, string strInvestorID);
	bool GetAllHisPositions(vector<InvestorPositionField> &fieldsVec);
	bool DelInvestorHisPosition(string strBrokerID, string strInvestorID);

	// 结算单信息
	bool UpdSettlementSheet(SettlementSheetField &field);
	// return -1:faild; 0: no data; 1:true
	int GetSettlementSheet(string &strSettlementSheet, string strBrokerID, string strInvestorID, string strTradingDay);
	bool DelInvestorSettlementSheet(string strBrokerID, string strInvestorID);

	// 结算单确认信息
	bool UpdSettlementConfirmInfo(SettlementSheetConfirmInfoField &field);
	int GetSettlementConfirmInfo(SettlementSheetConfirmInfoField &field, string strBrokerID, string strInvestorID);
	bool DelInvestorSettlementConfirmInfo(string strBrokerID, string strInvestorID);

	// tick行情信息
	bool UpdLastTick(DepthMarketDataField &tick);
	// return -1:faild; 0: no data; 1:true
	int GetLastTick(DepthMarketDataField &tick, std::string strTradingDay, std::string strExchangeID, std::string strInstrumentID);
	// return -1:faild; 0: no data; 1:true
	int GetLastTick(std::vector<DepthMarketDataField> &ticksVec, std::string strTradingDay);

	// 结算信息
	bool UpdDailySettlementFlag(DailySettlementFlagField &field);
	// return -1:faild; 0: no data; 1:true
	int GetLastDailySettlementFlag(DailySettlementFlagField &field);

	// 持仓明细信息
	bool UpdPositionDetail(InvestorPositionDetailField &positionDetail);
	bool DelPositionDetail(InvestorPositionDetailField &positionDetail);
	bool DelZeroPositionDetails();
	bool GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strBrokerID, string strInvestorID);
	bool GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strBrokerID, string strInvestorID, string strInstrumentID);
	bool GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strBrokerID, string strInvestorID, string strInstrumentID, string strDirection);
	bool GetPositionDetails(vector<InvestorPositionDetailField> &fieldsVec, string strInstrumentID);
	bool DelInvestorPositionDetails(string strBrokerID, string strInvestorID);

	// 是否有内部出入金权限
	bool IsHasDepositAuth(string strInvestorID);

	// 查询账号范围
	bool GetServerInvestorRange(string strServerID, long long &from, long long &to);

	// 自动止损信息
	bool UpdAutoStopLoss(AutoStopLossField &field);
	bool DelAutoStopLoss(const string strBrokerID, const string strInvestorID, const string strInstrumentID);
	bool GetAutoStopLoss(vector<AutoStopLossField> &fieldVec, const string strBrokerID, const string strInvestorID, const string strInstrumentID);

	// 交易风险揭示书信息
	bool UpdTradeRiskTmpl_Prepare(TradeRiskTmplField &field);
	bool UpdTradeRiskTmpl(TradeRiskTmplField &field);
	bool GetTradeRiskTmpl(TradeRiskTmplField &field, const string strBrokerID, const string strInvestorID, int clientType = 0);
	bool DelInvestorTradeRiskTmpl(string strBrokerID, string strInvestorID);

	// 交易柜台信息
	bool UpdTradeCounter(TradeCounterField &fieldVec);
	bool GetTradeCounter(vector<TradeCounterField> &fieldVec);

	//执行宣告信息
	bool UpdExecOrder(ExecOrderField &execOrder);
	bool CleanExecOrder();
	bool GetExecOrder(vector<ExecOrderField> &execOrdersVec, string strBrokerID, string strInvestorID, string strTradingDay);
	bool GetAllExecOrder(vector<ExecOrderField> &execOrdersVec, string strTradingDay);
	bool GetAllExecOrder(vector<ExecOrderField> &execOrdersVec);

	//历史执行宣告信息
	bool UpdHisExecOrder(ExecOrderField &execOrder);
	bool GetHisExecOrder(vector<ExecOrderField> &execOrdersVec, string strBrokerID, string strInvestorID, string strTradingDay);
	bool GetAllHisExecOrder(vector<ExecOrderField> &execOrdersVec, string strTradingDay);

private:
	void ConnectDB();
//	void CheckConnectionStatus(void *pParam);

private:
    CDataBase m_DataBase;
    string m_Host;
    string m_User;
    string m_Passwd;
    string m_DBName;
    unsigned int m_Port;
    unsigned int m_ClientFlag;
//    boost::shared_ptr<TimedTask> m_pCheckConnectionStatusTimedTask;
};



#endif /* DBOPERATION_H_ */
