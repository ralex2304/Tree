#ifndef TREE_H_
#define TREE_H_

#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include "utils/macros.h"

#define TREE_DEBUG //< enables TREE_DEBUG mode

/**
 * @brief Indicates side of child node
 */
enum class TreeChildSide {
    LEFT  = -1,
    ROOT  =  0,
    RIGHT =  1,
};

/**
 * @brief Traversal action function result
 */
enum class TreeNodeActionRes {
    EXIT_FULL = -2,
    EXIT_NODE = -1,
    ERR       =  0,
    OK        =  1,
};

/**
 * @brief Function template for elem_t destructor and verificator
 *
 * @param elem
 * @return bool
 */
typedef bool Elem_t_func(void* elem);

/**
 * @brief Function template for elem_t value string
 *
 * @param elem
 * @return char*
 */
typedef char* Elem_t_val_str(const void* elem);

/**
 * @brief Specifies tree node data
 */
struct TreeNode {
    void* elem = nullptr;   //< value

    TreeNode* parent = nullptr; //< node parent pointer

    TreeNode* left  = nullptr;  //< left child
    TreeNode* right = nullptr;  //< right child
};

/**
 * @brief Specifies tree data
 */
struct Tree {
    static const ssize_t UNITIALISED_VAL = -1;  //< Default poison value

    // Error codes
    enum Results {
        OK                  = 0x000000,
        ALLOC_ERR           = 0x000002,

        ALREADY_INITIALISED = 0x000004,
        UNITIALISED         = 0x000008,
        INVALID_NODE_PTR    = 0x000010,
        POISON_VAL_FOUND    = 0x000020,
        INVALID_SIZE        = 0x000800,
        NEGATIVE_SIZE       = 0x001000,
        DAMAGED_PATH        = 0x040000,
        ACTION_ERROR        = 0x080000,
        NODE_ALREADY_EXISTS = 0x100000,
        CANNOT_DELETE       = 0x200000,
        INVALID_ELEM_T_SIZE = 0x400000,
        NODE_VERIFY_FAIL    = 0x800000,
    };

    ssize_t size      = UNITIALISED_VAL;    //< number of nodes in tree
    ssize_t elem_size = UNITIALISED_VAL;    //< one element size in bytes
    TreeNode* root = nullptr;               //< tree root pointer

    Elem_t_func* node_elem_t_dtor = nullptr; //< Elem_t dtor. nullptr if not needed

#ifdef TREE_DEBUG
    VarCodeData var_data = {};

    Elem_t_func* node_elem_t_verify  = nullptr;     //< Elem_t verificator. nullptr if not needed
    Elem_t_val_str* node_elem_t_val_str = nullptr;  //< Must return Elem_t value in string. nullptr if not needed
#endif //< #ifdef TREE_DEBUG
};

/**
 * @brief Traversal action function template. Can receive arguments through va_list
 *
 * @param tree
 * @param node
 * @param args
 * @param depth distance from root
 * @param side child side
 */
typedef TreeNodeActionRes (TreeNodeAction)(Tree* tree, TreeNode** node, va_list* args,
                                           size_t depth, const TreeChildSide side);

/**
 * @brief (Use macros TREE_CTOR) Tree constructor
 *
 * @param tree
 * @param elem_size sizeof(Elem_t)
 * @param elem_t_dtor Elem_t destructor
 * @return int
 */
int tree_ctor(Tree* tree, const size_t elem_size, Elem_t_func* elem_t_dtor);

/**
 * @brief Tree destructor
 *
 * @param tree
 * @return int
 */
int tree_dtor(Tree* tree);

/**
 * @brief Get element value from TreeNode
 *
 * @param tree
 * @param node
 * @param dest
 * @return int
 */
int tree_get_elem(Tree* tree, const TreeNode* node, void** dest);

/**
 * @brief Set element value in TreeNode
 *
 * @param tree
 * @param node
 * @param src
 * @param dtor do dtor on old value?
 * @return int
 */
int tree_set_elem(Tree* tree, TreeNode* node, void* src, bool dtor = true);

/**
 * @brief Insert node in tree
 *
 * @param tree
 * @param node destination pointer
 * @param parent
 * @param elem
 * @return int
 */
int tree_insert(Tree* tree, TreeNode** node, TreeNode* parent, void* elem);

