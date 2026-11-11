# Historical .NET 10 fixtures

These nine sources are isolated copies of the fixtures referenced by the imported
.NET 10 test project. They come from the previously reviewed `coreclr-wqaetly`
source snapshot. Their current Core equivalents are empty or intentionally disabled;
copying them here restores the historical project's build without changing Mono tests.

They are historical test inputs, not the compatibility specification. In particular,
the signed constant float conversion assertions also fail on the installed .NET 10
runtime. `scripts/dotnet10/legacy-matrix.py` runs each independent entry group and
records stock-runtime output for every failing group. Worker-thread and worker-task
groups are intentionally unsupported by the minimal single-threaded profile.
