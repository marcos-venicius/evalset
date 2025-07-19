id = iota() # Some kind of sequential id generator like in golang
anything = concat_a(["name", 10, 10.5, nil, true, false])
result = concat_a(
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
keys = keys({
  foo = 10
  bar = 30
  baz = "Hello Guys"
  array = [
    {
      "@version" = 20
    }
  ]
})
# adding = sqrt(20)
# multiplying = mul(10, 50)
# subtracting = sub(100, 30)
# adding = add(20, 40, 10, 60)
# sum_of_all = sum($/adding, $/multiplying, $/subtracting)

# 2 + (3 * 2)
expr = sum_f(2, sum_f(3, 2, sum_af(concat_a([20, 30], [4, 8.7]))))

permissions = {
  default = ["user:read" "user:write"]
  admin = ["user:read" "user:write" "repository:read" "repository:write"]
  test = iota()
  testing = iota()
}

default_permissions = $/permissions["default"]
admin_permissions = $/permissions["admin"]

empty_array = [sum_i(10, 10)]
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
      }
      {}
    ]
  ]
]

pages = [
  {
    route = "/user"
    permissions = [
      $/permissions["default"]
      $/permissions["admin"]
    ]
    test = $/permissions["admin"]
  }
]

