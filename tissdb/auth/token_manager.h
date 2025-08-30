#ifndef TISSDB_TOKEN_MANAGER_H
#define TISSDB_TOKEN_MANAGER_H

#include <string>

namespace TissDB {
namespace Auth {

class TokenManager {
public:
    TokenManager();
    ~TokenManager();

    // Generates a new API token.
    std::string generate_token();

    // Validates an API token.
    bool validate_token(const std::string& token) const;
};

} // namespace Auth
} // namespace TissDB

#endif // TISSDB_TOKEN_MANAGER_H
