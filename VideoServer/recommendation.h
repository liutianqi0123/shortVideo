#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <hiredis/hiredis.h>
#include <RedisTools.h>
#include <numeric>
#include <algorithm>
#include <list>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

class Recommendation
{
public:
    Recommendation();
    ~Recommendation();
    vector<string> GetUserTag(string key);
    vector<int> GetSameUser(vector<string> &tag);

    double Person(vector<string> &inst1, vector<string> &inst2);


    vector<int> ComparedUserID(vector<int> &user1, vector<int> &user2);
    //获取相似度数组
    vector<double> GetSimilarUsers(vector<int> &users,string key);

    int TheMostFamilier(vector<double> &scores);
    //相似度数组TheSimilarScores进行降序排列，找到最相似的几个用户
    vector<string> sortSimilarUser(vector<double> &scores);
    //在redis中找到相似用户的id并且通过id找到该用户的视频id的数组
    vector<string> getVideoID(vector<string> &userid);
    //找到视频数组中该用户没有看过的并且得分很高的用户的视频进行视频推荐

    //维护一个视频池存满为200+的视频量，随时给用户进行推荐
private:
    RedisTool redis;
    list <vector<string>> useridStringlist;
    list <vector<int>> useridIntlist;
//    vector <string> TheSameUsers;
};

#endif // RECOMMENDATION_H
