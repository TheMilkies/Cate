#include "catel.h"

CatelFile catel = {
    .def.x = "build.cate",
    .dir = "."
};

void catel_run(TokensArray* toks, TokenValuesArray* vals) {
    if(cs_file_exists("cate")) {
        memcpy(catel.dir.x, "cate", 5);
    }
    if(!cs_file_exists(".catel")) return;

    string_view file = {0};
    if(sv_load_file(&file, ".catel")) {
        cate_error("failed to open catel file!");
    }

    //cleanup
    toks->size = 0;
    vals->size - 0;
}