/**
 * @brief Delete node from tree
 *
 * @param tree
 * @param node
 * @param recursive if true - deletes all children too
 * @return int
 */
int tree_delete(Tree* tree, TreeNode** node, bool recursive = false);

/**
 * @brief Insert to root
 *
 * @param tree
 * @param elem
 * @return int
 */
inline int tree_insert_root(Tree* tree, void* elem) {
    return tree_insert(tree, &tree->root, nullptr, elem);
}

/**
 * @brief Tree traversal function. Recieves TreeNodeAction functions and va_args for them
 *
 * @param tree
 * @param root start point
 * @param actions {pre_action, in_action, post_action, null_action}
 * @param ...
 * @return int
 */
int tree_traversal(Tree* tree, TreeNode** root, TreeNodeAction* actions[4], ...);

/**
 * @brief Tree preorder traversal function. Recieves TreeNodeAction function and va_args for it
 *
 * @param tree
 * @param root start point
 * @param action prefix action
 * @param ...
 * @return int
 */
int tree_preorder(Tree* tree, TreeNode** root, TreeNodeAction* action, ...);

/**
 * @brief Tree inorder traversal function. Recieves TreeNodeAction function and va_args for it
 *
 * @param tree
 * @param root start point
 * @param action infix action
 * @param ...
 * @return int
 */
int tree_inorder(Tree* tree, TreeNode** root, TreeNodeAction* action, ...);

/**
 * @brief Tree postorder traversal function. Recieves TreeNodeAction function and va_args for it
 *
 * @param tree
 * @param root start point
 * @param action postfix action
 * @param ...
 * @return int
 */
int tree_postorder(Tree* tree, TreeNode** root, TreeNodeAction* action, ...);

/**
 * @brief Checks if TreeNode was initialised
 *
 * @param node
 * @return true
 * @return false
 */
inline bool tree_node_is_initialised(const TreeNode* node) {
    return !(node->elem == nullptr && node->left == nullptr && node->right == nullptr);
}

/**
 * @brief Checks if Tree was initialised
 *
 * @param tree
 * @return true
 * @return false
 */
inline bool tree_is_initialised(const Tree* tree) {
    return !(tree->size == tree->UNITIALISED_VAL && tree->root == nullptr &&
             tree->node_elem_t_dtor == nullptr);
}

