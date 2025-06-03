permissions = {
    default = ["user:read" "user:write"]
    admin = ["user:read" "user:write" "repository:read" "repository:write"]
}

default_permissions = $/permissions/default
admin_permissions = $/permissions/admin

pages = [
    {
        route = "/user"
        permissions = [
            $/permissions/default
            $/permissions/admin
        ]
    }
    {
        route = "/user/repos"
        permissions = $/permissions/admin
    }
    {
        route = "/user/repos"
        permissions = $/permissions/admin
    }
    {
        route = "/user/repos"
        permissions = $/admin_permissions
    }
    {
        route = "/user"
        permissions = $/permissions/default
    }
    {
        route = "/user"
        permissions = $/permissions/default
    }
    {
        route = "/user"
        permissions = $/permissions/default
    }
]

