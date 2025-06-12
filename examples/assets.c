#include "./evalset.h"

int main(void) {
    Evalset evalset = evalset_init("./path/to/settings.es");

    evalset_code_t err = evalset_compile(&evalset);

    if (err != 0) {
        evalset_free(&evalset);

        return 1;
    }

    Evalset_Item map_matrix, row, bit_item, assets, object;

    err = evalset_get(evalset.root, &map_matrix, "map_matrix", NULL);

    if (err != EVALSET_OK_CODE) {
        evalset_free(&evalset);

        // TODO: handle it?
        return 1;
    }

    for (int i = 0; i < map_matrix.as.array.size; ++i) {
        evalset_get(map_matrix, &row, "%d", i); // handle error

        for (int j = 0; j < row.as.array.size; ++j) {
            evalset_get(row, &bit_item, "%d", j); // handle error
            int bit = evalset_unwrap_integer(bit_item);

            if (j > 0) printf(" ");
            printf("%d", bit);
        }
        printf("\n");
    }

    evalset_get(evalset.root, &assets, "%s", "assets"); // handle error
    
    for (int i = 0; i < assets.as.array.size; ++i) {
        evalset_get(assets, &object, "%d", i); // handle error

        Evalset_Item id, path, size, width, height;

        // handle errors
        evalset_get(object, &id, "%s", "id");
        evalset_get(object, &path, "%s", "path");
        evalset_get(object, &size, "%s", "size");
        evalset_get(size, &width, "%s", "width");
        evalset_get(size, &height, "%s", "height");

        printf(
            "ID: %d\nPATH: %.*s\nSIZE: (%dx%d)\n\n",
            evalset_unwrap_integer(id),
            (int)evalset_unwrap_string(path).size,
            evalset_unwrap_string(path).value,
            evalset_unwrap_integer(width),
            evalset_unwrap_integer(height)
        );
    }

    evalset_free(&evalset);

    return 0;
}
