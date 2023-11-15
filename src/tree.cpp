#include "tree.h"

/**
 * @brief Node constructor
 *
 * @param node
 * @param elem
 * @param elem_size
 * @param parent
 * @return int
 */
static int tree_node_ctor_(TreeNode** node, void* elem, size_t elem_size, TreeNode* parent);

/**
 * @brief Node destructor
 *
 * @param tree
 * @param node
 * @return int
 */
static int tree_node_dtor_(Tree* tree, TreeNode** node);

/**
 * @brief Recursive traversal function
 *
 * @param tree
 * @param node
 * @param args
 * @param actions
 * @param depth
 * @param side
 * @return int
 */
static int tree_traversal_(Tree* tree, TreeNode** node, va_list* args, TreeNodeAction* actions[4],
                           size_t depth, TreeChildSide side);

/**
 * @brief Traversal node deletion action function
 *
 * @param tree
 * @param node
 * @param args
 * @param depth
 * @param side
 * @return TreeNodeActionRes
 */
static TreeNodeActionRes tree_delete_node_(Tree* tree, TreeNode** node, va_list* args,
                                           size_t depth, const TreeChildSide side);

int tree_ctor(Tree* tree, const size_t elem_size, Elem_t_func* elem_t_dtor) {
    assert(tree);
    assert(elem_size > 0);
    // elem_t_dtor can be nullptr

    int res = tree->OK;

    if (tree_is_initialised(tree)) {
        res |= tree->ALREADY_INITIALISED;
        TREE_OK(tree, res);
        return res;
    }

    tree->size = 0;
    tree->elem_size = elem_size;
    tree->node_elem_t_dtor = elem_t_dtor;

    res |= TREE_ASSERT(tree);
    return res;
}

int tree_dtor(Tree* tree) {
    assert(tree);

    int res = TREE_ASSERT(tree);

    res |= tree_delete(tree, &tree->root, true);
    if (res != Tree::OK)
        return res;

    assert(tree->root == nullptr);

    tree->size = Tree::UNITIALISED_VAL;
    tree->elem_size = Tree::UNITIALISED_VAL;

    return res;
}

static int tree_node_ctor_(TreeNode** node, void* elem, size_t elem_size, TreeNode* parent) {
    assert(node);
    assert(elem);
    assert(elem_size > 0);

    int res = Tree::OK;

    if (*node != nullptr) {
        res |= Tree::ALREADY_INITIALISED;
        return res;
    }

    *node = (TreeNode*)calloc(1, sizeof(TreeNode) + elem_size);

    if (*node == nullptr) {
        res |= Tree::ALLOC_ERR;
        return res;
    }

    (*node)->elem = *node + sizeof(TreeNode);
    memcpy((*node)->elem, elem, elem_size);

    (*node)->parent = parent;

    return res;
}

static int tree_node_dtor_(Tree* tree, TreeNode** node) {
    assert(tree);
    assert(node);

    int res = Tree::OK;

    if (*node == nullptr) {
        res |= Tree::INVALID_NODE_PTR;
        return res;
    }

    if ((*node)->left != nullptr || (*node)->right != nullptr) {
        res |= Tree::CANNOT_DELETE;
        return res;
    }

    if (tree->node_elem_t_dtor != nullptr) {
        if (!tree->node_elem_t_dtor((*node)->elem)) {
            res |= Tree::CANNOT_DELETE;
            return res;
        }
    }

    (*node)->parent = nullptr;

    FREE(*node);

    return res;
}

int tree_insert(Tree* tree, TreeNode** node, TreeNode* parent, void* elem) {
    assert(tree);
    assert(node);
    assert(elem);

    int res = TREE_ASSERT(tree);

    res |= tree_node_ctor_(node, elem, tree->elem_size, parent);
    if (res != Tree::OK) {
        TREE_OK(tree, res);
        return res;
    }

    tree->size++;

    res |= TREE_ASSERT(tree);
    return res;
}

static TreeNodeActionRes tree_delete_node_(Tree* tree, TreeNode** node, va_list* args,
                                           size_t depth, const TreeChildSide side) {
    (void) tree;
    (void) args;
    (void) depth;
    (void) side;
    assert(node);

    int res = tree_node_dtor_(tree, node);
    if (res != Tree::OK)
        return TreeNodeActionRes::ERROR;

    return TreeNodeActionRes::EXIT;
}

int tree_delete(Tree* tree, TreeNode** node, bool recursive) {
    int res = TREE_ASSERT(tree);

    if (*node == nullptr) {
        res |= Tree::INVALID_NODE_PTR;
        TREE_OK(tree, res);
        return res;
    }

    if (recursive)
        res |= tree_postorder(tree, node, &tree_delete_node_);
    else
        res |= tree_node_dtor_(tree, node);

    if (res != Tree::OK) {
        TREE_OK(tree, res);
        return res;
    }

    tree->size--;

    return res;
}

#define DO_ACTION_(action_) do {                                                                        \
                                if (action_ == nullptr)                                                 \
                                    continue;                                                           \
                                                                                                        \
                                TreeNodeActionRes action_res = action_(tree, node, args, depth, side);  \
                                if (action_res == TreeNodeActionRes::ERROR) {                           \
                                    res |= Tree::ACTION_ERROR;                                          \
                                    return res;                                                         \
                                }                                                                       \
                                                                                                        \
                                if (action_res == TreeNodeActionRes::EXIT)                              \
                                    return res;                                                         \
                            } while (0)

