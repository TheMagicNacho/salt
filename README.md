# Salt Text Editor

This is the text editor if your salty that microsoft made notepad so bloated.

## Developer Run

### Dependencies

- msys2 : https://www.msys2.org/
- windows sdk: https://learn.microsoft.com/en-gb/windows/apps/windows-sdk/
- bazel : https://bazel.build/install/windows
  - bazelisk: https://github.com/bazelbuild/bazelisk

## set up

- get teh compile_commands.json by running `bazelisk run @hedron_compile_commands//:refresh_all`

### Commands

- To run: `bazelisk run //main:salt`
- To build: `bazelisk build //main:salt`
  - Building will place the executable in: `.\bazel-bin\main\salt.exe`

# FAQ

- Does this have to run in windows 11?
  - Yes
- Will you ever support Mac?
  - No
- Do I have to develop in Windows 11?
  - I suppose you could cross compile, but that seems annoying.
- Why isn't this in Rust?
  - Cuz im lazy. Make your own in rust if you care so much.
