#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;
        int npl; // null path length

        Node(const T& val) : data(val), left(nullptr), right(nullptr), npl(1) {}
    };

    Node* root;
    size_t sz;
    Compare comp;

    // Helper functions
    int getNPL(Node* node) const {
        return node ? node->npl : 0;
    }

    void updateNPL(Node* node) {
        if (node) {
            node->npl = getNPL(node->right) + 1;
        }
    }

    Node* merge(Node* h1, Node* h2) {
        if (!h1) return h2;
        if (!h2) return h1;

        // Ensure h1 has larger priority (smaller value for max heap)
        if (comp(h1->data, h2->data)) {
            std::swap(h1, h2);
        }

        // Merge h2 into right subtree of h1
        h1->right = merge(h1->right, h2);

        // Maintain leftist property
        if (getNPL(h1->left) < getNPL(h1->right)) {
            std::swap(h1->left, h1->right);
        }

        updateNPL(h1);
        return h1;
    }

    void clear(Node* node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

    Node* copy(Node* node) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->data);
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        newNode->npl = node->npl;
        return newNode;
    }

public:
    /**
     * @brief default constructor
     */
    priority_queue() : root(nullptr), sz(0) {}

    /**
     * @brief copy constructor
     * @param other the priority_queue to be copied
     */
    priority_queue(const priority_queue &other) : root(nullptr), sz(0), comp(other.comp) {
        root = copy(other.root);
        sz = other.sz;
    }

    /**
     * @brief deconstructor
     */
    ~priority_queue() {
        clear(root);
    }

    /**
     * @brief Assignment operator
     * @param other the priority_queue to be assigned from
     * @return a reference to this priority_queue after assignment
     */
    priority_queue &operator=(const priority_queue &other) {
        if (this != &other) {
            clear(root);
            root = copy(other.root);
            sz = other.sz;
            comp = other.comp;
        }
        return *this;
    }

    /**
     * @brief get the top element of the priority queue.
     * @return a reference of the top element.
     * @throws container_is_empty if empty() returns true
     */
    const T & top() const {
        if (empty()) {
            throw container_is_empty();
        }
        return root->data;
    }

    /**
     * @brief push new element to the priority queue.
     * @param e the element to be pushed
     */
    void push(const T &e) {
        Node* newNode = new Node(e);
        try {
            root = merge(root, newNode);
            sz++;
        } catch (...) {
            delete newNode;
            throw;
        }
    }

    /**
     * @brief delete the top element from the priority queue.
     * @throws container_is_empty if empty() returns true
     */
    void pop() {
        if (empty()) {
            throw container_is_empty();
        }
        Node* oldRoot = root;
        root = merge(root->left, root->right);
        delete oldRoot;
        sz--;
    }

    /**
     * @brief return the number of elements in the priority queue.
     * @return the number of elements.
     */
    size_t size() const {
        return sz;
    }

    /**
     * @brief check if the container is empty.
     * @return true if it is empty, false otherwise.
     */
    bool empty() const {
        return sz == 0;
    }

    /**
     * @brief merge another priority_queue into this one.
     * The other priority_queue will be cleared after merging.
     * The complexity is at most O(logn).
     * @param other the priority_queue to be merged.
     */
    void merge(priority_queue &other) {
        if (this == &other) return;

        try {
            root = merge(root, other.root);
            sz += other.sz;
            other.root = nullptr;
            other.sz = 0;
        } catch (...) {
            // If merge fails, we need to restore both queues to their original state
            // Since merge is atomic (either succeeds completely or fails completely),
            // and we're using leftist heap properties, this should be handled by
            // the merge function itself. If an exception occurs during comparison,
            // it will propagate up and the caller is responsible for handling it.
            throw;
        }
    }
};

}

#endif