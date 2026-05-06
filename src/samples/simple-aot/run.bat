
call build.bat

call dotnet build ..\..\leanaot\Test\Test.csproj

build\bin\Debug\simple-aot.exe -l ..\..\libraries\dotnetframework4.x -l ..\..\leanaot\Test\bin\Debug -e App::Main Test
