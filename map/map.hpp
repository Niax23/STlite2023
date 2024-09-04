/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

 // only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"
#include <cassert>

namespace sjtu {


    enum class Color {
        RED,
        BLACK
    };
    enum class Direction {
        LEFT,
        RIGHT,
        ROOT
    };
    template<
        class Key,
        class T,
        class Compare = std::less<Key>
    > class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
    private:



        struct RBTNode {
            Color color = Color::RED;
            value_type* data;
            RBTNode* left;    // 左孩子
            RBTNode* right;    // 右孩子
            RBTNode* parent; // 父结点

            RBTNode()
                : data(nullptr), left(nullptr), right(nullptr), parent(nullptr) {}

            RBTNode(const value_type& x, RBTNode* _parent = nullptr, RBTNode* _left = nullptr, RBTNode* _right = nullptr, Color _color = Color::RED)
                : parent(_parent), left(_left), right(_right), color(_color) {
                data = (value_type*)malloc(sizeof(value_type));
                new(data) value_type(x.first, x.second);
            }



            ~RBTNode() {
                // 释放data指向的内存（如果是动态分配的）
                if (data) {
                      data->first.~Key();
                data->second.~T();
                free(data);
                }

                // 注意：通常不需要在析构函数中释放 left, right, parent，因为它们通常是由树结构管理的
            }

            inline bool isLeaf() const noexcept {
                return this->left == nullptr && this->right == nullptr;
            }

            inline bool isRoot() const noexcept { return this->parent && this->parent->parent == nullptr; }

            inline bool isRed() const noexcept { return this->color == Color::RED; }

            inline bool isBlack() const noexcept { return this->color == Color::BLACK; }

            inline Direction direction() const noexcept {
                if (!this->isRoot()) {
                    if (this == this->parent->left) {
                        return Direction::LEFT;
                    }
                    else {
                        return Direction::RIGHT;
                    }
                }
                else {
                    return Direction::ROOT;
                }
            }

            inline RBTNode* sibling() const noexcept {
                assert(!this->isRoot());
                if (this->direction() == Direction::LEFT) {
                    return this->parent->right;
                }
                else {
                    return this->parent->left;
                }
            }

            inline bool hasSibling() const noexcept {
                return !this->isRoot() && this->sibling() != nullptr;
            }

            inline RBTNode* uncle() const noexcept {
                assert(!this->isRoot());
                return parent->sibling();
            }

            inline bool hasUncle() const noexcept {
                return !this->isRoot() && this->parent->hasSibling();
            }

            inline RBTNode* grandParent() const noexcept {
                assert(!this->parent->isRoot());
                return this->parent->parent;
            }

            inline bool hasGrandParent() const noexcept {
                return !this->isRoot() && !this->parent->isRoot();
            }


        };

        inline bool equal_key(Key x, Key y) const { return !(Compare()(x, y) || Compare()(y, x)); }

       // using Direction = typename RBTNode::Direction;

       void clear(RBTNode* node) {
            if (!node)return;
            node->parent = nullptr;
            if (node->left)clear(node->left);
            if (node->right)clear(node->right);
            delete node;
            node = nullptr;
            //注：这里只需要delete node即可 这样node会调用RBTnode的析构函数，自动释放data指针的空间 所以只要写好析构函数就可以
        }

        RBTNode* sentinel;
        size_t len;


        void copy(RBTNode*& node, RBTNode* ori) {
            if (!ori)return;
            node = new RBTNode(*(ori->data),nullptr,nullptr,nullptr,ori->color);
            copy(node->left, ori->left);
            if (node->left)node->left->parent = node;
            copy(node->right, ori->right);
            if (node->right)node->right->parent = node;

        }



        // 根据左右孩子 更新左右孩子的parent指针位置
        void maintainRelationship(RBTNode* node) {
            if (node->left != nullptr) {
                node->left->parent = node;
            }
            if (node->right != nullptr) {
                node->right->parent = node;
            }
        }





