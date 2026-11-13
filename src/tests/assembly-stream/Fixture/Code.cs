namespace StreamFixture { public static class Entry { public static int Calls; public static int Answer() { Calls++;return StreamDependency.Math.Answer(); } } }
