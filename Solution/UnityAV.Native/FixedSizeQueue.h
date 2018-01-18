#pragma once

#include "stdafx.h"

using namespace std;

namespace UnityAV
{
    /**
     * \brief Responsible for the implementation of a thread safe fixed size queue
     * \tparam T The class type to store in the queue
     */
    template <class T> class FixedSizeQueue
    {
    public:
        /**
        * \brief Initializes a new instance of FixedSizeQueue
        * \param maxSize The maximum number of instances to hold at a time
        */
        explicit FixedSizeQueue<T>(int maxSize) : _maxSize(maxSize)
        {
            if (maxSize < 1)
            {
                throw exception("FixedSizeQueue::FixedSizeQueue maxSize must > 1");
            }
        }
        
        // Move constructor
        explicit FixedSizeQueue<T>(FixedSizeQueue<T>&& other) noexcept 
            : _queue(move(other._queue)), _maxSize(other._maxSize)
        {

        }

        // Move assignment
        FixedSizeQueue<T>& operator=(FixedSizeQueue<T>&& other) noexcept
        {
            _queue = move(other._queue);
            _maxSize = other._maxSize;

            return *this;
        }

        // Disabled copy constructor
        explicit FixedSizeQueue<T>(const FixedSizeQueue<T>& other) = delete;
        // Disabled move assignment
        FixedSizeQueue<T>& operator=(const FixedSizeQueue<T>& other) = delete;

        /**
        * \brief Evaluates the current element count
        * \return Returns the current element count
        */
        size_t Count() const
        {
            return _queue.size();
        }

        /**
         * \brief Evaluates if the queue is full
         * \return Returns true if the queue is full
         */
        bool Full() const
        {
            return Count() == static_cast<size_t>(_maxSize);
        }

        /**
        * \brief Evaluates if the queue is empty
        * \return Returns true if the queue is empty
        */
        bool Empty() const
        {
            return Count() == static_cast<size_t>(0);
        }
        
        /**
         * \brief Flushes the queue to become empty;
         */
        void Flush()
        {
            // lock the mutex
            lock_guard<mutex> lock(_mutex);

            // swap the queue with an empty queue
            queue<T> empty;
            swap(_queue, empty);
        }

        /**
        * \brief Pops an instance off the top of the queue
        * \return The instance from the top of the queue
        */
        T Pop()
        {
            // lock the mutex
            lock_guard<mutex> lock(_mutex);

            if(Empty())
            {
                return nullptr;
            }

            // get the value first then pop
            auto target = move(_queue.front());
            _queue.pop();

            return target;
        }

        /**
        * \brief Pushes a instance onto the queue
        * \param instance The instance to push onto the queue, will take ownership
        * \return Was the instance successfully pushed onto the queue?
        */
        bool Push(T instance)
        {
            // get rid of the oldest entry if we've reached the max size
            if (_queue.size() >= _maxSize)
            {
                _queue.pop();
            }

            // lock the mutex
            lock_guard<mutex> lock(_mutex);
            // push down the instance
            _queue.push(move(instance));

            return true;
        }

    private:
        queue<T> _queue;
        mutex _mutex;
        int _maxSize;
    };
}
