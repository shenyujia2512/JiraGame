@echo off

cd ..\
protobuf-net\ProtoGen\protogen.exe ^
-i:command.txt ^
-i:scene.txt ^
-o:PBMessage\PBMessage.cs -ns:PBMessage
cd gen