#ifdef TREE_DEBUG
    /**
     * @brief Prints text error to log file
     *
     * @param err_code
     */
    void tree_print_error(const int err_code);

    /**
     * @brief (Use TREE_VERIFY macros) Verifies Tree data
     *
     * @param tree
     * @param do_traversal if true, will do traversal and verify each node
     * @return int
     */
    int tree_verify(Tree* tree, bool do_traversal = true);

    /**
     * @brief (Use TREE_NODE_VERIFY macros) Verifies TreeNode
     *
     * @param tree
     * @param node
     * @param depth
     * @return int
     */
    int tree_node_verify(const Tree* tree, const TreeNode* node, const size_t depth);

    /**
     * @brief (Use TREE_CTOR macros) TREE_DEBUG mode constructor
     *
     * @param tree
     * @param elem_size
     * @param elem_t_dtor
     * @param elem_t_verify
     * @param elem_t_val_str
     * @param var_data
     * @return int
     */
    int tree_ctor_debug(Tree* tree, const size_t elem_size,
                        Elem_t_func* elem_t_dtor, Elem_t_func* elem_t_verify,
                        Elem_t_val_str* elem_t_val_str, const VarCodeData var_data);

    /**
     * @brief (Use TREE_DUMP macros) Dumps tree to log file
     *
     * @param tree
     * @param var_data
     */
    void tree_dump(Tree* tree, const VarCodeData var_data);

    /**
     * @brief Dumps tree to dot img file
     *
     * @param tree
     * @param img_filename
     * @return true
     * @return false
     */
    bool tree_dump_dot(Tree* tree, char* img_filename);

    /**
     * @brief Tree constructor
     *
     * @param tree
     * @param elem_size sizeof(Elem_t)
     * @param dtor Elem_t destructor function
     * @param verify Elem_t verificator function
     * @param val_str Elem_t val_str functions
     *
     * @return int
     */
    #define TREE_CTOR(tree, elem_size, dtor, verify, val_str) tree_ctor_debug(tree, elem_size, dtor,    \
                                                                              verify, val_str,          \
                                                                              VAR_CODE_DATA_PTR(tree))
    /**
     * @brief Tree dump
     *
     * @param tree
     */
    #define TREE_DUMP(tree) tree_dump(tree, VAR_CODE_DATA(tree))

    /**
     * @brief Checks res and writes log if res is not OK
     *
     * @param tree
     * @param res
     */
    #define TREE_OK(tree, res)  do {                            \
                                    if (res != tree->OK) {      \
                                        tree_print_error(res);  \
                                        TREE_DUMP(tree);        \
                                    }                           \
                                } while (0)

    /**
     * @brief Tree verificator
     *
     * @param tree
     *
     * @return int
     */
    #define TREE_VERIFY(tree)   tree_verify(tree)

    /**
     * @brief No traversal Tree verificator
     *
     * @param tree
     *
     * @return int
     */
    #define TREE_VERIFY_NO_TRAVERSAL(tree)   tree_verify(tree, false)

    /**
     * @brief Tree assertion macros
     *
     * @param tree
     */
    #define TREE_ASSERT(tree)   TREE_VERIFY(tree);    \
                                TREE_OK(tree, res);   \
                                if (res != Tree::OK)  \
                                    return res

    /**
     * @brief No traversal Tree assertion macros
     *
     * @param tree
     */
    #define TREE_ASSERT_NO_TRAVERSAL(tree)                      \
                                TREE_VERIFY_NO_TRAVERSAL(tree); \
                                TREE_OK(tree, res);             \
                                if (res != Tree::OK)            \
                                    return res

    /**
     * @brief Node verificator
     *
     * @param tree
     * @param node
     * @param depth
     *
     * @return int
     */
    #define TREE_NODE_VERIFY(tree, node, depth) tree_node_verify(tree, node, depth)

    /**
     * @brief Node assertion function
     *
     * @param tree
     * @param node
     * @param depth
     */
    #define TREE_NODE_ASSERT(tree, node, depth) TREE_NODE_VERIFY(tree, node, depth);    \
                                                TREE_OK(tree, res);                     \
                                                if (res != Tree::OK)                    \
                                                    return res

#else //< #ifndef TREE_DEBUG

    /**
     * @brief Tree constructor
     *
     * @param tree
     * @param elem_size sizeof(Elem_t)
     * @param dtor Elem_t destructor function
     * @param verify not needed (TREE_DEBUG)
     * @param val_str not needed (TREE_DEBUG)
     *
     * @return int
     */
    #define TREE_CTOR(tree, elem_size, dtor, verify, val_str) tree_ctor(tree, elem_size, dtor)

    /**
     * @brief (Disabled) Tree dump
     *
     * @param tree
     */
    #define TREE_DUMP(tree) (void) 0

    /**
     * @brief (Disabled) Tree verificator
     *
     * @param tree
     *
     * @return int
     */
    #define TREE_VERIFY(tree)   Tree::OK

    /**
     * @brief (Disabled) No traversal Tree verificator
     *
     * @param tree
     *
     * @return int
     */
    #define TREE_VERIFY_NO_TRAVERSAL(tree) Tree::OK

    /**
     * @brief (Disabled) Tree assertion macros
     *
     * @param tree
     */
    #define TREE_ASSERT(tree)   Tree::OK

    /**
     * @brief (Disabled) No traversal Tree assertion macros
     *
     * @param tree
     */
    #define TREE_ASSERT_NO_TRAVERSAL(tree) Tree::OK

    /**
     * @brief (Disabled) Node verificator
     *
     * @param tree
     * @param node
     * @param depth
     *
     * @return int
     */
    #define TREE_NODE_VERIFY(tree, node, depth) Tree::OK

    /**
     * @brief (Disabled) Node assertion function
     *
     * @param tree
     * @param node
     * @param depth
     */
    #define TREE_NODE_ASSERT(tree, node, depth) Tree::OK

    /**
     * @brief (Disabled) Checks res and writes log if res is not OK
     *
     * @param tree
     * @param res
     */
    #define TREE_OK(tree, res) (void) 0

#endif //< #ifdef TREE_DEBUG

#endif //< #ifndef TREE_H_
