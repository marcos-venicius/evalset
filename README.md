# EVALSET

The JSON sucessor?

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/c05c3c0f-43e5-432f-b193-bd6c16ce049a" />

## The idea

This is a simple example of the `evalset` syntax: 

```console
root = {
    # this is the base url for the app
    base_url = "http://localhost:3030"
    app_name = "testing some stuff" # this is the name of my app
    id = 129
    other = 129.19

    route_names = {
        home = "Home"
        dashboard = "Dashboard"
    }
}

routes = [
    {
        id = mul($/root/id, 2)
        path = concat($/root/base_url, "/")
        name = $/root/route_names/home
    }
    {
        id = mul($/root/id, 3)
        path = concat($/root/base_url, "/dashboard")
        name = $/root/route_names/dashboard
    }
]

# Commas are entirely optional
dependencies = {
  test = "hello"
  outro = "now"
  number = 10
  floats = 3.1415,
  this_is_true = true,
  this_is_false = false
  this_is_nil = nil
  array = [
    "hello world"
    [
      10, 20
    ]
    {
      "this is a valid key" = "my very secret jwt token",
      my_lucy_number = 10
      child = {
        intern_node = "Hello Guys"
      }
    }
  ]
}

list = ["hello", 4239]
```

And, as you can see, evalset also suport "string named variables" with the following syntax:

```toml
"@angular/version" = "18.9.0"
```

The idea behind the project is to learn more about C, compilers, interpreters, etc.

The purpose of this language is to allow the user to create a configuration file like json but with some incremental features.

One of the features **Evaluation**, where the user can do math of string operations.
Another feature is **Variables**, you can define top level variables and reference them later.
In the end of the day, when the user try to get a value from a property, the property will be evaluated and returned the result to the user.

One of the thougts is to have a possibility to convert the `evalset` to json format, by evaluating all the fields and creating a valid json file. 

> [!NOTE]
> Probably it'll be lazy evaluated to avoid big files being slow to load, it should be optional to the user api.

> [!NOTE]
> Comma to separate elements inside arrays, objects and function arguments are entirely optional

## Data types

- String ("....")
- Integer (c int)
- Float (c doubles)
- Nullables (nil)
- Boolean (true | false)
- Array
- Object
- Paths ($/[a-zA-Z_])

## To implement

I think the API to this language in C should work in a very special way.

So, we'll have a function, maybe called `evalset_get` which returns a "generic type".

Basically this will return the primitive types already evaluated like: int, nil, array, object, etc.

So, this function will expect an instance of the evalset and a string as second parameter.

The second parameter should basically accept an evalset syntax, which you will be able to reference variables, get indexes call builtin functions and all of it.

But, it'll not have the possibility to create variables, only to do some work upon all of them.

If you look carefully to the code, the file itself is an object of many keys, so you will be able to iterate over this keys refering to it as `$/root`, then inside `$/root` you'll have
access to all other variables and do syntax like `$/root["permissions"][sub_i(len($/root["permissions"]), 1)]` and all kind of available syntax.

> [!WARNING]
> This is not the final thought about how it'll be implemented, it's just one of many ideas. But, to be honest, I like this one

## Internal functions

- [x] `integer sum_i(integer...)`, receive N `integers` as argument and return the sum of all of it as integer.
- [x] `float sum_f(float...)`, receive N `floats` as argument and return the sum of all of it as float.
- [x] `integer sum_ai([]integer...)`, receive an `array of integers` as argument and return the sum of all it's items as `integer`.
- [x] `float sum_af([]float...)`, receive an `array of floats` as argument and return the sum of all its items as `float`.
- [x] `[]any concat_a([]any...)`, receive N `arrays` as argument and return all arrays concatenated.
- [x] `string concat_s(string...)`, receive N `strings` as argument and return a single string concatenated.
- [x] `string join_as([]string, string?)`, receive an array of strings and a separator (which can be nil) then return a joined string. if separator is `nil`, the array will be joined without counting with the separator.
- [x] `[]string keys(object)`, receive an object as argument and return an array of strings with all its keys
- [x] `integer len(array)`, receive an array as argument and return the length of it
- [x] `integer len(string)`, receive a string as argument and return the length of it
- [x] `integer iota()`, returns an integer which auto-increment every time it's called

I still have some internal functions in my, but for now, I'll leave only these ones.
I'm thinking about the best way to implement this yet.

**Another important point, is to have a special function to allow the user to debug, some kind of print**
**But today we cannot parse a function without associating it to a variable**

## The name

The name _evalset_ is due to the language properties you "set vars and evaluate the" so, _evalset_ (believe it or not).
