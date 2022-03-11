/*
 * RedisUserDAO.h
 *
 *  Created on: Jul 8, 2014
 *      Author: fuyf
 */

#ifndef REDISUSERDAO_H_
#define REDISUSERDAO_H_
#include <set>
#include <boost/shared_ptr.hpp>
#include <pbserdes/pbserdes.h>
#include "RedisDAO.h"

class RedisUserDAO : public RedisDAO
{
public:
    RedisUserDAO(RedisConnectionPool &pool);
    virtual ~RedisUserDAO();

    int FindUserID(std::string userCode);
    boost::shared_ptr<UserField> FindUser(int userid);
    std::set<int> FindSubAccounts(std::string userCode);
    int CountUser();
};

typedef boost::shared_ptr<RedisUserDAO> RedisUserDAOPtr;

#endif /* REDISUSERDAO_H_ */
