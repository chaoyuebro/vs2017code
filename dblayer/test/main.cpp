/*
 * main.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: fuyf
 */

#include <boost/shared_ptr.hpp>
#include "debug.h"
#include "benchmark.h"
#include "HiRedisConnection.h"
#include "RedisConnectionPool.h"
#include "RedisUserDAO.h"

using namespace std;

void simple_test();
void redisdao_test();

int main()
{
    DEBUG << "Start Benchmark Test..." << std::endl;

    redisdao_test();
//    simple_test();

    return 0;
}

void zset_test1(boost::shared_ptr<HiRedisConnection> pConn);
void simple_test()
{
    boost::shared_ptr<HiRedisConnection> pConn(new HiRedisConnection("192.168.66.68", 29379));
    pConn->Connect();

//    set_test1(pConn);
//    get_test1(pConn);
    zset_test1(pConn);
}


void redisdao_zset_test1(RedisConnectionPool &pool);
void redisdao_test()
{
    RedisConnectionPool pool("192.168.66.68", 29379, 1);

    redisdao_zset_test1(pool);
}




void zset_test1(boost::shared_ptr<HiRedisConnection> pConn)
{
    DEBUG << "ZSet test1 end" <<endl;

    /// * UserSubAccount:[UserCode]:UserZSet        zset(score: SubAccountID, value: UserSubAccountID)

    std::string table("UserSubAccount");
    std::vector<std::string> cond { "20001" };

    std::vector<std::string> rlt;

    int total = 300000;
    BEGIN_BENCHMARK();
    for(int i=0; i<total; ++i)
    {
        DEFINE_BENCHMARK_MAX_MIN(range_zset_max_interval, range_zset_min_interval);
        BEGIN_BENCHMARK();


        DBCommand cmd;
        bool ret = RangeZSetCommand(cmd.inner, table, cond, "UserZSet", 0, -1);  //
        ASSERT(ret);

        DBCommandVec cmds { cmd };

        DBOResponseVec rv = pConn->Execute(cmds, 1000);

        std::vector<std::string> result;
        RangeZSetResult(result, rv[0]);

        for(auto it=rv.begin(); it!=rv.end(); ++it)
            FreeDBOResponse(*it);


        END_BENCHMARK();
        SHOW_BENCHMARK_MAX_MIN(range_zset_max_interval, range_zset_min_interval);
    }
    END_BENCHMARK();

    INFO << "Total costs: " << TIME_INTERVAL_US() << "us, avg: " << (float)TIME_INTERVAL_US()/total << std::endl;


    INFO << "ZSet test1 end" <<endl;
}


void redisdao_zset_test1(RedisConnectionPool &pool)
{
    DEBUG << "RedisDAO ZSet test1 end" <<endl;

    /// * UserSubAccount:[UserCode]:UserZSet        zset(score: SubAccountID, value: UserSubAccountID)

    RedisUserDAO userdao(pool);

    int total = 300000;
    BEGIN_BENCHMARK();

    for(int i=0; i<total; ++i)
    {
        DEFINE_BENCHMARK_TOTAL_COUNT(range_zset_total, range_zset_counter);
        DEFINE_BENCHMARK_MAX_MIN(dao_range_zset_max_interval, dao_range_zset_min_interval);
        BEGIN_BENCHMARK();

        userdao.FindSubAccounts("20001");

        END_BENCHMARK();
        SHOW_BENCHMARK_MAX_MIN(dao_range_zset_max_interval, dao_range_zset_min_interval);
        INCR_BENCHMARK_TOTAL_COUNT(range_zset_total, range_zset_counter);
        if(range_zset_counter % 100 == 0)
            SHOW_BENCHMARK_TOTAL_COUNT_AVG(range_zset_total, range_zset_counter);

//        usleep(4000);
    }

    END_BENCHMARK();

    INFO << "Total costs: " << TIME_INTERVAL_US() << "us, avg: " << (float)TIME_INTERVAL_US()/total << std::endl;


    INFO << "RedisDAO ZSet test1 end" <<endl;
}
