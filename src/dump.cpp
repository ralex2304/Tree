#include "tree.h"

#ifdef TREE_DEBUG

#include "utils/html.h"
#include "utils/ptr_valid.h"
#include "log/log.h"
#include "log/graph_log.h"

extern LogFileData log_file;

#define LOG_(...) log_printf(&log_file, __VA_ARGS__)

/**
 * @brief Dump value printing function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_log_nil(Tree* tree, TreeNode** node, va_list* args,
                                           size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) args;
    (void) depth;
    (void) side;
    assert(node);

    LOG_("nil ");

    return TreeNodeActionRes::OK;
}

/**
 * @brief Dump value printing function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_log_val(Tree* tree, TreeNode** node, va_list* args, size_t depth,
                                           const TreeChildSide side) {
    (void) args;
    (void) depth;
    (void) side;
    assert(node);
    assert(*node);

    if (tree->node_elem_t_val_str != nullptr) {
        char* val = tree->node_elem_t_val_str((*node)->elem);

        if (val == nullptr)
            return TreeNodeActionRes::ERR;

        LOG_("%s ", val);

        FREE(val);
    } else
        LOG_("not_given(%p) ", *node);

    return TreeNodeActionRes::OK;
}

/**
 * @brief Dump value printing function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_log_open_node(Tree* tree, TreeNode** node, va_list* args,
                                                 size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) args;
    (void) depth;
    (void) side;
    assert(node);

    LOG_("( ");

    return TreeNodeActionRes::OK;
}

/**
 * @brief Dump value printing function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_log_close_node(Tree* tree, TreeNode** node, va_list* args,
                                                  size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) args;
    (void) depth;
    (void) side;
    assert(node);

    LOG_(") ");

    return TreeNodeActionRes::OK;
}

/**
 * @brief Dump value printing function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_log_close_node_val(Tree* tree, TreeNode** node, va_list* args,
                                                      size_t depth, const TreeChildSide side) {
    (void) depth;
    (void) side;
    assert(node);

    tree_dump_log_val(tree, node, args, depth, side);

    tree_dump_log_close_node(tree, node, args, depth, side);

    return TreeNodeActionRes::OK;
}

/**
 * @brief Dump value printing function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_log_open_node_val(Tree* tree, TreeNode** node, va_list* args,
                                                     size_t depth, const TreeChildSide side) {
    (void) depth;
    (void) side;
    assert(node);

    tree_dump_log_open_node(tree, node, args, depth, side);

    tree_dump_log_val(tree, node, args, depth, side);

    return TreeNodeActionRes::OK;
}

void tree_dump(Tree* tree, const VarCodeData call_data) {
    assert(tree);

    LOG_(HTML_BEGIN);

    LOG_("    tree_dump() called from %s:%d %s\n"
         "    %s[%p] initialised in %s:%d %s \n",
         call_data.file, call_data.line, call_data.func,
         tree->var_data.name, tree,
         tree->var_data.file, tree->var_data.line, tree->var_data.func);

    LOG_("    {\n");
    LOG_("    size = %zd\n", tree->size);
    LOG_("    root = %p\n", tree->root);
    LOG_("        {\n");

    if (!is_ptr_valid(tree->root)) {

        if (tree->size != 0 && tree_is_initialised(tree))
            LOG_(HTML_RED("        can't read (invalid pointer)\n"));

        LOG_("        }\n"
             "    }\n" HTML_END);
        return;
    }

    LOG_("        "" prefix:  ");

    TreeNodeAction* pre_actions[4] = {&tree_dump_log_open_node_val, nullptr,
                                      &tree_dump_log_close_node, &tree_dump_log_nil};

    tree_traversal(tree, &tree->root, pre_actions);

    LOG_("\n");

    LOG_("        "" infix:   ");

    TreeNodeAction* in_actions[4] = {&tree_dump_log_open_node, &tree_dump_log_val,
                                     &tree_dump_log_close_node, &tree_dump_log_nil};

    tree_traversal(tree, &tree->root, in_actions);

    LOG_("\n");

    LOG_("        "" postfix: ");

    TreeNodeAction* post_actions[4] = {&tree_dump_log_open_node, nullptr,
                                       &tree_dump_log_close_node_val, &tree_dump_log_nil};

    tree_traversal(tree, &tree->root, post_actions);

    LOG_("\n");


    LOG_("        }\n"
         "    }\n" HTML_END);

    char img_filename[log_file.MAX_FILENAME_LEN] = {};

    tree_dump_dot(tree, img_filename);

    LOG_("<img src=\"../../%s\">\n", img_filename);
}
#undef LOG_

#define FPRINTF_(...) if (fprintf(file, __VA_ARGS__) == 0) return TreeNodeActionRes::ERR

#define BACKGROUND_COLOR "\"#1f1f1f\""
#define FONT_COLOR       "\"#000000\""
#define NODE_PREFIX      "elem_"
#define NODE_PARAMS      "shape=\"plaintext\", style=\"filled\", fillcolor=\"#6e7681\""
#define ZERO_NODE_PARAMS "shape=\"plaintext\", style=\"filled\", fillcolor=\"#6e7681\", color=yellow"

/**
 * @brief Dumps node value for dot file
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_dump_dot_log_node_(Tree* tree, TreeNode** node, va_list* args,
                                                 size_t depth, const TreeChildSide side) {
    (void) depth;
    (void) side;
    assert(node);
    assert(args);

    va_list args_dup = {};
    va_copy(args_dup, *args);

    FILE* file = va_arg(args_dup, FILE*);

    if (tree->node_elem_t_val_str != nullptr) {
        char* val = tree->node_elem_t_val_str((*node)->elem);

        if (val == nullptr)
            return TreeNodeActionRes::ERR;

        FPRINTF_(NODE_PREFIX "%p [label=\"%s\"]\n", *node, val);

        FREE(val);
    } else
        FPRINTF_(NODE_PREFIX "%p [label=\"not_given\n%p\"]\n", *node, *node);

    if ((*node)->left != nullptr) {
        FPRINTF_(NODE_PREFIX "%p->" NODE_PREFIX "%p[color=green]\n", *node, (*node)->left);
    }

    if ((*node)->right != nullptr) {
        FPRINTF_(NODE_PREFIX "%p->" NODE_PREFIX "%p[color=green]\n", *node, (*node)->right);
    }

    va_end(args_dup);

    return TreeNodeActionRes::OK;
}
#undef FPRINTF_

#define FPRINTF_(...) if (fprintf(file, __VA_ARGS__) == 0) return false

bool tree_dump_dot(Tree* tree, char* img_filename) {
    assert(tree);
    assert(img_filename);

    static size_t dot_number = 0;

    char dot_filename[log_file.MAX_FILENAME_LEN] = {};

    size_t str_len = strncat_len(dot_filename, log_file.timestamp_dir, log_file.MAX_FILENAME_LEN);
    snprintf(dot_filename + str_len, log_file.MAX_FILENAME_LEN - str_len,
             "%zd", dot_number);
    str_len = strncat_len(dot_filename, ".dot", log_file.MAX_FILENAME_LEN);

    FILE* file = fopen(dot_filename, "wb");
    if (file == nullptr)
        return false;

    FPRINTF_("digraph List{\n"
             "    graph [bgcolor=" BACKGROUND_COLOR "];\n"
             "    node[color=white, fontcolor=" FONT_COLOR ", fontsize=14, "
             "fontname=\"verdana\", style=\"filled\", fillcolor=\"#6e7681\"];\n\n");

    tree_preorder(tree, &tree->root, &tree_dump_dot_log_node_, file);

    FPRINTF_("}\n");

    if (fclose(file) != 0) {
        perror("Error closing file");
        return false;
    }

    str_len = strncat_len(img_filename, log_file.timestamp_dir, log_file.MAX_FILENAME_LEN);
    snprintf(img_filename + str_len, log_file.MAX_FILENAME_LEN - str_len, "%zd", dot_number++);
    str_len = strncat_len(img_filename, ".svg", log_file.MAX_FILENAME_LEN);

    if (!create_img(dot_filename, img_filename)) {
        fprintf(stderr, "Error creating dot graph\n");
        return false;
    }

    return true;
}
#undef FPRINTF_

#define PRINT_ERR_(code, descr)  if ((err_code) & Tree::code)                                       \
                                    log_printf(&log_file,                                           \
                                               HTML_TEXT(HTML_RED("!!! " #code ": " descr "\n")));
void tree_print_error(const int err_code) {
    if (err_code == Tree::OK) {
        log_printf(&log_file, HTML_TEXT(HTML_GREEN("No error\n")));
    } else {
        PRINT_ERR_(ALLOC_ERR,           "Can't allocate memory");
        PRINT_ERR_(ALREADY_INITIALISED, "Constructor called for already initialised or corrupted tree");
        PRINT_ERR_(UNITIALISED,         "Tree is not initialised");
        PRINT_ERR_(INVALID_NODE_PTR,    "TreeNode pointer is not valid for writing");
        PRINT_ERR_(POISON_VAL_FOUND,    "There is poison value in TreeNode");
        PRINT_ERR_(INVALID_SIZE,        "Actual tree size doesn't equal tree.size");
        PRINT_ERR_(NEGATIVE_SIZE,       "Negative tree.size");
        PRINT_ERR_(DAMAGED_PATH,        "Tree is damaged. Invalid path");
        PRINT_ERR_(ACTION_ERROR,        "Some action on TreeNode failed");
        PRINT_ERR_(NODE_ALREADY_EXISTS, "Insertion is not allowed. Node already exists here");
        PRINT_ERR_(CANNOT_DELETE,       "Cannot delete element. It has child/children");
        PRINT_ERR_(NODE_VERIFY_FAIL,    "Node elem verification failed");
        PRINT_ERR_(INVALID_ELEM_T_SIZE, "tree.elem_size is <= 0");
    }
}
#undef PRINT_ERR_

#endif //< #ifdef TREE_DEBUG
