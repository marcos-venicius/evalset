dependencies = {
  "@angular/react/native" = "^10.0.0"
}

root = [
  {
    # this is the base url for the app
    base_url = "http://localhost:3030"
    app_name = "testing some stuff" # this is the name of my app
    id = 129
    other = 129.19

    route_names = {
      home = "Home"
      dashboard = "Dashboard"
      sub_routes = [
        {
          name = "user"
          permissions = ["one" "two" "three"]
        }
      ]
    }
  }
]
