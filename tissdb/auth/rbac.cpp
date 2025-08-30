#include "rbac.h"

namespace TissDB {
namespace Auth {

RBACManager::RBACManager() {
    // Constructor
}

RBACManager::~RBACManager() {
    // Destructor
}

bool RBACManager::has_permission(Role role, Permission permission) const {
    switch (role) {
        case Role::Admin:
            return true; // Admin has all permissions.
        case Role::ReadWrite:
            switch (permission) {
                case Permission::DocRead:
                case Permission::DocWrite:
                case Permission::DocDelete:
                case Permission::CollectionCreate:
                    return true;
                default:
                    return false;
            }
        case Role::ReadOnly:
            return permission == Permission::DocRead || permission == Permission::DbRead;
        case Role::NoAccess:
        default:
            return false;
    }
}

Role RBACManager::get_role_from_string(const std::string& role_str) {
    if (role_str == "admin") return Role::Admin;
    if (role_str == "read_write") return Role::ReadWrite;
    if (role_str == "read_only") return Role::ReadOnly;
    return Role::NoAccess;
}

} // namespace Auth
} // namespace TissDB