        void rotateLeft(RBTNode* node) {
            // clang-format off
            //     |                       |
            //     N                       S
            //    / \     l-rotate(N)     / \
            //   L   S    ==========>    N   R
            //      / \                 / \
            //     M   R               L   M
            assert(node != nullptr && node->right != nullptr);
            // clang-format on
            RBTNode* parent = node->parent;
            Direction direction = node->direction();

            RBTNode* successor = node->right;
            node->right = successor->left;
            successor->left = node;

            maintainRelationship(node);
            maintainRelationship(successor);

            switch (direction) {
            case Direction::ROOT:
                //this->root = successor;
                sentinel->left = successor;
                break;
            case Direction::LEFT:
                parent->left = successor;
                break;
            case Direction::RIGHT:
                parent->right = successor;
                break;
            }

            successor->parent = parent;
        }

        void rotateRight(RBTNode* node) {
            // clang-format off
            //       |                   |
            //       N                   S
            //      / \   r-rotate(N)   / \
            //     S   R  ==========>  L   N
            //    / \                     / \
            //   L   M                   M   R
            assert(node != nullptr && node->left != nullptr);
            // clang-format on

            RBTNode* parent = node->parent;
            Direction direction = node->direction();

            RBTNode* successor = node->left;
            node->left = successor->right;
            successor->right = node;

            maintainRelationship(node);
            maintainRelationship(successor);

            switch (direction) {
            case Direction::ROOT:
                //this->root = successor;
                sentinel->left = successor;
                break;
            case Direction::LEFT:
                parent->left = successor;
                break;
            case Direction::RIGHT:
                parent->right = successor;
                break;
            }

            successor->parent = parent;
        }



        void rotateSameDirection(RBTNode* node, Direction direction) {
            switch (direction)
            {
            case Direction::LEFT:
                rotateLeft(node);
                break;

            default:
                rotateRight(node);
                break;
            }
        }

        void rotateOppositeDirection(RBTNode* node, Direction direction) {
            switch (direction)
            {
            case Direction::LEFT:
                rotateRight(node);
                break;

            default:
                rotateLeft(node);
                break;
            }
        }


        void maintainAfterInsert(RBTNode* node) {
            assert(node != nullptr);

            if (node->isRoot()) {
                // Case 1: Current node is root (RED)

                //  No need to fix.

                // maybe i need to paint it to black
                node->color = Color::BLACK;
                return;
            }

            if (node->parent->isBlack()) {
                // Case 2: Parent is BLACK
                //  No need to fix.
                return;
            }


            if (node->hasUncle() && node->uncle()->isRed()) {
                // clang-format off
                // Case 4: Both parent and uncle are RED
                //   Paint parent and uncle to BLACK;
                //   Paint grandparent to RED.
                //        [G]             <G>
                //        / \             / \
                //      <P> <U>  ====>  [P] [U]
                //      /               /
                //    <N>             <N>
                // clang-format on
                assert(node->parent->isRed());
                node->parent->color = Color::BLACK;
                node->uncle()->color = Color::BLACK;
                node->grandParent()->color = Color::RED;
                maintainAfterInsert(node->grandParent());
                return;
            }

            if (!node->hasUncle() || node->uncle()->isBlack()) {
                // Case 5 & 6: Parent is RED and Uncle is BLACK
                //   p.s. NIL nodes are also considered BLACK
                assert(!node->isRoot());

                if (node->direction() != node->parent->direction()) {
                    // clang-format off
                    // Case 5: Current node is the opposite direction as parent
                    //   Step 1. If node is a LEFT child, perform l-rotate to parent;
                    //           If node is a RIGHT child, perform r-rotate to parent.
                    //   Step 2. Goto Case 6.
                    //      [G]                 [G]
                    //      / \    rotate(P)    / \
                    //    <P> [U]  ========>  <N> [U]
                    //      \                 /
                    //      <N>             <P>
                    // clang-format on

                    // Step 1: Rotation
                    RBTNode* parent = node->parent;
                    if (node->direction() == Direction::LEFT) {
                        rotateRight(node->parent);
                    }
                    else /* node->direction() == Direction::RIGHT */ {
                        rotateLeft(node->parent);
                    }
                    node = parent;
                    // Step 2: vvv
                }

                // clang-format off
                // Case 6: Current node is the same direction as parent
                //   Step 1. If node is a LEFT child, perform r-rotate to grandparent;
                //           If node is a RIGHT child, perform l-rotate to grandparent.
                //   Step 2. Paint parent (before rotate) to BLACK;
                //           Paint grandparent (before rotate) to RED.
                //        [G]                 <P>               [P]
                //        / \    rotate(G)    / \    repaint    / \
                //      <P> [U]  ========>  <N> [G]  ======>  <N> <G>
                //      /                         \                 \
                //    <N>                         [U]               [U]
                // clang-format on

                assert(node->grandParent() != nullptr);

                // Step 1
                if (node->parent->direction() == Direction::LEFT) {
                    rotateRight(node->grandParent());
                }
                else {
                    rotateLeft(node->grandParent());
                }

                // Step 2
                node->parent->color = Color::BLACK;
                node->sibling()->color = Color::RED;

                return;
            }
        }
        //内部用来递归的插入函数
        //注意这个指针引用 如果我把x->left(nullptr)传进来 给他赋予一个node x->left就也修改了
        pair<RBTNode*, bool> insert(const value_type& value, RBTNode*& node, RBTNode* p) {

            //找到合适的位置 插入 调整留着到外面的函数用
            if (!node) {
                node = new RBTNode(value, p);
                return pair<RBTNode*, bool>(node, true);
            }
            //key已经被使用了
            if (equal_key(node->data->first, value.first))return pair<RBTNode*, bool>(node, false);
            if (Compare()(value.first, node->data->first)) {
                return insert(value, node->left, node);
            }
            else {
                return insert(value, node->right, node);
            }
        }

