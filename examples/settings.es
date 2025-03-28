root = {
    # this is the base url for the app
    base_url = "http://localhost:3030"
    app_name = "\\testing some\tstuff\n" # this is the name of my app
    id = 129
    other = (129.19 + 2) / -4 - 2 % 3
    negative = -10
    negative_float = -10.230234
    boolean_true = true
    boolean_false = false
    nullable = nil

    route_names = {
        home = "Home"
        dashboard = "Dashboard"
    }

    titles = {
        home = "\"Home\""
        dashboard = "\"Dashboard\""
    }
}

routes = [
    {
        id = $/root/id * 2
        path = $/root/base_url + "/"
        name = $/root/route_names/home
        title = $/root/titles/home
    }
    {
        id = $/root/id * 3
        path = $/root/base_url + "/dashboard"
        name = $/root/route_names/dashboard
        title = $/root/titles/dashboard
    }
]
