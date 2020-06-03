#include <AsyncUnity.hpp>
#include <MyLib.hpp>

// ASYNC_UNITY_DESCRIBE("my thing", {
  // ASYNC_UNITY_IT("should fail", {
    // TEST_ASSERT_TRUE(false);
  // });

  // ASYNC_UNITY_IT("should pass", {
    // TEST_ASSERT_TRUE(true);
  // });

  // ASYNC_UNITY_IT("should fail", {
    // TEST_ASSERT_TRUE(false);
  // });
// });

AsyncUnity::Root asyncUnityRoot(__FILE__, 256, "my module", [](AsyncUnity::Describe * asyncUnityEntry) {
  asyncUnityEntry->describe("my thing", [](AsyncUnity::Describe * asyncUnityEntry) {
    asyncUnityEntry->it("should fail", __LINE__, []() {
      TEST_ASSERT_TRUE(false);
    });
    asyncUnityEntry->it("should pass", __LINE__, []() {
      TEST_ASSERT_TRUE(true);
    });
    asyncUnityEntry->it("should fail", __LINE__, []() {
      TEST_ASSERT_TRUE(false);
    });
  });
});