        RBTNode* find(Key key, RBTNode* node)const {
            if (!node || equal_key(key, node->data->first))return node;
            if (Compare()(key, node->data->first))return find(key, node->left);
            return find(key, node->right);
        }


        static void swapNode(RBTNode* lhs, RBTNode* rhs) {
            if (lhs == rhs) return; // 如果两个节点相同，直接返回

            // 交换左右孩子指针
            std::swap(lhs->left, rhs->left);
            std::swap(lhs->right, rhs->right);

            // 更新左右孩子的父指针
            if (lhs->left) lhs->left->parent = lhs;
            if (lhs->right) lhs->right->parent = lhs;
            if (rhs->left) rhs->left->parent = rhs;
            if (rhs->right) rhs->right->parent = rhs;

            // 交换父指针
            std::swap(lhs->parent, rhs->parent);

            // 更新父节点的指针，确保父节点正确指向新位置的节点
            if (lhs->parent) {
                if (lhs->parent->left == rhs) {
                    lhs->parent->left = lhs;
                }
                else if (lhs->parent->right == rhs) {
                    lhs->parent->right = lhs;
                }
            }

            if (rhs->parent) {
                if (rhs->parent->left == lhs) {
                    rhs->parent->left = rhs;
                }
                else if (rhs->parent->right == lhs) {
                    rhs->parent->right = rhs;
                }
            }

            // 交换节点颜色（因为红黑树的平衡取决于颜色）
            std::swap(lhs->color, rhs->color);
        }



