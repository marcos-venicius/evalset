id = iota() # Some kind of sequential id generator like in golang
#adding = sqrt(20)
#multiplying = mul(10, 50)
#subtracting = sub(100, 30)
#adding = add(20, 40, 10, 60)

#permissions = {
#    default = ["user:read" "user:write"]
#    admin = ["user:read" "user:write" "repository:read" "repository:write"]
#}
#
#default_permissions = $/permissions/default
#admin_permissions = $/permissions/admin
#
#pages = [
#    {
#        route = "/user"
#        permissions = [
#            $/permissions/default
#            $/permissions/admin
#        ]
#    }
#    {
#        route = "/user/repos"
#        permissions = $/permissions/admin
#    }
#    {
#        route = "/user/repos"
#        permissions = $/permissions/admin
#    }
#    {
#        route = "/user/repos"
#        permissions = $/admin_permissions
#    }
#    {
#        route = "/user"
#        permissions = $/permissions/default
#    }
#    {
#        route = "/user"
#        permissions = $/permissions/default
#    }
#    {
#        route = "/user"
#        permissions = $/permissions/default
#    }
#]
#
