#include "log/log.h"
#include "tree.h"

LogFileData log_file = {"log"};

const int INT_POISON_VAL = __INT_MAX__ - 13;

inline bool int_dtor(void* elem) {
    assert(elem);

    *((int*)elem) = INT_POISON_VAL;
    return true;
}

inline bool int_verify(void* elem) {
    if (elem == nullptr)
        return false;

    if (*((int*)elem) == INT_POISON_VAL)
        return false;
    return true;
}

inline char* int_str(const void* elem) {
    static const size_t MAX_INT_STR_LEN = 15;

    char* str = (char*)calloc(MAX_INT_STR_LEN, sizeof(char));

    if (str == nullptr) return nullptr;

    snprintf(str, MAX_INT_STR_LEN - 1, "%d", *((const int*)elem));

    return str;
}



int main() {
    log_open_file(&log_file, "wb");

    Tree tree = {};
    TREE_CTOR(&tree, sizeof(int), &int_dtor, &int_verify, &int_str);

    int push_val = 10;
    tree_insert_root(&tree, &push_val);

    TREE_DUMP(&tree);

    push_val = 30;
    tree_insert(&tree, &tree.root->right, tree.root, &push_val);

    TREE_DUMP(&tree);

    push_val = 20;
    tree_insert(&tree, &tree.root->left, tree.root, &push_val);

    TREE_DUMP(&tree);

    tree_delete(&tree, &tree.root->right, false);

    TREE_DUMP(&tree);

    tree_dtor(&tree);

    log_close_file(&log_file);
}