        void maintainAfterRemove(RBTNode* node) {
            if (node->isRoot()) {
                return;
            }

            assert(node->isBlack() && node->hasSibling());

            Direction direction = node->direction();

            RBTNode* sibling = node->sibling();
            if (sibling->isRed()) {
                // clang-format off
                // Case 1: Sibling is RED, parent and nephews must be BLACK
                //   Step 1. If N is a left child, left rotate P;
                //           If N is a right child, right rotate P.
                //   Step 2. Paint S to BLACK, P to RED
                //   Step 3. Goto Case 2, 3, 4, 5
                //      [P]                   <S>               [S]
                //      / \    l-rotate(P)    / \    repaint    / \
                //    [N] <S>  ==========>  [P] [D]  ======>  <P> [D]
                //        / \               / \               / \
                //      [C] [D]           [N] [C]           [N] [C]
                // clang-format on
                RBTNode* parent = node->parent;
                assert(parent != nullptr && parent->isBlack());
                assert(sibling->left != nullptr && sibling->left->isBlack());
                assert(sibling->right != nullptr && sibling->right->isBlack());
                // Step 1
                rotateSameDirection(node->parent, direction);
                // Step 2
                sibling->color = Color::BLACK;
                parent->color = Color::RED;
                // Update sibling after rotation
                sibling = node->sibling();
                // Step 3: vvv
            }

            RBTNode* closeNephew =
                direction == Direction::LEFT ? sibling->left : sibling->right;
            RBTNode* distantNephew =
                direction == Direction::LEFT ? sibling->right : sibling->left;

            bool closeNephewIsBlack = closeNephew == nullptr || closeNephew->isBlack();
            bool distantNephewIsBlack =
                distantNephew == nullptr || distantNephew->isBlack();

            assert(sibling->isBlack());

            if (closeNephewIsBlack && distantNephewIsBlack) {
                //兄弟的孩子都是黑色

                if (node->parent->isRed()) {
                    //父为红 变单黑 直接结束
                    // clang-format off
                    // Case 2: Sibling and nephews are BLACK, parent is RED
                    //   Swap the color of P and S
                    //      <P>             [P]
                    //      / \             / \
                    //    [N] [S]  ====>  [N] <S>
                    //        / \             / \
                    //      [C] [D]         [C] [D]
                    // clang-format on
                    sibling->color = Color::RED;
                    node->parent->color = Color::BLACK;
                    return;
                }
                else {
                    //父为黑 双黑上移
                    // clang-format off
                    // Case 3: Sibling, parent and nephews are all black
                    //   Step 1. Paint S to RED
                    //   Step 2. Recursively maintain P
                    //      [P]             [P]
                    //      / \             / \
                    //    [N] [S]  ====>  [N] <S>
                    //        / \             / \
                    //      [C] [D]         [C] [D]
                    // clang-format on
                    sibling->color = Color::RED;
                    maintainAfterRemove(node->parent);
                    return;
                }
            }
            else {
                //兄弟有一个子节点是红色
                if (closeNephew != nullptr && closeNephew->isRed()) { //为什么不要保证dist nephew是黑色
                    // clang-format off
                    // Case 4: Sibling is BLACK, close nephew is RED,
                    //         distant nephew is BLACK
                    //   Step 1. If N is a left child, right rotate S;
                    //           If N is a right child, left rotate S.
                    //   Step 2. Swap the color of close nephew and sibling
                    //   Step 3. Goto case 5
                    //                            {P}                {P}
                    //      {P}                   / \                / \
                    //      / \    r-rotate(S)  [N] <C>   repaint  [N] [C]
                    //    [N] [S]  ==========>        \   ======>        \
                    //        / \                     [S]                <S>
                    //      <C> [D]                     \                  \
                    //                                  [D]                [D]
                    // clang-format on

                    // Step 1
                    rotateOppositeDirection(sibling, direction);
                    // Step 2
                    closeNephew->color = Color::BLACK;
                    sibling->color = Color::RED;
                    // Update sibling and nephews after rotation
                    sibling = node->sibling();
                    closeNephew =
                        direction == Direction::LEFT ? sibling->left : sibling->right;
                    distantNephew =
                        direction == Direction::LEFT ? sibling->right : sibling->left;
                    // Step 3: vvv
                }

                // clang-format off
                // Case 5: Sibling is BLACK, distant nephew is RED
                //   Step 1. If N is a left child, left rotate P;
                //           If N is a right child, right rotate P.
                //   Step 2. Swap the color of parent and sibling.
                //   Step 3. Paint distant nephew D to BLACK.
                //      {P}                   [S]               {S}
                //      / \    l-rotate(P)    / \    repaint    / \
                //    [N] [S]  ==========>  {P} <D>  ======>  [P] [D]
                //        / \               / \               / \
                //      {C} <D>           [N] {C}           [N] {C}
                // clang-format on
                assert(distantNephew->isRed());
                // Step 1
                rotateSameDirection(node->parent, direction);
                // Step 2
                sibling->color = node->parent->color;
                node->parent->color = Color::BLACK;
                if (distantNephew != nullptr) {
                    distantNephew->color = Color::BLACK;
                }
                return;
            }
        }









