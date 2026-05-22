const config_module =require("./config")
const redis =require("ioredis")

//创建redis客户端实例
const redisCli = new redis({
    host:config_module.redis_host,
    port:config_module.redis_port,
    password:config_module.redis_passwd,
});

//监听事件
redisCli.on("error",function(error){
    console.log("RedisCli connect error");
    redisCli.quit();
});

async function GetRedis(key)
{
    try{
        const result =await redisCli.get(key);
        if(result == null){
            console.log('result:','<'+result+'>', 'This key cannot be find...');
            return null;
        }
        console.log('Result:','<'+result+'>','Get key success!...');
        return result
    }catch(error)
    {
        console.log('GetRedis error is', error);
        return null
    }
}
/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key) {
    try{
        const result = await redisCli.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
          console.log('result:<','<'+result+'>','This key is null...');
          return null
        }
        console.log('Result:','<'+result+'>','With this value!...');
        return result
    }catch(error){
        console.log('QueryRedis error is', error);
        return null
    }
  }
/**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function SetRedisExpire(key,value, exptime){
    try{
        // 设置键和值
        await redisCli.set(key,value)
        // 设置过期时间（以秒为单位）
        await redisCli.expire(key, exptime);
        return true;
    }catch(error){
        console.log('SetRedisExpire error is', error);
        return false;
    }
}
/**
 * 退出函数
 */
function Quit(){
    RedisCli.quit();
}
module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}