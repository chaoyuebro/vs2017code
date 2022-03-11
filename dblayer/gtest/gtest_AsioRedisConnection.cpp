/*
 * gtest_RedisDBOperator.cpp
 *
 *  Created on: Jul 23, 2013
 *      Author: fuyf
 */
#if 0
#include <string>
#include <set>
#include <boost/thread.hpp>
#include "gtest/gtest.h"
#include "debug.h"
#include "AsioRedisConnection.h"
#include "RedisCommandInterface.h"
#include "hiredis/hiredis.h"

enum RedisCommandType
{
    INVALID_REDIS_TYPE = 0,

    INCR_COUNT = 1,

    MAX_REDIS_COMMAND_TYPE
};

class AsioRedisConnectionTest : public ::testing::Test
{
public:
    AsioRedisConnectionTest(std::string ip="127.0.0.1", int port=8379)
    : mIos()
    {
        mDBConnPtr.reset(new AsioRedisConnection(mIos, ip, port));
        mDBConnPtr->RegisterDisconnectCallback(boost::bind(&AsioRedisConnectionTest::DisconnectDBCallback, this));

        ConnectDB();
        boost::thread t(boost::bind(&AsioRedisConnectionTest::StartIOService, this, &mIos));
    }
    ~AsioRedisConnectionTest()
    {}

    void ConnectDB()
    {
        if(!DBConnected())
        {
            mDBConnPtr->Connect(boost::bind(&AsioRedisConnectionTest::ConnectDBCallback, this, _1, _2));
        }
    }
    void ConnectDBCallback(int code, std::string message)
    {
        if(code == 0)
        {
            DEBUG << "Connect to Redis DB successfully" << std::endl;
        }
        else
        {
            ERROR << "Connect to Redis DB error! Error code: " << code << "; Error message: " << message << std::endl;
        }
    }
    void DisconnectDBCallback()
    {
        ERROR << "Disconnected from Redis DB" << std::endl;
    }

    bool DBConnected()
    {
        return mDBConnPtr->DBConnected();
    }
    void StartIOService(boost::asio::io_service *ios)
    {
        boost::asio::io_service::work work(*ios);
        ios->run();

        ERROR << "Reach here is not a good thing!!!" << std::endl;
    }

    void CheckRedisReply(const redisReply *reply)
    {
        if(reply->type == REDIS_REPLY_ERROR)
        {
            ERROR << "Redis command error! Detail: " << std::string(reply->str, reply->len) << std::endl;
        }
        else if(reply->type == REDIS_REPLY_INTEGER)
        {
            DEBUG << "Redis command Integer reply: " << reply->integer << std::endl;
        }
        else if(reply->type == REDIS_REPLY_NIL)
        {
            WARN << "Redis command returned a NULL reply" << std::endl;
        }
        else if(reply->type == REDIS_REPLY_STATUS)
        {
            DEBUG << "Redis command Status reply! Detail: " << std::string(reply->str, reply->len) << std::endl;
        }
        else if(reply->type == REDIS_REPLY_STRING)
        {
            DEBUG << "Redis command String reply! Detail: " << std::string(reply->str, reply->len) << std::endl;
        }
        else if(reply->type == REDIS_REPLY_ARRAY)
        {
            DEBUG << "Redis command MultiBulk reply! " << std::endl << "<<<<<<<<<<<<<<<" << std::endl;
            for(size_t i=0; i<reply->elements; i++)
            {
                CheckRedisReply(*(reply->element+i));
            }
            DEBUG << std::endl << ">>>>>>>>>>>>>>>" << std::endl;
        }
    }

