base_path = "/base/path"

assets = [
    {
        id = iota()
        path = join_as([$/base_path, "logo.png"], "/")
        size = {
            width = 250
            height = 250
        }
    }
    {
        id = iota()
        path = join_as([$/base_path, "footer.png"], "/")
        size = {
            width = 250
            height = 250
        }
    }
    {
        id = iota()
        path = concat_s($/base_path, "/hero.png")
        size = {
            width = 250
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

