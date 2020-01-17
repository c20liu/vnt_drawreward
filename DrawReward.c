#include "vntlib.h"

KEY address owner; // 管理员
KEY uint32 staffNum = 105; //员工总数
KEY uint32 currentDraw = 106; //当前抽取数
KEY uint32 currentSeed = 0; //当前轮次的随机种子
KEY bool start = false;

KEY mapping(uint32, bool) record; //记录是否被抽中过

// 构造函数
constructor DrawReward()
{
    owner = GetSender();
    start = true;
    currentSeed = GetTimestamp();
}

MUTABLE
void SetStopGame()
{
    Require(Equal(GetSender(), owner), "You are not the owner");
    start = false;
}

//设置抽奖员工总数，会重置抽奖，抽奖开始后不再执行
MUTABLE
void SetTotalStaffNum(uint32 num)
{
    Require(Equal(GetSender(), owner), "You are not the owner");
    staffNum = num;
    uint32 i;
    for (i = 0; i < num; i++) {
        record.key = i;
        record.value = false;
    }
}

int getIndexOfSigns(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if(ch >= 'A' && ch <='F') 
    {
        return ch - 'A' + 10;
    }
    if(ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    return -1;
}

long hexToDec(char *source)
{
    long sum = 0;
    long t = 1;
    int i, len;

    len = strlen(source);
    PrintInt64T("len is ", len);
    for(i=len-1; i>=0; i--)
    {
        sum += t * getIndexOfSigns(*(source + i));    //从低位开始，getIndexOfSigns函数返回其中一位字符对应的十进制数
        t *= 16;
    }  
    return sum;
} 

// 用链上时间加外部传入随机值，求三次哈希，增加随机性
// 取哈希值的前24位作为种子
void _getSeed(uint32 exterSeed)
{
  uint64 time = GetTimestamp();
  PrintUint64T("get time ", time);
  uint64 newTime = time + exterSeed;
  PrintUint64T("new time ", newTime);
  string time_sha3 = SHA3(SHA3(SHA3(FromU64(newTime))));
  PrintStr("get time sha3 ", time_sha3);
  char tmp[8] = {0};
  memcpy(tmp, &time_sha3[2], 7);
  tmp[8] = '\0';
  long seed = hexToDec(tmp);
  PrintUint64T("a ", seed);
  currentSeed = seed;
}

// 实现了c标准库中的rand()函数
uint32 _getRandom()
{
    PrintUint32T("current seed ", currentSeed);
    int random = ((currentSeed * 214013L + 2531011L) >> 16) & 0x7fff;
    PrintInt32T("the random is ", random);
    int random_v = random % staffNum + 1;
    PrintInt32T("the random_v is ", random_v);
    return random_v;
}

UNMUTABLE
uint64 testRandom(uint32 exterSeed) { _getSeed(exterSeed); return _getRandom(); }

//抽奖
MUTABLE
uint32 Draw(uint32 exterSeed)
{
    Require(start == true, "The game is not started"); 
    Require(Equal(GetSender(), owner), "You are not the owner");
    _getSeed(exterSeed);
    uint32 id = (uint32)(_getRandom());
    uint32 tmpSeed = exterSeed;
    record.key = id;
    bool isDrawed = record.value;
    while (isDrawed == true) {
        if(tmpSeed > id) {
            tmpSeed = tmpSeed - id;
        } else {
            tmpSeed = id - tmpSeed;
        }
        PrintUint32T("use exterSeed ", tmpSeed);
        _getSeed(tmpSeed);
        id = (uint32)(_getRandom());
        record.key = id;
        isDrawed = record.value;
        
    }
    record.key = id;
    record.value = true;
    currentDraw = id;
    PrintUint32T("current draw ", currentDraw);
    return id;
}
// ##########################
// 增加员工人数
MUTABLE
void AddStaffNum(uint32 num)
{
    Require(Equal(GetSender(), owner), "You are not the owner");
    uint32 i;
    uint32 currentStaffNum = staffNum + num;
    for (i = staffNum; i < currentStaffNum; i++) {
        record.key = i;
        record.value = false;
    }
    staffNum = currentStaffNum;
}

// 减少员工人数
MUTABLE
void SubStaffNum(uint32 num)
{
    Require(Equal(GetSender(), owner), "You are not the owner");
    Require(num <= staffNum, "sub num is bigger than staff num");
    uint32 i;
    uint32 currentStaffNum = staffNum - num;
    for (i = currentStaffNum; i < staffNum; i++) {
        record.key = i;
        record.value = true;
    }
    staffNum = currentStaffNum;
}

// 重置整个游戏
MUTABLE
void ResetGame()
{
    Require(Equal(GetSender(), owner), "You are not the owner");
    uint32 i;
    for (i = 0; i < staffNum; i++) {
        record.key = i;
        record.value = false;
    }
}

// 重置某个人参与或者不参与游戏
// false: 参与游戏
// true: 不参与游戏
MUTABLE
void ResetSomeOne(uint32 id, bool v)
{
    Require(Equal(GetSender(), owner), "You are not the owner");
    Require(id <= staffNum, "you are not one staff");
    record.key = id;
    record.value = v;
    if (currentDraw == id)
    {
        currentDraw = 106;
    }
}

// ##########################

// ########################## Get Function ######################
// 获取员工数
UNMUTABLE
uint32 GetStaffNum()
{
    return staffNum;
}

// 获取某编号是否已经被抽中
UNMUTABLE
bool IsDrawed(uint32 id)
{
    Require(id <= staffNum, "you are not one staff");
    record.key = id;
    return record.value;
}

UNMUTABLE
uint32 GetCurrentDraw()
{
    return currentDraw;
}