    void IncrCountCallback(int expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(IncrCountResult(result, reply));
        DEBUG << "IncrCountResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void GetCountCallback(std::string expect, RedisReply reply)
    {
        std::string result;
        ASSERT_TRUE(GetCountResult(result, reply));
        DEBUG << "GetCountResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void GetIDCallback(std::string expect, RedisReply reply)
    {
        std::string result;
        ASSERT_TRUE(GetIDResult(result, reply));
        DEBUG << "GetIDResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void InsIDCallback(std::string expect, RedisReply reply)
    {
        std::string result;
        ASSERT_TRUE(InsIDResult(result, reply));
        DEBUG << "GetIDResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void DelIDCallback(int expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(DelIDResult(result, reply));
        DEBUG << "DelIDResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void InsDetailCallback(bool expect, RedisReply reply)
    {
        bool result;
        ASSERT_TRUE(InsDetailResult(result, reply));
        DEBUG << "InsDetailResult is " << std::boolalpha << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void UpdDetailCallback(bool expect, RedisReply reply)
    {
        bool result;
        ASSERT_TRUE(UpdDetailResult(result, reply));
        DEBUG << "UpdDetailResult is " << std::boolalpha << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void GetDetailCallback(std::string expect, RedisReply reply)
    {
        std::string result;
        ASSERT_TRUE(GetDetailResult(result, reply));
        DEBUG << "GetDetailResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void DelDetailCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(DelDetailResult(result, reply));
        DEBUG << "DelDetailResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void RPushListCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(RPushListResult(result, reply));
        DEBUG << "RPushListResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void LenListCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(LenListResult(result, reply));
        DEBUG << "LenListResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void RangeListCallback(std::vector<std::string> &expect, RedisReply reply)
    {
        std::vector<std::string> result;
        ASSERT_TRUE(RangeListResult(result, reply));
        DEBUG << "RangeListResult ... " << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void LPopListCallback(std::string expect, RedisReply reply)
    {
        std::string result;
        ASSERT_TRUE(LPopListResult(result, reply));
        DEBUG << "LPopListResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void InsZSetCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(InsZSetResult(result, reply));
        DEBUG << "InsZSetResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void LenZSetCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(LenZSetResult(result, reply));
        DEBUG << "LenZSetResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void LenRangeZSetCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(LenRangeZSetResult(result, reply));
        DEBUG << "LenRangeZSetResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void RangeZSetCallback(std::vector<std::string> &expect, RedisReply reply)
    {
        std::vector<std::string> result;
        ASSERT_TRUE(RangeZSetResult(result, reply));
        DEBUG << "RangeZSetResult ..." << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void SRangeZSetCallback(std::vector<std::string> &expect, RedisReply reply)
    {
        std::vector<std::string> result;
        ASSERT_TRUE(SRangeZSetResult(result, reply));
        DEBUG << "SRangeZSetResult ..." << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }
    void DelZSetElemCallback(long long expect, RedisReply reply)
    {
        long long result;
        ASSERT_TRUE(DelZSetElemResult(result, reply));
        DEBUG << "DelZSetElemResult is " << result << std::endl;
        ASSERT_EQ(expect, result);

        FreeDBOResponse(reply);
    }


    void ErrorRedisCallback(std::vector<std::string> cmd, redisReply * reply)
    {
        std::cout << "=== Command: ";
        for(size_t i=0; i<cmd.size(); i++)
            std::cout << cmd[i] << " ";
        std::cout << "===" << std::endl;

        CheckRedisReply(reply);
        ASSERT_EQ(REDIS_REPLY_ERROR, reply->type);
        //must free by user
        FreeDBOResponse(reply);
    }
    void IntegerRedisCallback(std::vector<std::string> cmd, redisReply *reply, long long expect)
    {
        std::cout << "=== Command: ";
        for(size_t i=0; i<cmd.size(); i++)
            std::cout << cmd[i] << " ";
        std::cout << "===" << std::endl;

        CheckRedisReply(reply);
        ASSERT_EQ(REDIS_REPLY_INTEGER, reply->type);
        ASSERT_EQ(expect, reply->integer);
        //must free by user
        FreeDBOResponse(reply);
    }
    void NilRedisCallback(std::vector<std::string> cmd, redisReply *reply)
    {
        std::cout << "=== Command: ";
        for(size_t i=0; i<cmd.size(); i++)
            std::cout << cmd[i] << " ";
        std::cout << "===" << std::endl;

        CheckRedisReply(reply);
        ASSERT_EQ(REDIS_REPLY_NIL, reply->type);
        //must free by user
        FreeDBOResponse(reply);
    }

    void StatusRedisCallback(std::vector<std::string> cmd, redisReply *reply, std::string expect)
    {
        std::cout << "=== Command: ";
        for(size_t i=0; i<cmd.size(); i++)
            std::cout << cmd[i] << " ";
        std::cout << "===" << std::endl;

        CheckRedisReply(reply);
        ASSERT_EQ(REDIS_REPLY_STATUS, reply->type);
        ASSERT_EQ(expect, std::string(reply->str, reply->len));
        //must free by user
        FreeDBOResponse(reply);
    }

    void StringRedisCallback(std::vector<std::string> cmd, redisReply *reply, std::string expect)
    {
        std::cout << "=== Command: ";
        for(size_t i=0; i<cmd.size(); i++)
            std::cout << cmd[i] << " ";
        std::cout << "===" << std::endl;

        CheckRedisReply(reply);
        ASSERT_EQ(REDIS_REPLY_STRING, reply->type);
        ASSERT_EQ(expect, std::string(reply->str, reply->len));
        //must free by user
        FreeDBOResponse(reply);
    }

    void ArrayRedisCallback(std::vector<std::string> cmd, redisReply *reply, size_t elements)
    {
        std::cout << "=== Command: ";
        for(size_t i=0; i<cmd.size(); i++)
            std::cout << cmd[i] << " ";
        std::cout << "===" << std::endl;

        CheckRedisReply(reply);
        ASSERT_EQ(REDIS_REPLY_ARRAY, reply->type);
        ASSERT_EQ(elements, reply->elements);
        //must free by user
        FreeDBOResponse(reply);
    }


public:
    boost::asio::io_service mIos;
    AsioRedisConnectionPtr mDBConnPtr;
};

#ifdef GTEST_ALL
TEST_F(AsioRedisConnectionTest, Test1)
{
    //wait for db connected
    while(!mDBConnPtr->DBConnected());

    std::vector<std::string> args;

    args.clear();
    args.push_back("flushdb");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StatusRedisCallback, this, args, _1, "OK"), args);

    args.clear();
    args.push_back("set");
    args.push_back("k1");
    args.push_back("v1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StatusRedisCallback, this, args, _1, "OK"), args);

    args.clear();
    args.push_back("set");
    args.push_back("k1");
    args.push_back("v1");
    args.push_back("nx");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::NilRedisCallback, this, args, _1), args);

    args.clear();
    args.push_back("get");
    args.push_back("k1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StringRedisCallback, this, args, _1, "v1"), args);

    args.clear();
    args.push_back("incr");
    args.push_back("count");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("get");
    args.push_back("count");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StringRedisCallback, this, args, _1, "1"), args);

    args.clear();
    args.push_back("zadd");
    args.push_back("zset");
    args.push_back("1");
    args.push_back("1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("zadd");
    args.push_back("zset");
    args.push_back("2");
    args.push_back("2");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("zcard");
    args.push_back("zset");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("zrangebyscore");
    args.push_back("zset");
    args.push_back("1");
    args.push_back("2");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::ArrayRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("zrange");
    args.push_back("zset");
    args.push_back("0");
    args.push_back("-1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::ArrayRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("zcount");
    args.push_back("zset");
    args.push_back("0");
    args.push_back("4");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("zrem");
    args.push_back("zset");
    args.push_back("1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("zrangebyscore");
    args.push_back("zset");
    args.push_back("1");
    args.push_back("3");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::ArrayRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("zremrangebyscore");
    args.push_back("zset");
    args.push_back("0");
    args.push_back("5");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("zcard");
    args.push_back("zset");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 0), args);

    args.clear();
    args.push_back("hsetnx");
    args.push_back("detail");
    args.push_back("1");
    args.push_back("detail1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("hset");
    args.push_back("detail");
    args.push_back("1");
    args.push_back("detail11");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 0), args);

    args.clear();
    args.push_back("hget");
    args.push_back("detail");
    args.push_back("1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StringRedisCallback, this, args, _1, "detail11"), args);

    args.clear();
    args.push_back("hdel");
    args.push_back("detail");
    args.push_back("1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("rpush");
    args.push_back("list");
    args.push_back("1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 1), args);

    args.clear();
    args.push_back("rpush");
    args.push_back("list");
    args.push_back("2");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("lrange");
    args.push_back("list");
    args.push_back("0");
    args.push_back("-1");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::ArrayRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("lrange");
    args.push_back("list");
    args.push_back("5");
    args.push_back("10");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::ArrayRedisCallback, this, args, _1, 0), args);

    args.clear();
    args.push_back("llen");
    args.push_back("list");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IntegerRedisCallback, this, args, _1, 2), args);

    args.clear();
    args.push_back("lpop");
    args.push_back("list");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StringRedisCallback, this, args, _1, "1"), args);

    args.clear();
    args.push_back("lpop");
    args.push_back("list");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StringRedisCallback, this, args, _1, "2"), args);


    sleep(2);
}
#endif

TEST_F(AsioRedisConnectionTest, Test2)
{
    //wait for db connected
    while(!mDBConnPtr->DBConnected());

    //First, flushdb
    std::vector<std::string> args;
    args.clear();
    args.push_back("flushdb");
    mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::StatusRedisCallback, this, args, _1, "OK"), args);


    RedisCommand cmd;

    if(IncrCountCommand(cmd, "test"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IncrCountCallback, this, 1, _1), cmd);
    }
    if(IncrCountCommand(cmd, "test"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::IncrCountCallback, this, 2, _1), cmd);
    }

    if(GetCountCommand(cmd, "test"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::GetCountCallback, this, "2", _1), cmd);
    }

    std::vector<std::string> cond;
    cond.clear();
    cond.push_back("123");
    if(InsIDCommand(cmd, "test", cond, "id", "1"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::InsIDCallback, this, "OK", _1), cmd);
    }

    cond.clear();
    cond.push_back("123");
    if(GetIDCommand(cmd, "test", cond, "id"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::GetIDCallback, this, "1", _1), cmd);
    }
    if(DelIDCommand(cmd, "test", cond, "id"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::DelIDCallback, this, 1, _1), cmd);
    }

    long long field = 1023;
    if(InsDetailCommand(cmd, "test", field, "testdetailfield-1023"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::InsDetailCallback, this, true, _1), cmd);
    }
    field = 1024;
    if(InsDetailCommand(cmd, "test", field, "testdetailfield-1024"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::InsDetailCallback, this, true, _1), cmd);
    }
    field = 1023;
    if(UpdDetailCommand(cmd, "test", field, "testdetailfield-1023-changed"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::UpdDetailCallback, this, true, _1), cmd);
    }
    field = 1025;
    if(UpdDetailCommand(cmd, "test", field, "testdetailfield-1025"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::UpdDetailCallback, this, false, _1), cmd);
    }
    field = 1024;
    if(GetDetailCommand(cmd, "test", field))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::GetDetailCallback, this, "testdetailfield-1024", _1), cmd);
    }
    field = 1024;
    if(DelDetailCommand(cmd, "test", field))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::DelDetailCallback, this, 1, _1), cmd);
    }
    field = 1025;
    if(DelDetailCommand(cmd, "test", field))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::DelDetailCallback, this, 1, _1), cmd);
    }
    field = 1026;
    if(DelDetailCommand(cmd, "test", field))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::DelDetailCallback, this, 0, _1), cmd);
    }


    //list test
    std::vector<std::string> listcond;
    listcond.clear();
    listcond.push_back("listcond1");
    listcond.push_back("listcond2");
    if(RPushListCommand(cmd, "test", listcond, "list", "listvalue1"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::RPushListCallback, this, 1, _1), cmd);
    }
    if(RPushListCommand(cmd, "test", listcond, "list", "listvalue2"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::RPushListCallback, this, 2, _1), cmd);
    }
    if(LenListCommand(cmd, "test", listcond, "list"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::LenListCallback, this, 2, _1), cmd);
    }
    std::vector<std::string> expectRangeList;
    expectRangeList.emplace_back("listvalue1");
    expectRangeList.emplace_back("listvalue2");
    if(RangeListCommand(cmd, "test", listcond, "list", -100, 10))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::RangeListCallback, this, expectRangeList, _1), cmd);
    }
    if(LPopListCommand(cmd, "test", listcond, "list"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::LPopListCallback, this, "listvalue1", _1), cmd);
    }


    //zset test
    std::vector<std::string> zsetcond;
    zsetcond.clear();
    zsetcond.push_back("zsetcond1");
    zsetcond.push_back("zsetcond2");
    if(InsZSetCommand(cmd, "test", zsetcond, "zset", "1", "zsetvalue1"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::InsZSetCallback, this, 1, _1), cmd);
    }
    if(InsZSetCommand(cmd, "test", zsetcond, "zset", "2", "zsetvalue2"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::InsZSetCallback, this, 1, _1), cmd);
    }
    if(LenZSetCommand(cmd, "test", zsetcond, "zset"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::LenZSetCallback, this, 2, _1), cmd);
    }
    if(LenRangeZSetCommand(cmd, "test", zsetcond, "zset", "1", "2"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::LenRangeZSetCallback, this, 2, _1), cmd);
    }
    std::vector<std::string> expectRangeZSet;
    expectRangeZSet.emplace_back("zsetvalue1");
    expectRangeZSet.emplace_back("zsetvalue2");
    if(RangeZSetCommand(cmd, "test", zsetcond, "zset", 0, 1))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::RangeZSetCallback, this, expectRangeZSet, _1), cmd);
    }
    if(SRangeZSetCommand(cmd, "test", zsetcond, "zset", "-inf", "+inf"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::SRangeZSetCallback, this, expectRangeZSet, _1), cmd);
    }
    if(DelZSetElemCommand(cmd, "test", zsetcond, "zset", "zsetvalue2"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::DelZSetElemCallback, this, 1, _1), cmd);
    }
    if(InsZSetCommand(cmd, "test", zsetcond, "zset", "-3", "zsetvalue3"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::InsZSetCallback, this, 1, _1), cmd);
    }
    if(DelSRangeZSetCommand(cmd, "test", zsetcond, "zset", "-1", "10"))
    {
        mDBConnPtr->RedisAsyncCommandVector(boost::bind(&AsioRedisConnectionTest::DelZSetElemCallback, this, 1, _1), cmd);
    }


    sleep(1);
}
#endif
