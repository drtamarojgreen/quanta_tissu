#ifndef TISSDB_RBAC_H
#define TISSDB_RBAC_H

#include <string>
#include <vector>
#include <map>

namespace TissDB {
namespace Auth {

enum class Role {
    Admin,
    ReadWrite,
    ReadOnly,
    NoAccess
};

enum class Permission {
    DbRead,
    DbCreate,
    DbDelete,
    CollectionCreate,
    CollectionDelete,
    DocRead,
    DocWrite,
    DocDelete,
    AdminRead // For admin-only endpoints like log access
};

class RBACManager {
public:
    RBACManager();
    ~RBACManager();

    // Checks if a role has a specific permission.
    bool has_permission(Role role, Permission permission) const;

    // A utility to get a role from a string (e.g., from a token's metadata).
    static Role get_role_from_string(const std::string& role_str);
};

} // namespace Auth
} // namespace TissDB

#endif // TISSDB_RBAC_H
