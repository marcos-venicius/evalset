id = iota() # Some kind of sequential id generator like in golang
anything = hash("name", 10, 10.5, nil, true, false)
result = concat_arrays(
    [1, 2, 3]
    [4 ,5, 6]
    [
        [
            {
                test = "Hello World"
            }
        ]
    ]
)
keys = object_keys({
    foo = 10
    bar = 30
    baz = "Hello Guys"
    array = [
        {
            "@version" = 20
        }
    ]
})
adding = sqrt(20)
multiplying = mul(10, 50)
subtracting = sub(100, 30)
adding = add(20, 40, 10, 60)
sum_of_all = sum($/adding, $/multiplying, $/subtracting)

# 2 + (3 * 2)
expr = sum(2, mul(3, 2, sum_array(concat([20, 30], [4, 8.7]))))

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

