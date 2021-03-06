#pragma once

#include <cstddef>
#include <memory>
#include <stdio.h>
#include "Frame.hpp"
#include "Interface.hpp"
#include "Params.hpp"
#include "../Entry/List.hpp"
#include "../Entry/Callback/Params.hpp"
#include "../Entry/AsyncCallback/Params.hpp"
#include "../Memory/Pool/HasPool.hpp"
#include "../Memory/Stack/Instance.hpp"
#include "../Memory/Stack/Interface.hpp"

namespace BddUnity {
  namespace Depth {

    template <
      size_t maxDepth,
      size_t labelBufferSize,
      long defaultTimeout,
      size_t maxBefore,
      size_t maxAsyncBefore,
      size_t maxAfter,
      size_t maxAsyncAfter,
      size_t maxBeforeEach,
      size_t maxAsyncBeforeEach,
      size_t maxAfterEach,
      size_t maxAsyncAfterEach,
      bool safe
    >
    class Instance : public Interface, public Memory::Pool::HasPool<Interface, Params> {
      
      public:

        Instance(Memory::Pool::Interface<Interface, Params> * pool, const Params & params) :
          Memory::Pool::HasPool<Interface, Params>(pool)
        {}

        const Error * free() override {
          return Memory::Pool::HasPool<Interface, Params>::free();
        }

        const Error * push(const Frame & frame) override {
          return _frameStack.push(frame);
        }

        const Error * pop() override {
          const Error * ret = nullptr;
          const Error * e;
          const Frame & frame = *(_frameStack.current());
          e = _popEntryParams(frame.before, _beforeStack, _asyncBeforeStack);
          if (e) ret = e;
          e = _popEntryParams(frame.after, _afterStack, _asyncAfterStack);
          if (e) ret = e;
          e = _popEntryParams(frame.beforeEach, _beforeEachStack, _asyncBeforeEachStack);
          if (e) ret = e;
          e = _popEntryParams(frame.afterEach, _afterEachStack, _asyncAfterEachStack);
          if (e) ret = e;
          e = _frameStack.pop();
          if (e) ret = e;
          return ret;
        }

        const char * getLabel(const char * field) override {
          _resetLabel();
          _concatenateLabel(field);
          return _currentLabel;
        }

        const char * getLabel(const char * field1, const char * field2) override {
          _resetLabel();
          _concatenateLabel(field1);
          _concatenateLabel(field2);
          return _currentLabel;
        }

        const Error * setBefore(const Entry::Callback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.before != Frame::CallbackType::NONE) {
            setError(Error::Code::BEFORE_SET);
            return &error;
          }
          frame.before = Frame::CallbackType::SYNC;
          return _beforeStack.push(params);
        }

        const Error * setBefore(const Entry::AsyncCallback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.before != Frame::CallbackType::NONE) {
            setError(Error::Code::BEFORE_SET);
            return &error;
          }
          frame.before = Frame::CallbackType::ASYNC;
          return _asyncBeforeStack.push(params);
        }

        const Error * before(
          Entry::List & list,
          Memory::Pool::Interface<Entry::Interface, Entry::Callback::Params> & syncPool,
          Memory::Pool::Interface<Entry::Interface, Entry::AsyncCallback::Params> & asyncPool
        ) override {
          size_t syncIndex = _beforeStack.currentIndex();
          size_t asyncIndex = _asyncBeforeStack.currentIndex();
          const Frame & frame = *(_frameStack.current());
          return _createEntry(
            frame.before,
            list,
            true,
            syncPool,
            asyncPool,
            _beforeStack,
            _asyncBeforeStack,
            syncIndex,
            asyncIndex
          );
        }

