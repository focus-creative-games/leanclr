
using System;

namespace Tests.Instruments.Fields
{
    // Tracker class to observe whether StsfldI2CctorTestClass's static constructor was called.
    // This class is accessed first and has no explicit cctor, so it initializes independently.
    public static class StsfldI2CctorTracker
    {
        public static bool WasCalled;
    }

    // Test class with an explicit static constructor and a static short field.
    // Regression: stsfld.i2 was missing TRY_RUN_CLASS_STATIC_CCTOR, so writing to
    // a short static field would not trigger the class's static constructor.
    public class StsfldI2CctorTestClass
    {
        public static short ShortField;

        static StsfldI2CctorTestClass()
        {
            StsfldI2CctorTracker.WasCalled = true;
        }
    }

    internal class TC_stsfld_i2_cctor : GeneralTestCaseBase
    {
        [UnitTest]
        public void stsfld_i2_triggers_cctor()
        {
            // Ensure the tracker flag is false before the test
            Assert.IsFalse(StsfldI2CctorTracker.WasCalled);

            // Write to the short static field — this is the first access to StsfldI2CctorTestClass,
            // so it must trigger the static constructor.
            StsfldI2CctorTestClass.ShortField = 100;

            // Verify the static constructor was called
            Assert.IsTrue(StsfldI2CctorTracker.WasCalled);

            // Verify the field was set correctly
            Assert.Equal(100, (int)StsfldI2CctorTestClass.ShortField);
        }
    }
}
