/*
 * RedisUserDAO.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: fuyf
 */

#include <ctsp/TKException.h>
#include <ctsp/Errors.h>
#include <ctsp/debug.h>
#include <boost/lexical_cast.hpp>
#include "RedisUserDAO.h"
#include "benchmark.h"

//--- tables
static const char * USER_TABLE = "User";
static const char * USERSUBACCOUNT_TABLE = "UserSubAccount";

RedisUserDAO::RedisUserDAO(RedisConnectionPool &pool)
: RedisDAO(pool)
{

}

RedisUserDAO::~RedisUserDAO()
{
    DEBUG_ENTRY();
}

int RedisUserDAO::FindUserID(std::string userCode)
{
    DEBUG_ENTRY();
    std::string idstring;
    std::vector<std::string> cond { userCode };

    int ret = GetIDFunc(idstring, USER_TABLE, cond, "id");
    if(ret < 0)
        BUG_FILE();

    if(ret == 0)
        return -1;

    int id = -1;
    try{
        id = boost::lexical_cast<int>(idstring);
    }
    catch(...)
    {
        BUG_FILE();
    }

    return id;
}

boost::shared_ptr<UserField> RedisUserDAO::FindUser(int userid)
{
    DEBUG_ENTRY();

    std::string detail;
    int ret = GetDetailFunc(detail, USER_TABLE, userid);
    if(ret < 0)
        BUG_FILE();

    if(ret == 0)
        return boost::shared_ptr<UserField>();

    boost::shared_ptr<UserField> pRet(new UserField);
    if(!ProtobufSerializer::Deserialize(*pRet, detail))
        BUG_FILE();

    return pRet;
}

std::set<int> RedisUserDAO::FindSubAccounts(std::string userCode)
{
    DEBUG_ENTRY();

    int timeout = 10000;

    /// 1. Use RangeZSetCommand to get UserSubaccountIDs
    /// * UserSubAccount:[UserCode]:UserZSet        zset(score: SubAccountID, value: UserSubAccountID)

    std::string table(USERSUBACCOUNT_TABLE);
    std::vector<std::string> cond { userCode };

    std::vector<std::string> rlt;

    DEFINE_BENCHMARK_TOTAL_COUNT(range_zset_total, range_zset_counter);
    DEFINE_BENCHMARK_MAX_MIN(range_zset_max_interval, range_zset_min_interval);
    BEGIN_BENCHMARK();


    DBCommand cmd;
    bool ret = RangeZSetCommand(cmd, table, cond, "UserZSet", 0, -1);  //
    if(!ret)
        BUG_FILE();

    int s1 = 0;
    ret = Execute(cmd, rlt, s1, timeout);
    ret = (ret && (s1>=0));
    if(!ret)
        BUG_FILE();

    END_BENCHMARK();
    SHOW_BENCHMARK_MAX_MIN(range_zset_max_interval, range_zset_min_interval);
    INCR_BENCHMARK_TOTAL_COUNT(range_zset_total, range_zset_counter);
    if(range_zset_counter % 100 == 0)
        SHOW_BENCHMARK_TOTAL_COUNT_AVG(range_zset_total, range_zset_counter);

#if 1

    {
    DEFINE_BENCHMARK_TOTAL_COUNT(get_score_total, get_score_counter);
    DEFINE_BENCHMARK_MAX_MIN(get_score_max_interval, get_score_min_interval);
    BEGIN_BENCHMARK();

    /// 2. Use UserSubaccountID to get its score(SubAccountID)
    ret = true;
    for(auto it=rlt.begin(); it!=rlt.end(); ++it)
    {
        DBCommand tmpcmd;
        std::string r1;
        int s1 = 0;
        ret = ret && GetScoreInZSetCommand(tmpcmd, table, cond, "UserZSet", *it) && Execute(tmpcmd, r1, s1, timeout);     //
        if(!ret)
            BUG_FILE();
    }

#if 0
    DBCommandVec cmds;
    bool ret = true;
    for(auto it=rlt.begin(); it!=rlt.end(); ++it)
    {
        DBCommand tmpcmd;
        ret = ret && GetScoreInZSetCommand(tmpcmd, table, cond, "UserZSet", *it);     //
        cmds.push_back(tmpcmd);
    }
    if(!ret)
        BUG_FILE();


    if(!cmds.empty())
    {
        std::vector<std::string> subaccountids;
        int errors = 0;
        ret = Execute(cmds, subaccountids, errors, timeout);
        ret = (ret && (errors == 0));
        if(!ret)
            BUG_FILE();

        std::set<int> rset;
        for(auto it=subaccountids.begin(); it!=subaccountids.end(); ++it)
        {
            try{
                int subaccountid = boost::lexical_cast<int>(*it);
                rset.insert(subaccountid);
            }
            catch(...)
            {
                ERROR << "Find SubAccounts for " << userCode << "error?!" << std::endl;
                continue;
            }
        }


        return rset;
    }
    else
    {
        return std::set<int>();
    }
#endif
    END_BENCHMARK();
    SHOW_BENCHMARK_MAX_MIN(get_score_max_interval, get_score_min_interval);
    INCR_BENCHMARK_TOTAL_COUNT(get_score_total, get_score_counter);
    if(get_score_counter % 100 == 0)
        SHOW_BENCHMARK_TOTAL_COUNT_AVG(get_score_total, get_score_counter);
    }

#endif
    return std::set<int>();
}

int RedisUserDAO::CountUser()
{
    DEBUG_ENTRY();

    int timeout = 10000;

    /// 1. Use LenZSetCommand to count user
    /// * User:zset               zset(score: UserID, value: UserID)

    std::string table(USER_TABLE);
    std::vector<std::string> cond;

    DBCommand cmd;
    bool ret = LenZSetCommand(cmd, table, cond, "zset");
    if(!ret)
        BUG_FILE();

    long long rlt;
    int s1 = 0;
    ret = Execute(cmd, rlt, s1, timeout);
    if(!ret)
        BUG_FILE();

    return (int)rlt;
}
