# Private iOS native build

This repository contains only a frozen native source dependency archive and its
build workflow. It does not contain player captures, credentials, signing keys,
server sources, the game resource library, or an installable signed application.

The first workflow builds the existing arm64 native project without signing.
Client resources and the monotonic private OTA identity are integrated only after
the current feature repair batch freezes. A native shell artifact alone is not a
gameplay validation or a complete test package.
