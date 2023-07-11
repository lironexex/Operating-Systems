#ifndef THREAD_SAFE_LIST_H_
#define THREAD_SAFE_LIST_H_

#include <pthread.h>
#include <iostream>
#include <iomanip> // std::setw

using namespace std;

template <typename T>
class List 
{
    Node* head;
    size_t size;
    public:
        /**
         * Constructor
         */
        List()
        : head(NULL), size(0) {}
}

        /**
         * Destructor
         */
        ~List(){
            Node* next_iterator = this->head, *current_iterator;
            while(next_iterator != NULL){
                current_iterator = next_iterator;
                next_iterator = current_iterator->next;
                delete current_iterator;
            }
        }

        class Node {
            pthread_mutex node_lock;
         public:
            T data;
            Node* next;
            Node(const T& data, Node* const next) = default;
            ~Node() = default;

            void lock(){
                pthread_mutex_lock(&node_lock);
            }
            void unlock(){
                pthread_mutex_unlock(&node_lock);
            }

        };

        /**
         * Insert new node to list while keeping the list ordered in an ascending order
         * If there is already a node has the same data as @param data then return false (without adding it again)
         * @param data the new data to be added to the list
         * @return true if a new node was added and false otherwise
         */
        bool insert(const T& data) {
			//iterate to the right place for the new node
            Node* next_iterator = this->head, *current_iterator = NULL;

            while(next_iterator != NULL and next_iterator->data < data){
                next_iterator.lock();
                current_iterator = next_iterator;
                next_iterator = current_iterator->next;
                current_iterator->unlock();
            }
            next_iterator.unlock();

            //if the key was found in the list
            if(next_iterator != NULL and next_iterator->data == data)
                return false;

            //insertion
            Node new_node = new Node(data, next_iterator);
            if(current_iterator == NULL){
                //if the right place is the head
                this->head = &new_node;
            } else {
                current_iterator->next = &new_node;
            }
            this->size++;
            return true;
        }

        /**
         * Remove the node that its data equals to @param value
         * @param value the data to lookup a node that has the same data to be removed
         * @return true if a matched node was found and removed and false otherwise
         */
        bool remove(const T& value) {
            //find
            Node* next_iterator = this->head, *current_iterator = NULL;
            while(next_iterator != NULL and next_iterator->data < data){
                current_iterator = next_iterator;
                next_iterator = current_iterator->next;
            }
            //if not found
            if(next_iterator == NULL or next_iterator->data != data)
                return false;

            //if the head is to be removed
            if(current_iterator == NULL){
                delete this->head;
                this->head = this->head->next;
            } else {
                current_iterator->next = next_iterator->next;
                delete next_iterator;
            }
            this->size--;
            return true;
        }

        /**
         * Returns the current size of the list
         * @return the list size
         */
        unsigned int getSize() {
			return this->size;
        }

		// Don't remove
        void print() {
          pthread_mutex_lock(&list_mutex);
          Node* temp = head;
          if (temp == NULL)
          {
            cout << "";
          }
          else if (temp->next == NULL)
          {
            cout << temp->data;
          }
          else
          {
            while (temp != NULL)
            {
              cout << right << setw(3) << temp->data;
              temp = temp->next;
              cout << " ";
            }
          }
          cout << endl;
          pthread_mutex_unlock(&list_mutex);
        }

		// Don't remove
        virtual void __add_hook() {}
		// Don't remove
        virtual void __remove_hook() {}

    private:
        Node* head;
    // TODO: Add your own methods and data members
};

#endif //THREAD_SAFE_LIST_H_