static int tree_traversal_(Tree* tree, TreeNode** node, va_list* args, TreeNodeAction* actions[4],
                           size_t depth, TreeChildSide side) {
    assert(tree);
    assert(node);
    assert(args);

    int res = tree->OK;

    if (*node == nullptr) {
        DO_ACTION_(actions[3]);
        return res;
    }

    res |= TREE_NODE_VERIFY(tree, *node, depth);
    if (res != Tree::OK)
        return res;

    DO_ACTION_(actions[0]);

    res |= tree_traversal_(tree, &(*node)->left, args, actions, depth + 1, TreeChildSide::LEFT);
    if (res != Tree::OK)
        return res;

    DO_ACTION_(actions[1]);

    res |= tree_traversal_(tree, &(*node)->right, args, actions, depth + 1, TreeChildSide::RIGHT);
    if (res != Tree::OK)
        return res;

    DO_ACTION_(actions[2]);

    res |= TREE_NODE_VERIFY(tree, *node, depth);
    return res;
}

int tree_traversal(Tree* tree, TreeNode** root, TreeNodeAction* actions[4], ...) {
    assert(tree);
    assert(root);
    assert(actions);

    int res = TREE_ASSERT_NO_TRAVERSAL(tree);

    va_list args = {};
    va_start(args, actions);

    res |= tree_traversal_(tree, root, &args, actions, 0, TreeChildSide::ROOT);

    va_end(args);

    if (res != Tree::OK)
        return res;

    res |= TREE_ASSERT_NO_TRAVERSAL(tree);
    return res;
}

int tree_preorder(Tree* tree, TreeNode** root, TreeNodeAction* action, ...) {
    assert(tree);
    assert(root);
    assert(action);

    int res = TREE_ASSERT_NO_TRAVERSAL(tree);

    va_list args = {};
    va_start(args, action);

    TreeNodeAction* actions[4] = {action, nullptr, nullptr, nullptr};

    res |= tree_traversal_(tree, root, &args, actions, 0, TreeChildSide::ROOT);

    va_end(args);

    if (res != Tree::OK)
        return res;

    res |= TREE_ASSERT_NO_TRAVERSAL(tree);
    return res;
}

int tree_inorder(Tree* tree, TreeNode** root, TreeNodeAction* action, ...) {
    assert(tree);
    assert(root);
    assert(action);

    int res = TREE_ASSERT_NO_TRAVERSAL(tree);

    va_list args = {};
    va_start(args, action);

    TreeNodeAction* actions[4] = {nullptr, action, nullptr, nullptr};

    res |= tree_traversal_(tree, root, &args, actions, 0, TreeChildSide::ROOT);

    va_end(args);

    if (res != Tree::OK)
        return res;

    res |= TREE_ASSERT_NO_TRAVERSAL(tree);
    return res;
}

int tree_postorder(Tree* tree, TreeNode** root, TreeNodeAction* action, ...) {
    assert(tree);
    assert(root);
    assert(action);

    int res = TREE_ASSERT_NO_TRAVERSAL(tree);

    va_list args = {};
    va_start(args, action);

    TreeNodeAction* actions[4] = {nullptr, nullptr, action, nullptr};

    res |= tree_traversal_(tree, root, &args, actions, 0, TreeChildSide::ROOT);

    va_end(args);

    if (res != Tree::OK)
        return res;

    res |= TREE_ASSERT_NO_TRAVERSAL(tree);
    return res;
}

#ifdef TREE_DEBUG

static TreeNodeActionRes tree_node_count_(Tree* tree, TreeNode** node, va_list* args,
                                          size_t depth, const TreeChildSide side) {
    (void) depth;
    (void) side;
    assert(tree);
    assert(node);

    va_list args_dup = {};
    va_copy(args_dup, *args);

    size_t* cnt = va_arg(args_dup, size_t*);
    va_end(args_dup);

    (*cnt)++;

    return TreeNodeActionRes::OK;
}

#define CHECK_ERR_(clause, err) if (clause) res |= err

int tree_verify(Tree* tree, bool do_traversal) {
    assert(tree);

    int res = Tree::OK;

    if (!tree_is_initialised(tree)) {
        res |= Tree::UNITIALISED;
        return res;
    }

    CHECK_ERR_(tree->elem_size <= 0, tree->INVALID_ELEM_T_SIZE);
    CHECK_ERR_(tree->size < 0, tree->NEGATIVE_SIZE);

    if (do_traversal) {
        size_t cnt = 0;
        res |= tree_preorder(tree, &tree->root, &tree_node_count_, &cnt);

        CHECK_ERR_(res != tree->ACTION_ERROR && (ssize_t)cnt != tree->size, tree->INVALID_SIZE);
    }
    return res;
}

int tree_node_verify(const Tree* tree, const TreeNode* node, const size_t depth) {
    assert(tree);
    assert(node);

    int res = tree->OK;

    if (!tree_node_is_initialised(node)) {
        res |= tree->UNITIALISED;
        return res;
    }

    CHECK_ERR_(node != tree->root && (node->parent == nullptr ||
               !(node->parent->left == node || node->parent->right == node)), tree->DAMAGED_PATH);

    CHECK_ERR_((ssize_t)depth > tree->size, tree->DAMAGED_PATH);

    if (tree->node_elem_t_verify != nullptr)
        CHECK_ERR_(!tree->node_elem_t_verify(node->elem), tree->NODE_VERIFY_FAIL);

    return res;
}
#undef CHECK_ERR_

int tree_ctor_debug(Tree* tree, const size_t elem_size,
                    Elem_t_func* elem_t_dtor, Elem_t_func* elem_t_verify,
                    Elem_t_val_str* elem_t_val_str, const VarCodeData var_data) {
    assert(tree);

    tree->var_data = var_data;
    tree->node_elem_t_verify = elem_t_verify;
    tree->node_elem_t_val_str = elem_t_val_str;

    return tree_ctor(tree, elem_size, elem_t_dtor);
}

#endif //< #ifdef TREE_DEBUG
