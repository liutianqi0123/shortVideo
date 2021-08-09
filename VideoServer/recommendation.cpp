#include "recommendation.h"

Recommendation::Recommendation()
{

}

Recommendation::~Recommendation()
{

}

//获取用户标签
vector<string> Recommendation::GetUserTag(string key)
{
    vector<string> str = redis.HashKeysContent(key);
    return str;
}

//获取相同标签的用户
vector<int> Recommendation::GetSameUser(vector<string> &tag)
{

    for(auto ite = tag.begin();ite<tag.end();ite++)
    {
        vector<string> userid = redis.HashKeysContent(*ite);
        useridStringlist.push_back(userid);
    }
    //将链表中的用户id全部转成int类型
    for(vector <string> list:useridStringlist)
    {
        vector<int> vec;
        std::transform(list.begin(),list.end(),std::back_inserter(vec),[](const std::string &str){return std::stoi(str);});//lambda expression
        useridIntlist.push_back(vec);
    }
    //比较所有用户列表输出一个含有与用户相同标签的用户列表
    vector<int> res = useridIntlist.front();
    
    for(vector<int> ite:useridIntlist)
    {
        res = this->ComparedUserID(ite,res);
    }
    

    return res;
}

//比较两个链表的userid 相似的id

vector<int> Recommendation::ComparedUserID(vector<int> &user1,vector<int> &user2 )
{
    vector<int> res;
    if(user1.size()<0 || user2.size()<0) return {};
  
    for(vector<int>::const_iterator nite = user1.begin();nite != user1.end();nite++)
    {
        if(std::find(user2.begin(),user2.end(),*nite) != user2.end())
            res.push_back(*nite);
    }
    return res;
}

//获取最相似的topX的用户

vector<double> Recommendation::GetSimilarUsers(vector<int> &users,string key)
{
    if(users.size()<0) return{};
    
    int size = users.size();
    //从redis中把key用户的分数取出来
    vector <string> userscores = redis.HashValsContent(key);
    vector <double> TheSimilarScores;

    //遍历相关用户id数组 计算两个用户id的皮尔逊相似度
    for(int i=1; i<size;i++)
    {
        //计算相似度
        vector <string> SameUsers = redis.HashValsContent(std::to_string(users[i]));
        double dou = this->Person(userscores,SameUsers);
        //把得到的相似度值与用户id关联存到redis中
        std::stringstream ss;
        ss << dou;
        redis.setString(ss.str(),std::to_string(users[i]));
        //把相似度值存到数组中进行排序找出相似度最高的用户
        TheSimilarScores.push_back(dou);
    }
    return TheSimilarScores;
}



//返回相似用户数组
vector<string> Recommendation::sortSimilarUser(vector<double> &scores)
{
    if(scores.size()<0) return {};
    vector<string> res;
    sort(scores.begin(),scores.end());
    reverse(scores.begin(),scores.end());


    for(double ite:scores)
    {
         std::stringstream ss;
         ss << ite;
         res.push_back(ss.str());
    }
    return res;
}
//找到相应用户的视频数组
vector<string> Recommendation::getVideoID(vector<string> &userid)
{
    if(userid.size() < 0)return {};
    int size = (int)userid.size();
    int n = size>10?10:size;

    vector <string> TheSameUsers;

    for(int i=0;i<n;i++)
    {
        string str = redis.getString(userid[i]);
        TheSameUsers.push_back(str);
    }
//    vector<string> res = redis.HashValsContent(TheSameUsers.at(0));

//    vector<int> vec;
//    std::transform(res.begin(),res.end(),std::back_inserter(vec),[](const std::string &str){return std::stoi(str);});//lambda expression


    return TheSameUsers;
}



//皮尔逊相似度计算
double Recommendation::Person(vector<string> &inst1, vector<string> &inst2)
{
    if(inst1.size() != inst2.size()){
        cout<< "the size of the vectors is not the same\n" << endl;
        return 0;
    }

    vector<int> vec1;
    std::transform(inst1.begin(),inst1.end(),std::back_inserter(vec1),[](const std::string &str){return std::stoi(str);});//lambda expression
    vector<int> vec2;
    std::transform(inst2.begin(),inst2.end(),std::back_inserter(vec2),[](const std::string &str){return std::stoi(str);});//lambda expression
    size_t n = vec1.size();
    double person = n*inner_product(vec1.begin(),vec1.end(),vec2.begin(),0.0)-accumulate(vec1.begin(),vec1.end(),0.0)*
            accumulate(vec2.begin(),vec2.end(),0.0);
    double temp1 = n*inner_product(vec1.begin(),vec1.end(),vec1.begin(),0.0)-pow(accumulate(vec1.begin(),vec1.end(),0.0),2.0);
    double temp2 = n*inner_product(vec2.begin(),vec2.end(),vec2.begin(),0.0)-pow(accumulate(vec2.begin(),vec2.end(),0.0),2.0);

    temp1 = sqrt(temp1);
    temp2 = sqrt(temp2);
    person = person/(temp1*temp2);
    return person;
}
