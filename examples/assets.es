base_path = "/base/path"

is_true = true
assets = [
    {
        id = iota()
        path = join([$/base_path, "logo.png"], "/")
        active = true
        size = {
            width = 250
            height = 250
        }
    }
    {
        id = iota()
        path = join([$/base_path, "footer.png"], "/")
        active = false
        size = {
            width = 250
            height = 250
        }
    }
    {
        id = iota()
        path = join($/base_path, "/hero.png")
        size = {
            width = sum(200, 50)
            height = 250
        }
    }
]

map_matrix = [
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
    [0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1]
    [1 0 1 1 0 1 0 1 1 0 1 0 1 1 0 1 0 1 1 0]
]