        bool remove(RBTNode* node, Key key) {
            assert(node != nullptr);

            if (!equal_key(key , node->data->first)) {
                if (Compare()(key, node->data->first)) {
                    /* key < node->key */
                    RBTNode*& left = node->left;
                    if (left != nullptr && remove(left, key)) {
                        maintainRelationship(node);
                        return true;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    /* key > node->key */
                    RBTNode* right = node->right;
                    if (right != nullptr && remove(right, key)) {
                        maintainRelationship(node);
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }

            assert(equal_key(key, node->data->first));
            //action(node);

            if (this->size() == 1) {
                // Current node is the only node of the tree
                sentinel->left = nullptr;
                delete node;
                return true;
            }

            if (node->left != nullptr && node->right != nullptr) {
                // clang-format off
                // Case 1: If the node is strictly internal
                //   Step 1. Find the successor S with the smallest key
                //           and its parent P on the right subtree.
                //   Step 2. Swap the data (key and value) of S and N,
                //           S is the node that will be deleted in place of N.
                //   Step 3. N = S, goto Case 2, 3
                //     |                    |
                //     N                    S
                //    / \                  / \
                //   L  ..   swap(N, S)   L  ..
                //       |   =========>       |
                //       P                    P
                //      / \                  / \
                //     S  ..                N  ..
                // clang-format on

                // Step 1
                RBTNode* p = node->parent;//
                RBTNode* successor = node->right;
                RBTNode* parent = node;
                while (successor->left != nullptr) {
                    parent = successor;
                    successor = parent->left;
                }
                // Step 2
                swapNode(node, successor);
                maintainRelationship(parent);
                maintainRelationship(p);//
               // node = successor;
                // Step 3: vvv
            }

            if (node->isLeaf()) {
                // Current node must not be the root
                assert(!node->isRoot());

                // Case 2: Current node is a leaf
                //   Step 1. Unlink and remove it.
                //   Step 2. If N is BLACK, maintain N;
                //           If N is RED, do nothing.

                // The maintain operation won't change the node itself,
                //  so we can perform maintain operation before unlink the node.
                if (node->isBlack()) {
                    maintainAfterRemove(node);
                }
                if (node->direction() == Direction::LEFT) {
                    node->parent->left = nullptr;
                }
                else /* node->direction() == Direction::RIGHT */ {
                    node->parent->right = nullptr;
                }
            }
            else /* !node->isLeaf() */ {
                assert(node->left == nullptr || node->right == nullptr);
                // Case 3: Current node has a single left or right child
                //   Step 1. Replace N with its child
                //   Step 2. If N is BLACK, maintain N
                RBTNode* parent = node->parent;
                RBTNode* replacement = (node->left != nullptr ? node->left : node->right);
                switch (node->direction()) {
                case Direction::ROOT:
                    sentinel->left = replacement;
                    break;
                case Direction::LEFT:
                    parent->left = replacement;
                    break;
                case Direction::RIGHT:
                    parent->right = replacement;
                    break;
                }
                replacement->parent = parent;

                if (node->isBlack()) {
                    if (replacement->isRed()) {
                        replacement->color = Color::BLACK;
                    }
                    else {
                        maintainAfterRemove(replacement);
                    }
                }
            }

            delete node;
            node = nullptr;
            return true;
        }

         void remove(RBTNode* node) {
            assert(node != nullptr);
            if (this->size() == 1) {
                // Current node is the only node of the tree
                sentinel->left = nullptr;
                delete node;
                return;
            }

            if (node->left != nullptr && node->right != nullptr) {
                // clang-format off
                // Case 1: If the node is strictly internal
                //   Step 1. Find the successor S with the smallest key
                //           and its parent P on the right subtree.
                //   Step 2. Swap the data (key and value) of S and N,
                //           S is the node that will be deleted in place of N.
                //   Step 3. N = S, goto Case 2, 3
                //     |                    |
                //     N                    S
                //    / \                  / \
                //   L  ..   swap(N, S)   L  ..
                //       |   =========>       |
                //       P                    P
                //      / \                  / \
                //     S  ..                N  ..
                // clang-format on

                // Step 1
                RBTNode* p = node->parent;//
                RBTNode* successor = node->right;
                RBTNode* parent = node;
                while (successor->left != nullptr) {
                    parent = successor;
                    successor = parent->left;
                }
                // Step 2
                swapNode(node, successor);
                maintainRelationship(parent);
                maintainRelationship(p);//
               // node = successor;
                // Step 3: vvv
            }

            if (node->isLeaf()) {
                // Current node must not be the root
                assert(!node->isRoot());

                // Case 2: Current node is a leaf
                //   Step 1. Unlink and remove it.
                //   Step 2. If N is BLACK, maintain N;
                //           If N is RED, do nothing.

                // The maintain operation won't change the node itself,
                //  so we can perform maintain operation before unlink the node.
                if (node->isBlack()) {
                    maintainAfterRemove(node);
                }
                if (node->direction() == Direction::LEFT) {
                    node->parent->left = nullptr;
                }
                else /* node->direction() == Direction::RIGHT */ {
                    node->parent->right = nullptr;
                }
            }
            else /* !node->isLeaf() */ {
                assert(node->left == nullptr || node->right == nullptr);
                // Case 3: Current node has a single left or right child
                //   Step 1. Replace N with its child
                //   Step 2. If N is BLACK, maintain N
                RBTNode* parent = node->parent;
                RBTNode* replacement = (node->left != nullptr ? node->left : node->right);
                switch (node->direction()) {
                case Direction::ROOT:
                    sentinel->left = replacement;
                    break;
                case Direction::LEFT:
                    parent->left = replacement;
                    break;
                case Direction::RIGHT:
                    parent->right = replacement;
                    break;
                }
                replacement->parent = parent;

                if (node->isBlack()) {
                    if (replacement->isRed()) {
                        replacement->color = Color::BLACK;
                    }
                    else {
                        maintainAfterRemove(replacement);
                    }
                }
            }

            delete node;
            node = nullptr;
            return;
        }



















    public:
        class const_iterator;
        class iterator {
            friend class map;
        private:
            /**
             * TODO add data members
             *   just add whatever you want.
             */
            map* map_ptr;
            RBTNode* ptr;









        public:

            iterator() {
                // TODO
                map_ptr = nullptr;
                ptr = nullptr;
            }

            iterator(map* _m , RBTNode* _n) :map_ptr(_m), ptr(_n) {
                // TODO
            }
            iterator(const iterator& other) :map_ptr(other.map_ptr), ptr(other.ptr) {
                // TODO
            }
            /**
             * TODO iter++
             */
            iterator operator++(int) {
                if (ptr == map_ptr->sentinel) throw invalid_iterator();
                RBTNode* cur = ptr;
                //先找右子树
                if (ptr->right) {
                    ptr = ptr->right;
                    while (ptr->left)ptr = ptr->left;
                }
                //右子树没有 就找第一个在左子树的第一个祖先
                else {
                    while (ptr->parent->left != ptr) {
                       ptr = ptr->parent;
                    }
                    ptr = ptr->parent;
                }
                return iterator(map_ptr, cur);
            }
            /**
             * TODO ++iter
             */
            iterator& operator++() {
                if (ptr == map_ptr->sentinel) throw invalid_iterator();
                //先找右子树
                if (ptr->right) {
                    ptr = ptr->right;
                    while (ptr->left)ptr = ptr->left;
                }
                //右子树没有 就找第一个在左子树的第一个祖先
                else
                    while (1) {
                        if (ptr->direction() == Direction::RIGHT)ptr = ptr->parent;
                        else { ptr = ptr->parent; break; }
                    }
                return *this;
            }
            /**
             * TODO iter--
             */
            iterator operator--(int) {
                RBTNode* begin_ptr = map_ptr->sentinel;
                while (begin_ptr->left)begin_ptr = begin_ptr->left;
                if (ptr == begin_ptr)throw invalid_iterator();
                RBTNode* cur = ptr;
                if (ptr->left) {
                    ptr = ptr->left;
                    while (ptr->right)ptr = ptr->right;
                }
                else {
                    //只有begin才会一路回到根 这种情况已经被排除
                    while (ptr->direction() == Direction::LEFT)ptr = ptr->parent;
                    ptr = ptr->parent;
                }
                return iterator(map_ptr, cur);
            }
            /**
             * TODO --iter
             */
            iterator& operator--() {
                RBTNode* begin_ptr = map_ptr->sentinel;
                while (begin_ptr->left)begin_ptr = begin_ptr->left;
                if (ptr == begin_ptr)throw invalid_iterator();
                if (ptr->left) {
                    ptr = ptr->left;
                    while (ptr->right)ptr = ptr->right;
                }
                else {
                    while (ptr->direction() == Direction::LEFT)ptr = ptr->parent;
                    ptr = ptr->parent;
                }
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type& operator*() const {
                if (ptr == map_ptr->sentinel) throw invalid_iterator();
                return *(ptr->data);
            }
            bool operator==(const iterator& rhs) const {
                return map_ptr == rhs.map_ptr && ptr == rhs.ptr;
            }
            bool operator==(const const_iterator& rhs) const {
                return map_ptr == rhs.map_ptr && ptr == rhs.ptr;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator& rhs) const {
                return map_ptr != rhs.map_ptr || ptr != rhs.ptr;
            }
            bool operator!=(const const_iterator& rhs) const {
                return map_ptr != rhs.map_ptr || ptr != rhs.ptr;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
             //为什么不要判定异常 还是不太理解这个运算符
            value_type* operator->() const noexcept {
                return ptr->data;
            }
        };
        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            friend class map;
        private:
            // data members.
            const map* map_ptr;
            RBTNode* ptr;
        public:
            const_iterator(const map* _m = nullptr, RBTNode* _n = nullptr){
                // TODO
                map_ptr = _m;
                ptr = _n;
            }
            const_iterator(const const_iterator& other) :map_ptr(other.map_ptr), ptr(other.ptr) {
                // TODO
            }
            const_iterator(const iterator& other) :map_ptr(other.map_ptr), ptr(other.ptr) {
                // TODO
            }
            // And other methods in iterator.
            // And other methods in iterator.
            // And other methods in iterator.



       /**
         * TODO iter++
         */
            const_iterator operator++(int) {
                if (ptr == map_ptr->sentinel) throw invalid_iterator();
                RBTNode* cur = ptr;
                //先找右子树
                if (ptr->right) {
                    ptr = ptr->right;
                    while (ptr->left)ptr = ptr->left;
                }
                //右子树没有 就找第一个在左子树的第一个祖先
                else
                    while (1) {
                        if (ptr->direction() == Direction::RIGHT)ptr = ptr->parent;
                        else { ptr = ptr->parent; break; }
                    }
                return const_iterator(map_ptr, cur);
            }
            /**
             * TODO ++iter
             */
            const_iterator& operator++() {
                if (ptr == map_ptr->sentinel) throw invalid_iterator();
                //先找右子树
                if (ptr->right) {
                    ptr = ptr->right;
                    while (ptr->left)ptr = ptr->left;
                }
                //右子树没有 就找第一个在左子树的第一个祖先
                else
                    while (1) {
                        if (ptr->direction() == Direction::RIGHT)ptr = ptr->parent;
                        else { ptr = ptr->parent; break; }
                    }
                return *this;
            }
            /**
             * TODO iter--
             */
            const_iterator operator--(int) {
                RBTNode* begin_ptr = map_ptr->sentinel;
                while (begin_ptr->left)begin_ptr = begin_ptr->left;
                if (ptr == begin_ptr)throw invalid_iterator();
                RBTNode* cur = ptr;
                if (ptr->left) {
                    ptr = ptr->left;
                    while (ptr->right)ptr = ptr->right;
                }
                else {
                    //只有begin才会一路回到根 这种情况已经被排除
                    while (ptr->direction() == Direction::LEFT)ptr = ptr->parent;
                    ptr = ptr->parent;
                }
                return const_iterator(map_ptr, cur);
            }
            /**
             * TODO --iter
             */
            const_iterator& operator--() {
                RBTNode* begin_ptr = map_ptr->sentinel;
                while (begin_ptr->left)begin_ptr = begin_ptr->left;
                if (ptr == begin_ptr)throw invalid_iterator();
                RBTNode* cur = ptr;
                if (ptr->left) {
                    ptr = ptr->left;
                    while (ptr->right)ptr = ptr->right;
                }
                else {
                    //只有begin才会一路回到根 这种情况已经被排除
                    while (ptr->direction() == Direction::LEFT)ptr = ptr->parent;
                    ptr = ptr->parent;
                }
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            const value_type& operator*() const {
                if (ptr == map_ptr->sentinel) throw invalid_iterator();
                return *(ptr->data);
            }
            bool operator==(const iterator& rhs) const {
                return map_ptr == rhs.map_ptr && ptr == rhs.ptr;
            }
            bool operator==(const const_iterator& rhs) const {
                return map_ptr == rhs.map_ptr && ptr == rhs.ptr;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator& rhs) const {
                return map_ptr != rhs.map_ptr || ptr != rhs.ptr;
            }
            bool operator!=(const const_iterator& rhs) const {
                return map_ptr != rhs.map_ptr || ptr != rhs.ptr;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            const value_type* operator->() const noexcept {
                return ptr->data;
            }











        };
        /**
         * TODO two constructors
         */
        map() {
            len = 0;
            sentinel = new RBTNode();
        }
        map(const map& other) {
            len = other.len;
            sentinel = new RBTNode();
            //dfs一遍 走到哪复制到哪
            copy(sentinel->left, other.sentinel->left);
            //copy一个节点只会更新其子节点的父信息 所以sentinel的子节点要额外连起来
            if (sentinel->left)sentinel->left->parent = sentinel;
        }
        /**
         * TODO assignment operator
         */
        map& operator=(const map& other) {
            //先判断是否赋值自己
            if (this == &other)return *this;
            //清除当前内容
            clear(sentinel->left);
            copy(sentinel->left, other.sentinel->left);
            if (sentinel->left)sentinel->left->parent = sentinel;
            len = other.len;
            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map() {
            clear(sentinel->left);
            delete sentinel;
            sentinel = nullptr;
        }
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T& at(const Key& key) {
            RBTNode* p = find(key, sentinel->left);
            if (p)return p->data->second;
            throw index_out_of_bound();
        }
        const T& at(const Key& key) const {
            RBTNode* p = find(key, sentinel->left);
            if (p)return p->data->second;
            throw index_out_of_bound();
        }
        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T& operator[](const Key& key) {
            pair<iterator, bool> tmp = insert(value_type(key, T()));
            return tmp.first->second;

            /*RBTNode* p = find(key, sentinel->left);
            if (p) return p->data->second;
            pair<RBTNode*, bool> tmp = insert(value_type(key, T()), sentinel->left, sentinel);
            ++len;
            return tmp.first->data->second;*/
        }
        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T& operator[](const Key& key) const {
            RBTNode* p = find(key, sentinel->left);
            if (p)return p->data->second;
            throw index_out_of_bound();
        }
        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            RBTNode* node = sentinel;
            while (node->left)node = node->left;
            return iterator(this, node);
        }
        const_iterator cbegin() const {
            RBTNode* node = sentinel;
            while (node->left)node = node->left;
            return const_iterator(this, node);
        }
        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            return iterator(this, sentinel);
        }
        const_iterator cend() const {
            return const_iterator(this, sentinel);
        }
        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return len == 0;
        }
        /**
         * returns the number of elements.
         */
        size_t size() const {
            return len;
        }
        /**
         * clears the contents
         */
        void clear() {
            clear(sentinel->left);
            sentinel->left = nullptr;
            len = 0;
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */





        pair<iterator, bool> insert(const value_type& value) {
            pair<RBTNode*, bool>p = insert(value, sentinel->left, sentinel);
            if (p.second) { maintainAfterInsert(p.first); len++; }
            return pair<iterator, bool>(iterator(this, p.first), p.second);
        }
        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos == end() || pos.map_ptr != this)throw invalid_iterator();
            //感觉可以修改一下
            //if (remove(sentinel->left, pos->first))len--;
            remove(pos.ptr);
            len--;
        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key& key) const {
            RBTNode* node = find(key,sentinel->left);
            if (node) return 1;
            return 0;
        }
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */

        iterator find(const Key& key) {
            RBTNode* node = find(key, sentinel->left);
            if (node)return iterator(this, node);
            return end();
        }
        const_iterator find(const Key& key) const {
            RBTNode* node = find(key, sentinel->left);
            if (node)return const_iterator(this, node);
            return cend();
        }
    };

}

#endif
