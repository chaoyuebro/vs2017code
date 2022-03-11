/*
 * gtest_UserDAO.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: fuyf
 */

#include <pbserdes/pbserdes.h>
#include <boost/lexical_cast.hpp>
#include "gtest/gtest.h"
#include "DSOBuilder.h"

using namespace std;

class RedisUserDAO : public RedisDAO
{
public:
    RedisUserDAO(RedisConnectionPool &pool)
    : RedisDAO(pool)
    {}
    virtual ~RedisUserDAO()
    {}

#define USER_TABLE "user"
    int AddUser(const UserField &field)
    {
        int timeout = 1000;

        /// get count
        int userid = IncrCountFunc(USER_TABLE);
        if(userid < 0)
        {
            cerr << "Incr count failed!" << endl;
            return -1;
        }

        /// set strategy id
        UserField &ncfield = const_cast<UserField &>(field);
        ncfield.UserID = userid;

        /// add to zset and add detail
        // serialize data
        std::string pbstring;
        if(!ProtobufSerializer::Serialize(ncfield, pbstring))
        {
            cerr << "Serialize failed!" << endl;
            return -1;
        }

        // get id string
        std::string useridStr(boost::lexical_cast<std::string>(userid));

        // get table
        std::string table(USER_TABLE);

        // get cond for zset
        std::vector<std::string> cond; // in this case, cond is null
#if 0
/**
        DBCommand insDetailCmd;
        bool ret = InsDetailCommand(insDetailCmd, table, userid, pbstring);
        if(!ret)
        {
            cerr << "Get Redis command failed!" << endl;
            return -1;
        }
        bool insDetailRlt;
        int s1=0;
        ret = Execute(insDetailCmd, insDetailRlt, s1, timeout);
        ret = (ret && (s1>0));
        if(!ret)
        {
            cerr << "Execute Redis command failed!" << endl;
            return -1;
        }
*/
        DBCommand insZSetCmd;
        bool ret = InsZSetCommand(insZSetCmd, table, cond, "zset", useridStr, useridStr);
        if(!ret)
        {
            cerr << "Get Redis command failed!" << endl;
            return -1;
        }
        long long insZSetRlt;
        int s1=0;
        ret = Execute(insZSetCmd, insZSetRlt, s1, timeout);
        ret = (ret && (s1>0));
        if(!ret)
        {
            cerr << "Execute Redis command failed!" << endl;
            return -1;
        }
#else
        DBCommand insZSetCmd, insDetailCmd;
        bool ret = (InsZSetCommand(insZSetCmd, table, cond, "zset", useridStr, useridStr)
                && InsDetailCommand(insDetailCmd, table, userid, pbstring));
        if(!ret)
        {
            cerr << "Get Redis command failed!" << endl;
            return -1;
        }
        long long insZSetRlt;
        bool insDetailRlt;
        int s1=0, s2=0;
        ret = Execute(insZSetCmd, insDetailCmd, insZSetRlt, insDetailRlt, s1, s2, timeout);
        ret = (ret && (s1>0) && (s2>0));
        if(!ret)
        {
            cerr << "Execute Redis command failed!" << endl;
            return -1;
        }
#endif

        return userid;
    }
};
typedef boost::shared_ptr<RedisUserDAO> RedisUserDAOPtr;

class UserDSO
{
public:
    static boost::shared_ptr<UserDSO> CreateDSO(DSOBuilder &builder)
    {
        return builder.CreateDSO<UserDSO, RedisUserDAO>();
    }

    UserDSO(RedisUserDAOPtr redisDAO)
    : mRedisDAO(redisDAO)
    {}
    virtual ~UserDSO()
    {}

    int AddUser(const UserField &field)
    {
        return mRedisDAO->AddUser(field);
    }

private:
    RedisUserDAOPtr mRedisDAO;
};
typedef boost::shared_ptr<UserDSO> UserDSOPtr;

class TestUserDSOFixture : public testing::Test
{
public:
    TestUserDSOFixture()
    : pool("127.0.0.1", 9379, 10),
      dsobuilder(pool)
    {
        sleep(1);
    }

    virtual ~TestUserDSOFixture()
    {
    }

protected:
    RedisConnectionPool pool;
    DSOBuilder dsobuilder;
};

TEST_F(TestUserDSOFixture, Test1)
{
    cout << "Begin to allocate resource" << endl;

    UserDSOPtr userdsoPtr = UserDSO::CreateDSO(dsobuilder);
    ASSERT_TRUE(userdsoPtr);

    UserField *pUser = new UserField();
    UserField &user = *pUser;
    user.UserCode = "10000";
    user.Address = "sdfsdfsfdsfsdfffffffffffsdfssssssssssssssssssssssssssssssssssssssssssssssssssssssssss/sdfsdfjsljdflskjdfkjsldfksjfdls";
    user.Comment = "sdfsssssssssssssssffffffffffffffffffffffffffffffffffffffffffffffffdddddddddddddddddddddddddddddddddddddddddddddd";
    user.Name = "sdfsdfsdsssssssssssssssssssssssssssssssssssssssssssss";

    cout << "Begin to add" << endl;

    int i=1;
    while(i-- > 0)
    {
        int ret = userdsoPtr->AddUser(user);
        ASSERT_GT(ret, 0);
    }

    cout << "End Adding" <<endl;
    delete pUser;
    userdsoPtr.reset();

    sleep(600);
}