        const Error * setAfter(const Entry::Callback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.after != Frame::CallbackType::NONE) {
            setError(Error::Code::AFTER_SET);
            return &error;
          }
          frame.after = Frame::CallbackType::SYNC;
          return _afterStack.push(params);
        }

        const Error * setAfter(const Entry::AsyncCallback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.after != Frame::CallbackType::NONE) {
            setError(Error::Code::AFTER_SET);
            return &error;
          }
          frame.after = Frame::CallbackType::ASYNC;
          return _asyncAfterStack.push(params);
        }

        const Error * after(
          Entry::List & list,
          Memory::Pool::Interface<Entry::Interface, Entry::Callback::Params> & syncPool,
          Memory::Pool::Interface<Entry::Interface, Entry::AsyncCallback::Params> & asyncPool
        ) override {
          size_t syncIndex = _afterStack.currentIndex();
          size_t asyncIndex = _asyncAfterStack.currentIndex();
          const Frame & frame = *(_frameStack.current());
          return _createEntry(
            frame.after,
            list,
            false,
            syncPool,
            asyncPool,
            _afterStack,
            _asyncAfterStack,
            syncIndex,
            asyncIndex
          );
        }

        const Error * setBeforeEach(const Entry::Callback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.beforeEach != Frame::CallbackType::NONE) {
            setError(Error::Code::BEFORE_EACH_SET);
            return &error;
          }
          frame.beforeEach = Frame::CallbackType::SYNC;
          return _beforeEachStack.push(params);
        }

        const Error * setBeforeEach(const Entry::AsyncCallback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.beforeEach != Frame::CallbackType::NONE) {
            setError(Error::Code::BEFORE_EACH_SET);
            return &error;
          }
          frame.beforeEach = Frame::CallbackType::ASYNC;
          return _asyncBeforeEachStack.push(params);
        }

        const Error * beforeEach(
          Entry::List & list,
          Memory::Pool::Interface<Entry::Interface, Entry::Callback::Params> & syncPool,
          Memory::Pool::Interface<Entry::Interface, Entry::AsyncCallback::Params> & asyncPool
        ) override {
          size_t syncIndex = 0;
          size_t asyncIndex = 0;
          return _frameStack.forEach([&](const Frame & frame) {
            return _createEntry(
              frame.beforeEach,
              list,
              true,
              syncPool,
              asyncPool,
              _beforeEachStack,
              _asyncBeforeEachStack,
              syncIndex,
              asyncIndex
            );
          });
        }

        const Error * setAfterEach(const Entry::Callback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.afterEach != Frame::CallbackType::NONE) {
            setError(Error::Code::AFTER_EACH_SET);
            return &error;
          }
          frame.afterEach = Frame::CallbackType::SYNC;
          return _afterEachStack.push(params);
        }

        const Error * setAfterEach(const Entry::AsyncCallback::Params & params) override {
          Frame & frame = *(_frameStack.current());
          if (frame.afterEach != Frame::CallbackType::NONE) {
            setError(Error::Code::AFTER_EACH_SET);
            return &error;
          }
          frame.afterEach = Frame::CallbackType::ASYNC;
          return _asyncAfterEachStack.push(params);
        }

        const Error * afterEach(
          Entry::List & list,
          Memory::Pool::Interface<Entry::Interface, Entry::Callback::Params> & syncPool,
          Memory::Pool::Interface<Entry::Interface, Entry::AsyncCallback::Params> & asyncPool
        ) override {
          size_t syncIndex = 0;
          size_t asyncIndex = 0;
          return _frameStack.forEach([&](const Frame & frame) {
            return _createEntry(
              frame.afterEach,
              list,
              false,
              syncPool,
              asyncPool,
              _afterEachStack,
              _asyncAfterEachStack,
              syncIndex,
              asyncIndex
            );
          });
        }

        const long getTimeout(const long timeout) override {
          if (0 != timeout) {
            // timeout has been overriden
            return timeout;
          }
          long ret = defaultTimeout;
          _frameStack.forEach([&](const Frame & frame) {
            // only apply next timeout if it is an override
            const long t = frame.timeout;
            if (0 != t) {
              ret = t;
            }
            return nullptr;
          });
          return ret;
        }

        const size_t getMaxDepth() override {
          return maxDepth;
        }

      private:
        
        Memory::Stack::Instance<
          Frame,
          Frame,
          Frame,
          maxDepth,
          safe
        > _frameStack;

        Memory::Stack::Instance<
          Entry::Callback::Params,
          Entry::Callback::Params,
          Entry::Callback::Params,
          maxBefore,
          safe
        > _beforeStack;

        Memory::Stack::Instance<
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          maxAsyncBefore,
          safe
        > _asyncBeforeStack;

        Memory::Stack::Instance<
          Entry::Callback::Params,
          Entry::Callback::Params,
          Entry::Callback::Params,
          maxAfter,
          safe
        > _afterStack;

        Memory::Stack::Instance<
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          maxAsyncAfter,
          safe
        > _asyncAfterStack;

        Memory::Stack::Instance<
          Entry::Callback::Params,
          Entry::Callback::Params,
          Entry::Callback::Params,
          maxBeforeEach,
          safe
        > _beforeEachStack;

        Memory::Stack::Instance<
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          maxAsyncBeforeEach,
          safe
        > _asyncBeforeEachStack;

        Memory::Stack::Instance<
          Entry::Callback::Params,
          Entry::Callback::Params,
          Entry::Callback::Params,
          maxAfterEach,
          safe
        > _afterEachStack;

        Memory::Stack::Instance<
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          Entry::AsyncCallback::Params,
          maxAsyncAfterEach,
          safe
        > _asyncAfterEachStack;

        // we use 2 buffers to make concatenation simpler
        char _label1[labelBufferSize];
        char _label2[labelBufferSize];
        char * _currentLabel;

        void _resetLabel() {
          _currentLabel = _label1;
          _currentLabel[0] = 0;

          _frameStack.forEach([&](const Frame & frame) {
            _concatenateLabel(frame.label);
            return nullptr;
          });
        }

        void  _concatenateLabel(const char * field) {
          char * destination = _currentLabel == _label1 ? _label2 : _label1;
          snprintf(destination, labelBufferSize, "%s[%s]", _currentLabel, field);
          _currentLabel = destination;
        }

        const Error * _free() override {
          return nullptr;
        }

        static const Error * _popEntryParams(
          const Frame::CallbackType type,
          Memory::Stack::Interface<Entry::Callback::Params, Entry::Callback::Params> & syncStack,
          Memory::Stack::Interface<Entry::AsyncCallback::Params, Entry::AsyncCallback::Params> & asyncStack
        ) {
          switch (type) {
            case Frame::CallbackType::NONE:
              // do nothing
              return nullptr;
            case Frame::CallbackType::SYNC:
              return syncStack.pop();
            case Frame::CallbackType::ASYNC:
              return asyncStack.pop();
          }
          return nullptr;
        }

        static const Error * _createEntry(
          const Frame::CallbackType type,
          Entry::List & list,
          const bool append,
          Memory::Pool::Interface<Entry::Interface, Entry::Callback::Params> & syncPool,
          Memory::Pool::Interface<Entry::Interface, Entry::AsyncCallback::Params> & asyncPool,
          Memory::Stack::Interface<Entry::Callback::Params, Entry::Callback::Params> & syncStack,
          Memory::Stack::Interface<Entry::AsyncCallback::Params, Entry::AsyncCallback::Params> & asyncStack,
          size_t & syncIndex,
          size_t & asyncIndex
        ) {
          Entry::Interface * entry = nullptr;
          switch (type) {
            case Frame::CallbackType::NONE:
              // do nothing
              return nullptr;
            case Frame::CallbackType::SYNC:
              {
                entry = syncPool.create(*(syncStack.get(syncIndex++)));
                if (!entry) {
                  return &(syncPool.error);
                }
              }
              break;
            case Frame::CallbackType::ASYNC:
              {
                entry = asyncPool.create(*(asyncStack.get(asyncIndex++)));
                if (!entry) {
                  return &(asyncPool.error);
                }
              }
              break;
          }
          if (append) {
            list.append(entry);
          } else {
            list.prepend(entry);
          }
          return nullptr;
        }
    };

  }
}
