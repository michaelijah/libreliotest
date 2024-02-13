#include <memory> //unique_ptr, shared_ptr
#include <mutex> //lock_guard, mutex, 

namespace reliotest{
    using namespace std;

    template<typename T>
        class thread_safe_queue
        {
            private:
                struct node
                {
                    shared_ptr<T> data;
                    unique_ptr<node> next;
                };

                mutex head_mutex;
                unique_ptr<node> head;
                mutex tail_mutex;
                node* tail;

                node* get_tail()
                {
                    lock_guard<mutex> tail_lock(tail_mutex);
                    return tail;
                }

                unique_ptr<node> pop_head()
                {
                    unique_ptr<node> old_head = std::move(head);
                    head = std::move(old_head->next);
                    return old_head;
                }

                unique_ptr<node> try_pop_head()
                {
                    lock_guard<mutex> head_lock(head_mutex);
                    if(head.get() == get_tail())
                    {
                        return unique_ptr<node>();
                    }
                    return pop_head();

                };


                unique_ptr<node> try_pop_head(T& value)
                {
                    lock_guard<mutex> head_lock(head_mutex);
                    if(head.get() == get_tail())
                    {
                        return unique_ptr<node>();
                    }
                    value= std::move(*head->data);
                    return pop_head();

                }

            public:
                thread_safe_queue():head(new node), tail(head.get()){};
                thread_safe_queue(const thread_safe_queue& other) =delete;

                shared_ptr<T> try_pop()
                {
                    unique_ptr<node> old_head=try_pop_head();
                    return old_head ? old_head->data : shared_ptr<T>();
                };

                bool try_pop(T& value)
                {
                    unique_ptr<node> const old_head = try_pop_head(value);
                    return old_head != nullptr; //code example expected implicit conv to bool for empty/null check
                };

                void push(T new_value)
                {
                    shared_ptr<T> new_data (
                        make_shared<T>(std::move(new_value))
                        );

                    unique_ptr<node> p(new node);
                    tail->data = new_data;
                    node* const new_tail = p.get();
                    tail->next = std::move(p);
                    tail = new_tail;
                }
                bool empty()
                {
                    lock_guard<mutex> head_lock(head_mutex);
                    return(head.get() == get_tail());
                }



        };
}
