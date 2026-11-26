extern "C" {

// Intrusive red/black tree links: +0 left child, +4 right child, +8 parent | red.
// A zero child is a black leaf. Nodes/parent links must be at least two-byte aligned.
// The root has a usable parent sentinel: rotations update that sentinel's child link
// as well as the local root. Neither repair routine allocates or frees nodes.
struct Node8005DA74 {
    Node8005DA74* leftChild;
    Node8005DA74* rightChild;
    unsigned int tag;
};

// Recover a node's parent without its packed red bit; node must be non-null.
static Node8005DA74* owner8005DA74(Node8005DA74* node) {
    return (Node8005DA74*)(node->tag & ~1u);
}

// Repair insertion colors/links, then blacken the root (root changes stay in sentinel links).
// end is the inserted node, stop the current root; node is overwritten as parent scratch.
// In the loop end moves upward; own/other change from grandparent/uncle to rotation links.
void RBTree_RebalanceAfterInsert(Node8005DA74* end, Node8005DA74* stop, Node8005DA74* node) {
    Node8005DA74* own;
    Node8005DA74* other;
    end->tag = end->tag | 1;
    // 0x8005DDC8: rebalance while the parent is red and the root is not reached.
    while (end != stop && (node = owner8005DA74(end), (node->tag & 1) == 1)) {
        own = (Node8005DA74*)(((volatile Node8005DA74*)node)->tag & ~1u);
        other = own->leftChild;
        if (node == other) {
            // Parent is the grandparent's left child: red uncle -> recolor and ascend.
            own = own->rightChild;
            if (own != 0 && (own->tag & 1) == 1) {
                node->tag &= ~1u;
                own->tag &= ~1u;
                end = owner8005DA74(owner8005DA74(end));
                end->tag |= 1;
                continue;
            }

            {
                // Black uncle, inner (left-right) insertion: rotate parent left first.
                Node8005DA74* head = node->rightChild;
                if (end == head) {
                    end = node;
                    if (stop == node)
                        stop = head;
                    node->rightChild = head->leftChild;
                    if (head->leftChild)
                        head->leftChild->tag = (unsigned int)node | (head->leftChild->tag & 1);
                    head->tag = (node->tag & ~1u) | (head->tag & 1);
                    Node8005DA74* own = owner8005DA74(node);
                    if (node == own->leftChild)
                        own->leftChild = head;
                    else
                        own->rightChild = head;
                    head->leftChild = node;
                    node->tag = (unsigned int)head | (node->tag & 1);
                }
            }

            {
                // Outer (left-left) insertion: blacken parent, redden grandparent, rotate right.
                node = owner8005DA74(end);
                node->tag &= ~1u;
                node = owner8005DA74(end);
                node = owner8005DA74(node);
                node->tag |= 1;
                node = owner8005DA74(end);
                other = (Node8005DA74*)(((volatile Node8005DA74*)node)->tag & ~1u);
                own = other->leftChild;
                if (stop == other)
                    stop = own;
                other->leftChild = own->rightChild;
                node = own->rightChild;
                if (node)
                    node->tag = (unsigned int)other | (node->tag & 1);
                own->tag = (other->tag & ~1u) | (own->tag & 1);
                node = owner8005DA74(other);
                if (other == node->leftChild)
                    node->leftChild = own;
                else
                    node->rightChild = own;
                own->rightChild = other;
                other->tag = (unsigned int)own | (other->tag & 1);
            }
        } else {
            // Mirror cases: parent is on the right; the uncle is on the left.
            if (other != 0 && (other->tag & 1) == 1) {
                node->tag &= ~1u;
                other->tag &= ~1u;
                end = owner8005DA74(owner8005DA74(end));
                end->tag |= 1;
                continue;
            }

            {
                // Inner (right-left) insertion: rotate parent right to make an outer case.
                Node8005DA74* head = node->leftChild;
                if (end == head) {
                    end = node;
                    if (stop == node)
                        stop = head;
                    node->leftChild = head->rightChild;
                    if (head->rightChild)
                        head->rightChild->tag = (unsigned int)node | (head->rightChild->tag & 1);
                    head->tag = (node->tag & ~1u) | (head->tag & 1);
                    Node8005DA74* own = owner8005DA74(node);
                    if (node == own->leftChild)
                        own->leftChild = head;
                    else
                        own->rightChild = head;
                    head->rightChild = node;
                    node->tag = (unsigned int)head | (node->tag & 1);
                }
            }

            {
                // Outer (right-right) insertion: recolor, then rotate grandparent left.
                node = owner8005DA74(end);
                node->tag &= ~1u;
                node = owner8005DA74(end);
                node = owner8005DA74(node);
                node->tag |= 1;
                node = owner8005DA74(end);
                other = (Node8005DA74*)(((volatile Node8005DA74*)node)->tag & ~1u);
                own = other->rightChild;
                if (stop == other)
                    stop = own;
                other->rightChild = own->leftChild;
                node = own->leftChild;
                if (node)
                    node->tag = (unsigned int)other | (node->tag & 1);
                own->tag = (other->tag & ~1u) | (own->tag & 1);
                node = owner8005DA74(other);
                if (other == node->leftChild)
                    node->leftChild = own;
                else
                    node->rightChild = own;
                own->leftChild = other;
                other->tag = (unsigned int)own | (other->tag & 1);
            }
        }
    }
    stop->tag &= ~1u;
}

// Promote a's right child; update both root and parent/sentinel links, preserving colors.
// Preconditions: a, a->rightChild and a's parent are non-null; no ownership transfer.
static inline void RotateTreeLeft(Node8005DA74* a, Node8005DA74*& root) {
    Node8005DA74* pivot = a->rightChild;
    if (root == a)
        root = pivot;
    a->rightChild = pivot->leftChild;
    Node8005DA74* transfer = pivot->leftChild;
    if (transfer)
        transfer->tag = (unsigned int)a | (1 & transfer->tag);
    unsigned int color = (pivot->tag & 1);
    pivot->tag = (a->tag & ~1u) | color;
    Node8005DA74* rotParent = owner8005DA74(a);
    if (a == rotParent->leftChild)
        rotParent->leftChild = pivot;
    else
        rotParent->rightChild = pivot;
    pivot->leftChild = a;
    a->tag = (unsigned int)pivot | (a->tag & 1);
}

// Promote a's left child with the same root/sentinel and color contract as RotateTreeLeft.
static inline void RotateTreeRight(Node8005DA74* a, Node8005DA74*& root) {
    Node8005DA74* pivot = a->leftChild;
    if (root == a)
        root = pivot;
    a->leftChild = pivot->rightChild;
    Node8005DA74* transfer = pivot->rightChild;
    if (transfer)
        transfer->tag = (unsigned int)a | (1 & transfer->tag);
    unsigned int color = (pivot->tag & 1);
    pivot->tag = (a->tag & ~1u) | color;
    Node8005DA74* rotParent = owner8005DA74(a);
    if (a == rotParent->leftChild)
        rotParent->leftChild = pivot;
    else
        rotParent->rightChild = pivot;
    pivot->rightChild = a;
    a->tag = (unsigned int)pivot | (a->tag & 1);
}

// Unlink node, repair a removed black node's deficit, and return its pre-erase successor.
// root is the current root; its parent is the sentinel.
// The caller retains responsibility for destroying node; successor may be the sentinel.
Node8005DA74* RBTree_RebalanceAfterErase(Node8005DA74* node, Node8005DA74* root) {
    int left;
    // Phase 1: find the traversal successor before changing any links.
    Node8005DA74* child;
    Node8005DA74* removed;
    child = node->rightChild;
    removed = node;
    // Phase 2: splice a node with at most one child; move the successor into node's
    // place for a two-child erase. fix records whether a black node was removed.
    Node8005DA74* successor;
    if (child) {
        successor = child;
        while (successor->leftChild)
            successor = successor->leftChild;
    } else {
        // 0x8005DDF8: ascend until arriving from a left child.
        while ((successor = owner8005DA74(removed)), removed != successor->leftChild)
            removed = successor;
    }
    /* 0x8005DE40-0x8005DE6C: y = z, or the successor when z has two children; x = y->left, else y->right.
    Ours took x from the node the ascend loop stopped at (an ancestor when z, with only a left child, is a
    right child): the wrong subtree was relinked in place of the erased node (a cycle / freed nodes left in the tree;
    the fx sequence tree walk in CFxRegistryTree::FindInsertionPosition then spun forever after the ball landed). */
    removed = node;
    if (node->leftChild && child)
        removed = successor;
    child = removed->leftChild ? removed->leftChild : removed->rightChild;
    Node8005DA74* parent = owner8005DA74(removed);
    if (child)
        child->tag = (unsigned int)parent | (1 & child->tag);
    Node8005DA74* p = owner8005DA74(removed);

    if (removed == p->leftChild) {
        p->leftChild = child;
        left = 1;
    } else {
        p->rightChild = child;
        left = 0;
    }
    int fix = ((1 & removed->tag)) ^ 1;
    if (removed != node) {
        unsigned int saved = ((volatile Node8005DA74*)removed)->tag;

        unsigned int savedValue = saved;
        removed->tag = (node->tag & ~1u) | ((1 & savedValue));
        Node8005DA74* own = owner8005DA74(removed);
        if (node == own->leftChild)
            own->leftChild = removed;
        else
            own->rightChild = removed;
        Node8005DA74* t = node->leftChild;
        removed->leftChild = t;
        if (t)
            t->tag = (unsigned int)removed | (t->tag & 1);
        else
            parent = removed;
        t = node->rightChild;
        removed->rightChild = t;
        if (t)
            t->tag = (unsigned int)removed | (t->tag & 1);
        else
            parent = removed;
        if (node->tag & 1)
            removed->tag |= 1;
        else
            removed->tag &= ~1u;
        if (root == node)
            root = removed;
    } else if (!child && node == root)
        fix = 0;
    if (fix) {
        // Phase 3: push the black deficit upward, or discharge it with a rotation.
        // left tracks the missing child's side even when child itself is null.
        while (child != root && (!child || !(child->tag & 1))) {
            if (left) {
                Node8005DA74* sibling = parent->rightChild;
                if ((sibling->tag & 1) == 1) {
                    sibling->tag &= ~1u;
                    parent->tag = parent->tag | 1;
                    RotateTreeLeft(parent, root);
                    sibling = parent->rightChild;
                }
                child = sibling->leftChild;
                if ((0 == child || !(child->tag & 1))) {
                    if ((sibling->rightChild == 0 || !(sibling->rightChild->tag & 1))) {
                        sibling->tag = sibling->tag | 1;
                        child = parent;
                        parent = owner8005DA74(parent);
                        left = child == parent->leftChild;
                        continue;
                    }
                }
                if (!sibling->rightChild || !(sibling->rightChild->tag & 1)) {
                    child->tag &= ~1u;
                    sibling->tag |= 1;
                    RotateTreeRight(sibling, root);
                    sibling = parent->rightChild;
                }
                if (1 & parent->tag)
                    sibling->tag |= 1;
                else
                    sibling->tag &= ~1u;
                parent->tag &= ~1u;
                sibling->rightChild->tag &= ~1u;
                RotateTreeLeft(parent, root);
                child = root;
            } else {
                Node8005DA74* sibling = parent->leftChild;
                if ((sibling->tag & 1) == 1) {
                    sibling->tag &= ~1u;
                    parent->tag |= 1;
                    RotateTreeRight(parent, root);
                    sibling = parent->leftChild;
                }
                child = sibling->leftChild;
                if ((child == 0 || !(child->tag & 1))) {
                    if ((!sibling->rightChild || !(sibling->rightChild->tag & 1))) {
                        sibling->tag |= 1;
                        child = parent;
                        parent = owner8005DA74(parent);
                        left = child == parent->leftChild;
                        continue;
                    }
                }
                if (!child || !(child->tag & 1)) {
                    sibling->rightChild->tag &= ~1u;
                    sibling->tag |= 1;
                    RotateTreeLeft(sibling, root);
                    sibling = parent->leftChild;
                }
                if (parent->tag & 1)
                    sibling->tag |= 1;
                else
                    sibling->tag &= ~1u;
                parent->tag &= ~1u;
                sibling->leftChild->tag &= ~1u;
                RotateTreeRight(parent, root);
                child = root;
            }
        }
        if (child)
            child->tag &= ~1u;
    }
    return successor;
}

} // extern "C"
