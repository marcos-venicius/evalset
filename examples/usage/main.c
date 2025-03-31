#include <evalset.h>

int main(void) {
    // port
    // interface
    // assets paths
    // external libs configuration info

    EvalConfigs *configs = evalset_load_file("./configs.es");

    EvalString from = evalset_get_string(configs, "email.from");
    EvalObject *logo = evalset_get_object(configs, "assets[1]");

    EvalString name = evalset_get_string_from_object(&logo, "name");
    EvalString path = evalset_get_string_from_object(&logo, "path");

    EvalString logo_name = evalset_get_string(configs, "assets[1].name");
    char *logo_path = from_eval_string_to_cstring(evalset_get_string(configs, "assets[1].path"));

    return 0;
}
