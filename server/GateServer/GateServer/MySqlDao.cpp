#include "MySqlDao.h"
#include "ConfigMgr.h"
MySqlDao::MySqlDao() {
	auto &cfg = ConfigMgr::Inst();
    std::string host = cfg["Mysql"]["Host"];
    std::string port = cfg["Mysql"]["Port"];
    std::string pwd = cfg["Mysql"]["Passwd"];
    std::string schema = cfg["Mysql"]["Schema"];
    std::string user = cfg["Mysql"]["User"];
    _pool.reset(new MysqlPool(5,host + ":" + port, user, pwd, schema));
}
MySqlDao::~MySqlDao() {
    _pool->Close();
}
int MySqlDao::UserReg(const std::string& name, const std::string& email, const std::string& pwd) {
    auto conn = _pool->Getconnection();
    try {
        if (conn == nullptr) {
            std::cout << "conn 没有取到链接"<<std::endl;
            return -1;
        }
        std::unique_ptr < sql::PreparedStatement > stmt(conn->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // 设置输入参数
        stmt->setString(1, name);
        stmt->setString(2, email);
        stmt->setString(3, pwd);
        // 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值
          // 执行存储过程
        stmt->execute();
        // 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
       // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
        std::unique_ptr<sql::Statement> stmtResult(conn->_con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
        if (res->next()) {
            int result = res->getInt("result");
            std::cout << "Result: " << result << std::endl;
            _pool->returnconnection(std::move(conn));
            return result;
        }
        _pool->returnconnection(std::move(conn));
        std:: cout<< "问题所在" << std::endl;
        return -1;
    }
    catch (sql::SQLException& e) {
        _pool->returnconnection(std::move(conn));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return -1;
    }
}

bool MySqlDao::Checkemail(const std::string& name, const std::string& email)
{
    auto conn = _pool->Getconnection();
    try{
        if (conn==nullptr) {
            std::cout << "取出的连接为空" << std::endl;
            _pool->returnconnection(std::move(conn));
            return false;
        }
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));
        pstmt->setString(1, name);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            std::cout << "Check Email: " << res->getString("email") << std::endl;
            if (email != res->getString("email")) {
                _pool->returnconnection(std::move(conn));
                return false;
            }
            _pool->returnconnection(std::move(conn));
            return true;
        }
    }
    catch (sql::SQLException &e) {
        _pool->returnconnection(std::move(conn));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}
bool MySqlDao::Undatedpwd(const std::string& name, const std::string& pwd) {
    auto conn = _pool->Getconnection();
    try {
        if (conn == nullptr) {
            std::cout << "取出的连接为空" << std::endl;
            _pool->returnconnection(std::move(conn));
            return false;
        }
        std::unique_ptr<sql::PreparedStatement> ptsmt(conn->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));
        ptsmt->setString(1, pwd);
        ptsmt->setString(2, name);
        int  res=ptsmt->executeUpdate();
        std::cout << "Updated rows: " << res << std::endl;
        _pool->returnconnection(std::move(conn));
        return true;
    }
    catch (sql::SQLException& e) {
        _pool->returnconnection(std::move(conn));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MySqlDao::Checkpwd(const std::string& name, const std::string& pwd,UserInfo& userInfo)
{
    auto conn = _pool->Getconnection();
    try {
        if (conn == nullptr) {
            std::cout << "取出的连接为空" << std::endl;
            _pool->returnconnection(std::move(conn));
            return false;
        }
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
        pstmt->setString(1, name);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::string origin_pwd = "";
        //res中有多个数据，遍历取出密码
        while(res->next()) {
            origin_pwd = res->getString("pwd");//获取email对应的密码
            std::cout << "origin_pwd = " << origin_pwd << std::endl;
            break;
        }
        if (origin_pwd != pwd) {
            //注意一定要归还连接
            _pool->returnconnection(std::move(conn));
            return false;
        }
        userInfo.name = name;
        userInfo.email = res->getString("email");
        userInfo.uid = res->getInt("uid");
        userInfo.pwd = origin_pwd;
        _pool->returnconnection(std::move(conn));
        return true;
    }
    catch (sql::SQLException& e) {
        //归还连接
        _pool->returnconnection(std::move(conn));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}
