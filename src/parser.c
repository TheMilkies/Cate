#include <limits.h>
#include "parser.h"

typedef struct {
    TokensArray toks;
    TokenValuesArray vals;
    CateContext* ctx;
    size_t i_tok, i_val;
} Parser;

static int was_opened(CateContext* ctx, char* path) {
    for (size_t i = 0; i < ctx->opened_files.size; ++i) {
        if(strncmp(ctx->opened_files.data[i].x, path, FILENAME_MAX))
            return 1;
    }
    return 0;
}

static void cate_parse(Parser* p);
void cate_run(CateContext* ctx, string_view file_name) {   
    //construct the complete path:
    //"$catedir/$file_name.cate"
    CateSysPath path = {0};
    char* to_open = path.x;
    cs_path_append(&path, catel.dir.x);
    cs_path_directory_separator(&path);
    size_t before_filename = path.length; //if it doesn't exist in catedir
    cs_path_append(&path, file_name.text);
    if(!sv_ends_with(&file_name, ".cate", 5)) {
        cs_path_append(&path, ".cate");
    }

    //it might just be "$file_name.cate"
    if(!cs_file_exists(path.x)) {
        //skip the directory
        to_open = &path.x[before_filename];
    }

    //check if it was already parsed:
    {
        CateSysPath resolved;
        char* good = realpath(to_open, resolved.x);
        if(!good) {
            cate_error("can't open file \"%s\"", to_open);
        }
        //since cate 3.0, we don't error when a file was already opened
        if(was_opened(ctx, resolved.x)) return;
        da_append(ctx->opened_files, resolved);
    }

    //alright, let's parse it!
    Parser p = {.ctx = ctx};
    string_view file = {0};
    int err = sv_load_file(&file, to_open);
    if(err) {
        cate_error("can't open file \"%s\" because: ", to_open, strerror(err));
    }

    cate_tokenize(&file, &p.toks, &p.vals);
    cate_parse(&p);
}

static void next(Parser* p) {
    p->i_tok += 1;
}

static void cate_parse(Parser* p) {

}