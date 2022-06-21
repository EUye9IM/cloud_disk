/***
 * 基于 jwt-cpp 实现的可以简单使用的 token 模块
 * token 无状态，用于创建和验证
 **/

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "traits.h"
#include <exception>

namespace Anakin {
class token_exception : public std::exception
{
public:
    const char* what () const throw ()
    {
        return "token exception";
    }
};

class Token
{
public:
    Token() = default;
    static const std::string create(const int expire_time=15);
    /* 认证函数，如果出错抛出异常 */
    static void verify(const std::string token);

private:
    using min = std::chrono::minutes;
    using traits = jwt::traits::nlohmann_json;
	using claim = jwt::basic_claim<traits>;

};

inline const std::string Token::create(const int expire_time)
{
    const auto time = jwt::date::clock::now();
	const auto token = jwt::create<traits>()
						   .set_type("JWT")
						   .set_issuer("cloud-disk")
						   .set_audience("cloud-disk.com")
						   .set_issued_at(time)
						   .set_not_before(time)
						   .set_expires_at(time + min{expire_time})
						   .sign(jwt::algorithm::hs256{"secret"});

    return token;
}

inline void Token::verify(const std::string token) 
{
        const auto decoded = jwt::decode<traits>(token);

        jwt::verify<traits>()
            .allow_algorithm(jwt::algorithm::hs256{"secret"})
            .with_issuer("cloud-disk")
            .with_audience("cloud-disk.com")
            .verify(decoded);
}

}// end namespace Anakin

#endif