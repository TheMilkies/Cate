#include "recursive.h"
#include "class.h"
#include "system_functions.h"
#include "error.h"
#include "target.h"
#include <assert.h>

static string_view form_path(struct CateFullPath* res,
                    string_view* path, string_view* name, uint8_t is_dir) {
    string_view result = {.length = path->length + name->length,
                          .text = res->x};
    static const int separator_length = __PP_STRLEN(path_sep_str)-1;
    
    if(result.length + separator_length*is_dir >= PATH_MAX) {
        cate_error("\""sv_fmt"/"sv_fmt"\"is too long for recursive?",
                svptr_p(path), svptr_p(name));
    }

    memcpy(res->x, path->text, path->length);
    memcpy(&res->x[path->length], name->text, name->length);
    if(is_dir) {
        memcpy(&res->x[result.length], path_sep_str, separator_length);
        result.length += separator_length;
    }

    return result;
}

static void save_entry(SavedStringIndexes* arr,
                        string_view* path, string_view* name) {
    struct CateFullPath new_path = {0};
    string_view result = form_path(&new_path, path, name, 0);

    STIndex to_save = st_save_sv(&ctx.st, &result);
    da_append((*arr), to_save);
}

static void save_entry_cstr(SavedStringIndexes* arr,
                        string_view* path, char* name) {
    string_view name_sv = sv_from_cstr(name);
    save_entry(arr, path, &name_sv);
}

int cate_recursive(RecursiveData* data, string_view* path) {
    assert(!(data->to_get & RECURSIVE_GET_FILES
        && data->to_get & RECURSIVE_GET_FILES_WITH_EXT)
        && "can't get all files and only files with extension");
    
    if(!path->length) {
        static string_view this_dir = sv_from_const("." path_sep_str);
        path = &this_dir;
    }

    struct CateSysDirectory* dir = 0;
    if(path->text[path->length] != '\0') {
        const char backup = path->text[path->length];
        path->text[path->length] = 0;
        dir = cate_sys_open_dir(path->text);
        path->text[path->length] = backup;
    } else {
        dir = cate_sys_open_dir(path->text);
    }
    if(!dir) return 0;
    
    struct CateSysDirEntry ent = {0};
    while (cate_sys_dir_get(dir, &ent)) {
        switch (ent.type) {
        case CATE_SYS_DIRENT_FILE: {
            if(data->to_get & RECURSIVE_GET_FILES) {
                save_entry_cstr(data->arr, path, ent.name);
            } else if(data->to_get & RECURSIVE_GET_FILES_WITH_EXT) {
                string_view name = sv_from_cstr(ent.name);
                if(sv_ends_with_sv(&name, &data->extension)) {
                    save_entry(data->arr, path, &name);
                }
            }
        }   break;

        case CATE_SYS_DIRENT_DIR: {
            if(data->to_get & RECURSIVE_GET_DIRS) {
                save_entry_cstr(data->arr, path, ent.name);
            } 

            if(data->subrecursion) {
                struct CateFullPath p = {0};
                string_view name = sv_from_cstr(ent.name);
                string_view new_path = form_path(&p, path, &name, 1);
                if(!cate_recursive(data, &new_path)) return 0;
            }
        }   break;
        
        default:
            break;
        }   
    }
    
    cate_sys_dir_close(dir);
    return 1;
}