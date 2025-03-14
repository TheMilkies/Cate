#include "cate.h"
#include "testlib.h"

void temp_file(char* path) {
    FILE* f = fopen(path, "w");
    if(!f) {
        fprintf(stderr, "failed to create %s\n", path);
        exit(-1);
    }
    fclose(f);
}

test_def(system) {
    assert_eq(cs_create_directory("_test_idk_idk/a/b/c/d"), 1);
    assert_eq(cs_create_directory("_test_idk_idk/a/b/e"), 1);
    assert_eq(cs_file_exists("_test_idk_idk/a/b/e"), 1);
    assert_eq(cs_file_exists("_test_idk_idk/a/b/c/d"), 1);

    temp_file("_test_idk_idk/a/b/a.txt");
    assert_eq(cs_move("_test_idk_idk/a/b/a.txt", "_test_idk_idk/a/b/b.txt"), 1);
    assert_eq(cs_file_exists("_test_idk_idk/a/b/b.txt"), 1);
    assert_eq(cs_remove("_test_idk_idk/a/b/b.txt"), 1);
    assert_eq(cs_file_exists("_test_idk_idk/a/b/a.txt"), 0);
    assert_eq(cs_file_exists("_test_idk_idk/a/b"), 1);

    assert_eq(cs_remove("_test_idk_idk"), 1);
    assert_eq(cs_file_exists("_test_idk_idk"), 0);
}

test_main({
    test_register(system);
})