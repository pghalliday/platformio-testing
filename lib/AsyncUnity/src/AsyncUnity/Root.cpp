#include "./Root.hpp"

#include <unity.h>
#include "./Globals.hpp"
#include "./platform.hpp"
#include "Globals/TestParams.hpp"
#include "Globals/Timeout.hpp"

namespace AsyncUnity {

  Root::Root(
    const char * file,
    const char * thing,
    const int line,
    Entry::Describe::f_describe cb,
    long timeout
  ) :
    _file(file),
    _entries(Entry::Describe::create(thing, line, cb, timeout))
  {}

  void Root::setup() {
    UnityBegin(_file);
  }

  void Root::loop() {
    switch (_state) {
      case State::NEXT:
        {
          Entry::Entry * entry = _entries;
          if (entry) {
            _state = State::WAITING;
            _started = millis();
            entry->run([&](const Error * error = nullptr) {
              if (!entry->timedOut) {
                _state = State::DONE;
                error = error;
              }
            });
          } else {
            _end();
          }
          break;
        }
      case State::WAITING:
        {
          // check timeout
          if (_timeout != Timeout::NO_TIMEOUT) {
            if (millis() - _started > _timeout) {
              _entries->timedOut = true;
              _state = State::DONE;
              error = &Globals::timeoutError;
              const char * label = Globals::depth.getLabel(Globals::timeout.label);
              Globals::testLine = Globals::timeout.line;
              Globals::testMessage = error->c_str();
              UnityDefaultTestRun([]() {
                UNITY_TEST_FAIL(Globals::testLine, Globals::testMessage);
              }, label, Globals::timeout.line);
            }
          }
          break;
        }
      case State::DONE:
        {
          // Check for errors and update _entries
          // appropriately
          if (error) {
            // Clean everything up
            while (_entries) {
              _entries->free();
              _entries = _entries->next;
            }
          } else {
            // all fine line up the next entry and free
            // the previous one
            Entry::Entry * previous = _entries;
            _entries = _entries->next;
            previous->free();
          }
          _state = State::NEXT;
          break;
        }
      case State::FINISHED:
        {
          // do nothing
          break;
        }
    }
  }

  bool Root::isRunning() {
    return (_state != State::FINISHED);
  }

  void Root::_end() {
    status = UnityEnd();
    _state = State::FINISHED;
  }

}
