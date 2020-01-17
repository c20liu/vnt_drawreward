
## 年会抽奖小程序
在vntChain上通过智能合约实现。

## 程序原理
1. 外界调用传入一个随机值
2. 获取链上区块时间，与1中的随机值累加
3. 通过3次SHA，利用hash特性，随机打乱
4. 取前面的24bit作为种子
```
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
```
5. 借用c标准库中的rand函数，利用种子，计算随机数
6. 限制范围在1~staffNum之间
```
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

```
7. 记录已经抽中者
```
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
```

合约部署在主网公链上，每次的执行都有交易记录可查。

## 这只是抽奖流程的一个中间环节，抽奖的随机性还有入场时，号码的随机过程

## 大家有更好的随机算法，请帮忙实现，需要c语言（受限于链上c的能力，不调用特殊库，可以文件中实现库函数）

