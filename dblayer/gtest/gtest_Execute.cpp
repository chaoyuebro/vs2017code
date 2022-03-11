/*
 * gtest_Execute.cpp
 *
 *  Created on: May 21, 2014
 *      Author: fuyf
 */

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <pbserdes/pbserdes.h>
#include "gtest/gtest.h"
#include "AsioRedisConnection.h"
#include "RedisCommandInterface.h"
#include "HiRedisConnection.h"
#include "RedisConnectionPool.h"
#include "../src/benchmark.h"
#include "sched.h"

using namespace std;

#if 0
TEST(ExecuteTest, Test1)
{
    cout << "Begin to allocate resource" << endl;

    boost::asio::io_service ios;
    boost::asio::io_service::work work(ios);
    boost::thread(boost::bind(&boost::asio::io_service::run, &ios));

    boost::shared_ptr<AsioRedisConnection> pConn(new AsioRedisConnection(ios, "127.0.0.1", 9379));

    pConn->Connect();


    std::string table("user");
    std::vector<std::string> cond;
    long long id = 1;


    sleep(5);
    cout << "Begin to add" << endl;

    int freetimes = 0;
    while(id++ <= 5)
    {
        std::string useridStr(std::to_string(id));
        DBCommand_ insZSetCmd_, insDetailCmd_, incrByFloatCmd_;
        bool ret = (InsZSetCommand(insZSetCmd_, table, cond, "zset", useridStr, useridStr)
                && InsDetailCommand(insDetailCmd_, table, id, "sddsf")
                && IncrByFloatCommand(incrByFloatCmd_, table, cond, "value", 1.0));

        ASSERT_TRUE(ret);

        DBCommand insZSetCmd, insDetailCmd, incrByFloatCmd;
        insZSetCmd.inner = insZSetCmd_;
        insDetailCmd.inner = insDetailCmd_;
        incrByFloatCmd.inner = incrByFloatCmd_;

        DBCommandVec cmds;
        cmds.push_back(insZSetCmd);
        cmds.push_back(insDetailCmd);
        cmds.push_back(incrByFloatCmd);

        DBOResponseVec rv = pConn->Execute(cmds, 1000);

        for(auto it=rv.begin(); it!=rv.end(); ++it)
        {
            FreeDBOResponse(*it);
            ++freetimes;
        }
    }

    cout << freetimes << endl;

    cout << "End Adding" <<endl;
    sleep(600);
}


TEST(ExecuteTest, Test2)
{
    cout << "Begin to allocate resource" << endl;

    boost::shared_ptr<HiRedisConnection> pConn(new HiRedisConnection("127.0.0.1", 9379));

    pConn->Connect();

    std::string table("user");
    std::vector<std::string> cond;
    long long id = 1;

    cout << "Begin to add" << endl;

    int freetimes = 0;
    while(id++ <= 5)
    {
        std::string useridStr(std::to_string(id));
        DBCommand_ insZSetCmd_, insDetailCmd_, incrByFloatCmd_;
        bool ret = (InsZSetCommand(insZSetCmd_, table, cond, "zset", useridStr, useridStr)
                && InsDetailCommand(insDetailCmd_, table, id, "sddsf")
                && IncrByFloatCommand(incrByFloatCmd_, table, cond, "value", 1.0));

        ASSERT_TRUE(ret);

        DBCommand insZSetCmd, insDetailCmd, incrByFloatCmd;
        insZSetCmd.inner = insZSetCmd_;
        insDetailCmd.inner = insDetailCmd_;
        incrByFloatCmd.inner = incrByFloatCmd_;

        DBCommandVec cmds;
        cmds.push_back(insZSetCmd);
        cmds.push_back(insDetailCmd);
        cmds.push_back(incrByFloatCmd);

        DBOResponseVec rv = pConn->Execute(cmds, 1000);

        for(auto it=rv.begin(); it!=rv.end(); ++it)
        {
            FreeDBOResponse(*it);
            ++freetimes;
        }
    }

    cout << freetimes << endl;

    cout << "End Adding" <<endl;
    sleep(600);
}
#endif

TEST(ExecuteTest, Test3)
{
    cout << "Begin to allocate resource" << endl;

    boost::shared_ptr<HiRedisConnection> pConn(new HiRedisConnection("127.0.0.1", 9379));

    pConn->Connect();

    std::string table("test");
    std::vector<std::string> cond;

    cout << "Begin to test" << endl;

    {
    for(int i=0; i<10000000; ++i)
    {
        //Increase count on user table
        DBCommand cmd;
        if(IncrCountCommand(cmd, table))
        {
            DBCommandVec cmds;
            cmds.push_back(cmd);
            DBOResponseVec rsps = pConn->Execute(cmds, 1000);
            for(auto it=rsps.begin(); it!=rsps.end(); ++it)
                FreeDBOResponse(*it);
        }
    }
    }

    cout << "End Testing" <<endl;
    sleep(600);
}

std::string GetLuaSHA1(AbstractRedisConnectionPtr pConn, std::string script)
{
    DBCommand cmd(MakeRedisCommand());
    cmd->append("script");
    cmd->append("load");
    cmd->append(script);

    DBCommandVec cmds { cmd };
    DBOResponseVec rspVec = pConn->Execute(cmds, 10000);
    if(rspVec.size() == 1)
    {
        std::string sret;
        int iret = StringResult(sret, rspVec[0]);
        if(iret <= 0)
            return "";
        else
            return sret;
    }
    return "";
}

