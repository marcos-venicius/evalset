# EVALSET

The JSON sucessor?

![immagine](https://github.com/user-attachments/assets/3e07d07a-cd22-4dfb-934e-30cf195d734b)

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
        id = $/root/id * 2
        path = $/root/base_url + "/"
        name = $/root/route_names/home
    }
    {
        id = $/root/id * 3
        path = $/root/base_url + "/dashboard"
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
> Comma inside arrays and objects are entirely optional

## Data types

- String ("....")
- Integer (c int)
- Float (c doubles)
- Nullables (nil)
- Boolean (true | false)
- Array
- Object

## The name

The name _evalset_ is due to the language properties you "set vars and evaluate the" so, _evalset_ (believe it or not).
