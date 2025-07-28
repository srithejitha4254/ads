#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ORDER 5  // B-Tree order

typedef struct BTreeNode {
    int keys[ORDER - 1];
    struct BTreeNode* children[ORDER];
    int numKeys;
    int isLeaf;
} BTreeNode;

BTreeNode* createNode(int isLeaf) {
    int i;
    BTreeNode* newNode = (BTreeNode*)malloc(sizeof(BTreeNode));
    newNode->isLeaf = isLeaf;
    newNode->numKeys = 0;
    for (i = 0; i < ORDER; i++) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

void splitChild(BTreeNode* parent, int index, BTreeNode* child) {
    int i;
    BTreeNode* newChild = createNode(child->isLeaf);
    newChild->numKeys = ORDER / 2 - 1;

    // Copy the last (ORDER/2 - 1) keys of child to newChild
    for (i = 0; i < ORDER / 2 - 1; i++) {
        newChild->keys[i] = child->keys[i + ORDER / 2];
    }

    // Copy the last ORDER/2 children if not leaf
    if (!child->isLeaf) {
        for (i = 0; i < ORDER / 2; i++) {
            newChild->children[i] = child->children[i + ORDER / 2];
        }
    }

    child->numKeys = ORDER / 2 - 1;

    // Shift children of parent to make room for newChild
    for (i = parent->numKeys; i >= index + 1; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = newChild;

    // Shift keys of parent to make room for median key
    for (i = parent->numKeys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }

    // Move median key from child to parent
    parent->keys[index] = child->keys[ORDER / 2 - 1];
    parent->numKeys++;
}

void insertNonFull(BTreeNode* node, int key) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        // Insert key in this leaf node at correct position
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->numKeys++;
    } else {
        // Find the child which should have the new key
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        // If the found child is full, split it
        if (node->children[i]->numKeys == ORDER - 1) {
            splitChild(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insertNonFull(node->children[i], key);
    }
}

void insert(BTreeNode** root, int key) {
    BTreeNode* r = *root;
    if (r->numKeys == ORDER - 1) {
        BTreeNode* s = createNode(0);
        *root = s;
        s->children[0] = r;
        splitChild(s, 0, r);
        insertNonFull(s, key);
    } else {
        insertNonFull(r, key);
    }
}

int search(BTreeNode* root, int key) {
    int i = 0;
    while (i < root->numKeys && key > root->keys[i]) {
        i++;
    }
    if (i < root->numKeys && root->keys[i] == key) {
        return 1;  // Found
    }
    if (root->isLeaf) {
        return 0;  // Not found
    }
    return search(root->children[i], key);
}

// Forward declarations for deletion helper functions
void deleteKey(BTreeNode* node, int key);
void fill(BTreeNode* node, int idx);
void borrowFromPrev(BTreeNode* node, int idx);
void borrowFromNext(BTreeNode* node, int idx);
void merge(BTreeNode* node, int idx);

void removeFromLeaf(BTreeNode* node, int idx) {
    int i;
    for (i = idx + 1; i < node->numKeys; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    node->numKeys--;
}

void removeFromNonLeaf(BTreeNode* node, int idx) {
    int i, key = node->keys[idx];

    // If the child before key (idx) has at least ORDER/2 keys
    if (node->children[idx]->numKeys >= ORDER / 2) {
        BTreeNode* pred = node->children[idx];
        while (!pred->isLeaf) {
            pred = pred->children[pred->numKeys];
        }
        int predKey = pred->keys[pred->numKeys - 1];
        node->keys[idx] = predKey;
        deleteKey(node->children[idx], predKey);
    }
    // If the child after key (idx+1) has at least ORDER/2 keys
    else if (node->children[idx + 1]->numKeys >= ORDER / 2) {
        BTreeNode* succ = node->children[idx + 1];
        while (!succ->isLeaf) {
            succ = succ->children[0];
        }
        int succKey = succ->keys[0];
        node->keys[idx] = succKey;
        deleteKey(node->children[idx + 1], succKey);
    }
    // Otherwise, merge children and recurse
    else {
        BTreeNode* left = node->children[idx];
        BTreeNode* right = node->children[idx + 1];

        left->keys[left->numKeys] = key;
        for (i = 0; i < right->numKeys; i++) {
            left->keys[left->numKeys + 1 + i] = right->keys[i];
        }

        if (!left->isLeaf) {
            for (i = 0; i <= right->numKeys; i++) {
                left->children[left->numKeys + 1 + i] = right->children[i];
            }
        }

        left->numKeys += right->numKeys + 1;

        // Shift keys and children in node to remove right child
        for (i = idx + 1; i < node->numKeys; i++) {
            node->keys[i - 1] = node->keys[i];
        }
        for (i = idx + 2; i <= node->numKeys; i++) {
            node->children[i - 1] = node->children[i];
        }
        node->numKeys--;

        free(right);

        deleteKey(left, key);
    }
}

// Borrow a key from the (idx-1)th child and place it in idx-th child
void borrowFromPrev(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];

    // Shift keys and children in child to right to make space
    int i;
    for (i = child->numKeys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }
    if (!child->isLeaf) {
        for (i = child->numKeys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    // Move key from parent to child
    child->keys[0] = node->keys[idx - 1];

    // Move sibling's last child to child's first child if not leaf
    if (!child->isLeaf) {
        child->children[0] = sibling->children[sibling->numKeys];
    }

    // Move sibling's last key up to parent
    node->keys[idx - 1] = sibling->keys[sibling->numKeys - 1];

    child->numKeys += 1;
    sibling->numKeys -= 1;
}

// Borrow a key from the (idx+1)th child and place it in idx-th child
void borrowFromNext(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    // Move key from parent to child's last key
    child->keys[child->numKeys] = node->keys[idx];

    // Move sibling's first child to child's last child if not leaf
    if (!child->isLeaf) {
        child->children[child->numKeys + 1] = sibling->children[0];
    }

    // Move sibling's first key up to parent
    node->keys[idx] = sibling->keys[0];

    // Shift keys and children in sibling to left
    int i;
    for (i = 1; i < sibling->numKeys; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }
    if (!sibling->isLeaf) {
        for (i = 1; i <= sibling->numKeys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->numKeys += 1;
    sibling->numKeys -= 1;
}

// Merge idx-th child and (idx+1)-th child of node
void merge(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    int i;

    // Pull down key from node into child
    child->keys[ORDER / 2 - 1] = node->keys[idx];

    // Copy sibling's keys and children to child
    for (i = 0; i < sibling->numKeys; i++) {
        child->keys[i + ORDER / 2] = sibling->keys[i];
    }
    if (!child->isLeaf) {
        for (i = 0; i <= sibling->numKeys; i++) {
            child->children[i + ORDER / 2] = sibling->children[i];
        }
    }

    child->numKeys += sibling->numKeys + 1;

    // Shift keys and children in node to remove sibling
    for (i = idx + 1; i < node->numKeys; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    for (i = idx + 2; i <= node->numKeys; i++) {
        node->children[i - 1] = node->children[i];
    }

    node->numKeys--;

    free(sibling);
}

// Ensure child has at least ORDER/2 keys before descent
void fill(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->numKeys >= ORDER / 2) {
        borrowFromPrev(node, idx);
    } else if (idx != node->numKeys && node->children[idx + 1]->numKeys >= ORDER / 2) {
        borrowFromNext(node, idx);
    } else {
        if (idx != node->numKeys) {
            merge(node, idx);
        } else {
            merge(node, idx - 1);
        }
    }
}

void deleteKey(BTreeNode* node, int key) {
    int idx = 0;
    while (idx < node->numKeys && node->keys[idx] < key) {
        idx++;
    }

    if (idx < node->numKeys && node->keys[idx] == key) {
        // Key found in this node
        if (node->isLeaf) {
            removeFromLeaf(node, idx);
        } else {
            removeFromNonLeaf(node, idx);
        }
    } else {
        if (node->isLeaf) {
            printf("The key %d is not found in the tree.\n", key);
            return;
        }

        // Flag to check if key is present in last child
        int flag = (idx == node->numKeys) ? 1 : 0;

        // If child where key may exist has less than ORDER/2 keys, fix it
        if (node->children[idx]->numKeys < ORDER / 2) {
            fill(node, idx);
        }

        // After fill, recurse into correct child (note merge might have changed structure)
        if (flag && idx > node->numKeys) {
            deleteKey(node->children[idx - 1], key);
        } else {
            deleteKey(node->children[idx], key);
        }
    }
}

void printBTree(BTreeNode* root, int level) {
    int i;
    if (root != NULL) {
        printf("Level %d: ", level);
        for (i = 0; i < root->numKeys; i++) {
            printf("%d ", root->keys[i]);
        }
        printf("\n");
        for (i = 0; i <= root->numKeys; i++) {
            printBTree(root->children[i], level + 1);
        }
    }
}

int main() {
    int i;
    srand(time(0));
    BTreeNode* root = createNode(1);
    int arr[100];

    // Generate 100 random elements and insert into B-tree
    for (i = 0; i < 100; i++) {
        arr[i] = rand() % 1000;
        insert(&root, arr[i]);
    }

    printf("B-Tree structure after inserting 100 random elements:\n");
    printBTree(root, 0);

    // Searching for a random key
    int searchKey = arr[rand() % 100];
    printf("\nSearching for key %d: %s\n", searchKey, search(root, searchKey) ? "Found" : "Not Found");

    // Deleting a key
    int deleteKeyVal = arr[rand() % 100];
    printf("Deleting key %d...\n", deleteKeyVal);
    deleteKey(root, deleteKeyVal);

    printf("B-Tree structure after deleting key %d:\n", deleteKeyVal);
    printBTree(root, 0);

    return 0;
}