TEST(ExecuteTest, Test4)
{
  #if 1
    pid_t pid = getpid();
    //printf("Default schedule policy: %d\n", sched_getscheduler(pid));
    struct sched_param param;
    param.__sched_priority = 50;
    if(-1 == sched_setscheduler(pid, SCHED_FIFO, &param))
        perror("setscheduler");
    //printf("After set, schedule policy: %d\n", sched_getscheduler(pid));
  #endif

    cout << "Begin to allocate resource" << endl;

//    boost::shared_ptr<HiRedisConnection> pConn(new HiRedisConnection("127.0.0.1", 9379));
    RedisConnectionPool pool("127.0.0.1", 9379);

    std::string table("test");
    std::vector<std::string> cond;

    cout << "Begin to test" << endl;

    TradingAccountField field;
    memset(&field, '1', sizeof(field));

    std::string pbstring;
    if(!ProtobufSerializer::Serialize(field, pbstring))
        std::cerr << "Serialize failed" << std::endl;

    std::string lua_string =
            "local id = redis.call('get', 'SubAccountEquity:' .. KEYS[1] .. ':id')\n" \
            "if not id then\n" \
            "        return nil\n" \
            "end\n" \
            "if #id == 0 then\n" \
            "        return nil\n" \
            "else\n" \
            "        local index = tonumber(id) / 1024\n" \
            "        redis.call('hset', 'SubAccountEquity:detail:' .. tostring(math.floor(index)), id, KEYS[2])\n" \
            "        return id\n" \
            "end";

    static std::string lua_sha1;
    if(lua_sha1.empty())
    {
    	auto pConn = pool.GetSyncRedisConn();
        if((lua_sha1 = GetLuaSHA1(pool.GetSyncRedisConn(), lua_string)).empty())
            std::cerr << "GetLuaSHA1 error" << std::endl;
        pool.RecycleSyncRedisConn(pConn);
    }


    {
        /// build redis command
        DBCommand cmd(MakeRedisCommand());
        cmd->append("evalsha");
        cmd->append(lua_sha1);
        cmd->append("2");
        cmd->append("9999-015041-000");
        cmd->append(pbstring);

        /// execute the command
        DBCommandVec cmds;
        cmds.push_back(cmd);
        	auto pConn = pool.GetSyncRedisConn();
    for(int i=0; i<10000000; ++i)
    {
        DBOResponseVec rsps;
        {
        	Benchmark b("Execute", 1000);
        rsps = pConn->Execute(cmds, 100);
        }
        for(auto it=rsps.begin(); it!=rsps.end(); ++it)
            FreeDBOResponse(*it);
        usleep(100);
    }
        pool.RecycleSyncRedisConn(pConn);
    }

    cout << "End Testing" <<endl;
    sleep(600);
}

TEST(ExecuteTest, Test5)
{
    cout << "Begin to allocate resource" << endl;

    boost::shared_ptr<HiRedisConnection> pConn(new HiRedisConnection("127.0.0.1", 9379));

    pConn->Connect();

    std::string table("test");
    std::vector<std::string> cond;

    cout << "Begin to test" << endl;

    PositionDBField field;
    memset(&field, '1', sizeof(field));

    std::string pbstring;
    if(!ProtobufSerializer::Serialize(field, pbstring))
        std::cerr << "Serialize failed" << std::endl;

    std::cout << "pbstring size:" << pbstring.size() << std::endl;
    pbstring = "trade.PositionDB%%adxbadxx01adxx12adxxb7adxx03adxnadxx1frb1605adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx12adxx0badxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx1aadxx1e9999-015041-000adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxadxx012*adxx0112adxx0118adxx00@adxx00Hadxxa1adxx9cadxx01Padxx00Yadxx00adxx00adxx00 adxxd2adxxd7adxxa7Aaadxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00hadxx00padxx00yadxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx81adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx89adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx91adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx99adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxa1adxx01adxx00adxx00adxx00adxx00@adxx88adxxd3@adxxa9adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxb1adxx01adxxa5adxx19adxx85adxxebQadxx00i@adxxb9adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxc1adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxc9adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxd1adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxd9adxx01adxx00adxx00adxx00adxx00adxx00adxadxadxx9c@adxxe1adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxeaadxx01adxt20160201adxx00adxxf0adxx01adxx00adxxf9adxx01adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx81adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx88adxx02adxx00adxx90adxx02adxx00adxx98adxx02adxx00adxxa1adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxa9adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxb0adxx02adxx00adxxb9adxx02-Cadxx1cadxxebadxxe26adxx1a?adxxc1adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxc9adxx02adxx8dadxxedadxxb5adxxa0adxxf7adxxc6adxxb0>adxxd1adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxd9adxx02adxx8dadxxedadxxb5adxxa0adxxf7adxxc6adxxb0>adxxe1adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxe9adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxf1adxx02adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxxf8adxx02adxnadxx82adxx03adxtSHFEadxx00adxx00adxx00adxx00adxx00adxx88adxx03adxx00adxx91adxx03adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx00adxx98adxx03adxx00adxxa0adxx03adxx00";

    std::cout << "pbstring size again:" << pbstring.size() << std::endl;
    {
    for(int i=0; i<10000000; ++i)
    {
        /// build redis command
        DBCommand cmd(MakeRedisCommand());
        cmd->append("hset");
        cmd->append("SubAccountPosition:detail:0");
        cmd->append("1025");
        cmd->append(pbstring);

        /// execute the command
        DBCommandVec cmds;
        cmds.push_back(cmd);
        DBOResponseVec rsps;
        {
            Benchmark b("Execute", 10000);
        rsps = pConn->Execute(cmds, 1000);
        }
        for(auto it=rsps.begin(); it!=rsps.end(); ++it)
            FreeDBOResponse(*it);
    }
    }

    cout << "End Testing" <<endl;
    sleep(600);
}

