#pragma once

namespace silk_data {
	typedef enum Ñolor {
		RED = 0,
		BLACK = 1
	} Ñolor;

	template<typename T>
	class ICompareStrategy {
	public:
		virtual bool Equals(T& a, T& b) = 0;
		virtual bool Less(T& a, T& b) = 0;
	};
	template<typename T>
	class DefaultCompareStrategy : public ICompareStrategy<T> {
	public:
		bool Equals(T& a, T& b) {
			return a == b;
		}
		bool Less(T& a, T& b) {
			return a < b;
		}
	};


	template<typename K, typename V>
	struct Node {
		Node() {
			payload = nullptr;
			color = BLACK;
		}
		Node(K& _key, V* _payload) {
			key = _key;
			payload = _payload;
			color = BLACK;
		}
		~Node() { }

		char color;
		K key;
		V* payload;
		Node<K, V>* parent;
		Node<K, V>* left;
		Node<K, V>* right;
	};

	template <typename K, typename V>
	class RBTree {
	public:
		RBTree(ICompareStrategy<K>* _comparator = new DefaultCompareStrategy<K>) {
			nill = new Node<K, V>();
			nill->parent = nill;
			nill->left = nill;
			nill->right = nill;
			nill->color = BLACK;
			root = nill;
			comparator = _comparator;
		}
		~RBTree() {
			Clear();
			delete nill;
			delete comparator;
		}
		Node<K, V>* MakeNode(K& key, V* payload) {
			auto node = new Node<K, V>(key, payload);
			node->color = BLACK;
			node->parent = nill;
			node->left = nill;
			node->right = nill;
			return node;
		}
		Node<K, V>* Find(K& key) {
			auto node = root;
			while (node != nill) {
				if (comparator->Equals(node->key, key))
					return node;
				else if (comparator->Less(key, node->key))
					node = node->left;
				else
					node = node->right;
			}
			return nullptr;
		}
		void Clear() {
			VisitClearing(root);
		}
		void ToArray(Node<K, V>** array) {
			int counter = 0;
			Visit(root, array, &counter);
		}
		void Insert(Node<K, V>& z) {
			if (Find(z.key))
				return;
			auto y = FindParent(z.key);
			z.parent = y;
			if (y == nill)
				root = &z;
			else if (comparator->Less(z.key, y->key))
				y->left = &z;
			else
				y->right = &z;
			z.left = nill;
			z.right = nill;
			z.color = RED;

			InsertFixup(&z);
		}
		void Remove(Node<K, V>& z) {
			auto y = &z;
			char origColor = y->color;
			auto x = nill;
			if (z.left == nill) {
				x = z.right;
				Transplant(&z, z.right);
			}
			else if (z.right == nill) {
				x = z.left;
				Transplant(&z, z.left);
			}
			else {
				y = TreeMinimum(z.right);
				origColor = y->color;
				x = y->right;
				if (y->parent == &z)
					x->parent = y;
				else {
					Transplant(y, y->right);
					y->right = z.right;
					y->right->parent = y;
				}
				Transplant(&z, y);
				y->left = z.left;
				y->left->parent = y;
				y->color = z.color;
			}
			if (origColor == BLACK)
				RemoveFixup(x);
			delete &z;
		}
		Node<K, V>* TreeMinimum(Node<K, V>* x) {
			while (x->left != nill)
				x = x->left;
			return x;
		}
		Node<K, V>* TreeMaximum(Node<K, V>* x) {
			while (x->right != nill)
				x = x->right;
			return x;
		}
	private:
		Node<K, V>* FindParent(K& key) {
			auto y = nill;
			auto x = root;
			while (x != nill) {
				y = x;
				if (comparator->Less(key, x->key))
					x = x->left;
				else
					x = x->right;
			}
			return y;
		}
		void InsertFixup(Node<K, V>* z) {
			while (z->parent->color == RED) {
				if (z->parent == z->parent->parent->left) {
					auto y = z->parent->parent->right;
					if (y->color == RED) {
						z->parent->color = BLACK;
						y->color = BLACK;
						z->parent->parent->color = RED;
						z = z->parent->parent;
					}
					else {
						if (z == z->parent->right) {
							z = z->parent;
							LeftRotate(*z);
						}
						z->parent->color = BLACK;
						z->parent->parent->color = RED;
						RightRotate(*z->parent->parent);
					}
				}
				else {
					auto y = z->parent->parent->left;
					if (y->color == RED) {
						z->parent->color = BLACK;
						y->color = BLACK;
						z->parent->parent->color = RED;
						z = z->parent->parent;
					}
					else {
						if (z == z->parent->left) {
							z = z->parent;
							RightRotate(*z);
						}
						z->parent->color = BLACK;
						z->parent->parent->color = RED;
						LeftRotate(*z->parent->parent);
					}
				}
			}
			root->color = BLACK;
		}
		void RemoveFixup(Node<K, V>* x) {
			while (x != root && x->color == BLACK) {
				if (x == x->parent->left) {
					auto w = x->parent->right;
					if (w->color == RED) {
						w->color = BLACK;
						x->parent->color = RED;
						LeftRotate(*x->parent);
						w = x->parent->right;
					}
					if (w->left->color == BLACK && w->right->color == BLACK) {
						w->color = RED;
						x = x->parent;
					}
					else {
						if (w->right->color == BLACK) {
							w->left->color = BLACK;
							w->color = RED;
							RightRotate(*w);
							w = x->parent->right;
						}
						w->color = x->parent->color;
						x->parent->color = BLACK;
						w->right->color = BLACK;
						LeftRotate(*x->parent);
						x = root;
					}
				}
				else {
					auto w = x->parent->left;
					if (w->color == RED) {
						w->color = BLACK;
						x->parent->color = RED;
						RightRotate(*x->parent);
						w = x->parent->left;
					}
					if (w->right->color == BLACK && w->left->color == BLACK) {
						w->color = RED;
						x = x->parent;
					}
					else {
						if (w->left->color == BLACK) {
							w->right->color = BLACK;
							w->color = RED;
							LeftRotate(*w);
							w = x->parent->left;
						}
						w->color = x->parent->color;
						x->parent->color = BLACK;
						w->left->color = BLACK;
						RightRotate(*x->parent);
						x = root;
					}
				}
			}
			x->color = BLACK;
		}
		void LeftRotate(Node<K, V>& x) {
			auto y = x.right;
			x.right = y->left;
			if (y->left != nill)
				y->left->parent = &x;
			y->parent = x.parent;
			if (x.parent == nill)
				root = y;
			else if (&x == x.parent->left)
				x.parent->left = y;
			else
				x.parent->right = y;
			y->left = &x;
			x.parent = y;
		}
		void RightRotate(Node<K, V>& y) {
			auto x = y.left;
			y.left = x->right;
			if (x->right != nill)
				x->right->parent = &y;
			x->parent = y.parent;
			if (y.parent == nill)
				root = x;
			else if (&y == y.parent->right)
				y.parent->right = x;
			else
				y.parent->left = x;
			x->right = &y;
			y.parent = x;
		}
		void Transplant(Node<K, V>* u, Node<K, V>* v) {
			if (u->parent == nill)
				root = v;
			else if (u == u->parent->left)
				u->parent->left = v;
			else
				u->parent->right = v;
			v->parent = u->parent;
		}
		void VisitClearing(Node<K, V>* node) {
			if (node->left != nill)
				VisitClearing(node->left);
			if (node->right != nill)
				VisitClearing(node->right);
			delete node;
		}
		void Visit(Node<K, V>* node, Node<K, V>** array, int* i) {
			array[(*i)++] = node;
			if (node->left != nill)
				Visit(node->left, array, i);
			if (node->right != nill)
				Visit(node->right, array, i);
		}
	private:
		Node<K, V>* root;
		Node<K, V>* nill;
		ICompareStrategy<K>* comparator;
	};
}