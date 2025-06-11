id = iota() # Some kind of sequential id generator like in golang
adding = sqrt(20)
multiplying = mul(10, 50)
subtracting = sub(100, 30)
adding = add(20, 40, 10, 60)

permissions = {
    default = ["user:read" "user:write"]
    admin = ["user:read" "user:write" "repository:read" "repository:write"]
}

default_permissions = $/permissions/default
admin_permissions = $/permissions/admin

empty_array = []
empty_object = {}

matrix = [
    [
        0, true, false, "hello world", nil, 10.3, [], [10]
        0 true false "hello world" nil 10.3 [] [10]
        {
            hello = 10
            hey = "hello guys"
        }
        [
            {
                float = 20.0
                string = "hello"
                integer = 10
                booleanTrue = true
                booleanFalse = false
                nill = nil
                obj = {
                    "What?" = "Hey"
                }
                path = $/helo
            }
            {}
        ]
    ]
]

pages = [
    {
        route = "/user"
        permissions = [
            $/permissions/default
            $/permissions/admin
        ]
        test = $/permissions/admin
    }
]

